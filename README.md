# Annulateur de Bruit par Inversion de Phase

Application d'annulation de bruit en temps réel utilisant la technologie ASIO pour une latence minimale.

## État Actuel du Projet

- **Frontend** : Interface utilisateur réactive avec visualisations avancées (niveau d'entrée et analyse FFT)
- **Backend** : Serveur Express fonctionnel avec API REST complète
- **Module ASIO** : Module natif C++ implémenté avec mode simulation fonctionnel et détection des périphériques ASIO
- **Statut** : Application fonctionnelle en mode simulation, prête pour les tests avec des périphériques ASIO réels

## Dernières Améliorations (10/04/2025)

- Implémentation de la fonction de callback statique `bufferSwitchStatic` pour le traitement audio ASIO
- Adaptation du module pour fonctionner en mode simulation sans nécessiter le SDK ASIO complet
- Correction des problèmes de compilation liés aux variables statiques et aux directives de préprocesseur
- Simulation des périphériques ASIO, incluant le Focusrite Saffire Pro 24, Steinberg UR22 et RME Fireface UCX
- Initialisation, démarrage et arrêt du traitement audio fonctionnels
- Correction des problèmes de casse dans les méthodes JavaScript/C++ (`initialize` vs `Initialize`)
- Amélioration de la gestion des périphériques ASIO dans l'adaptateur JavaScript
- Implémentation de la méthode `SetInversionGain` pour le contrôle du gain d'inversion

## Fonctionnalités

- Sélection séparée des périphériques d'entrée et de sortie
- Contrôle du gain d'inversion de phase
- Visualisations en temps réel des niveaux audio
- Architecture hybride avec simulation intégrée pour faciliter le développement

## Prérequis

- Node.js 14+ et npm
- Environnement de compilation C++ (Visual Studio Build Tools ou équivalent)
- SDK ASIO 2.3+ (optionnel, disponible sur [steinberg.net](https://www.steinberg.net/asiosdk))

## Installation

1. Clonez le dépôt :
   ```
   git clone https://github.com/votre-utilisateur/annulateur.git
   cd annulateur
   ```

2. Installation des dépendances frontend :
   ```
   cd frontend
   npm install
   ```

3. Installation des dépendances backend :
   ```
   cd ../backend
   npm install
   npm install node-addon-api bindings
   ```

4. Compilation du module natif ASIO :
   ```
   cd asio
   node-gyp rebuild
   ```
   
   Si vous rencontrez des erreurs de compilation, vérifiez que vous avez bien installé les outils de build nécessaires :
   ```
   npm install -g node-gyp
   npm install -g windows-build-tools  # Sur Windows uniquement
   ```

## Utilisation

1. Compilez le module natif ASIO :
   ```
   cd backend/asio
   node-gyp rebuild
   ```

2. Démarrez le backend :
   ```
   cd backend
   node server.js
   ```

3. Démarrez le frontend dans un autre terminal :
   ```
   cd frontend
   npm run dev
   ```

4. Accédez à l'application dans votre navigateur :
   ```
   http://localhost:5174
   ```

5. Utilisation de l'application :
   - Cliquez sur "Initialiser ASIO"
   - Sélectionnez un périphérique d'entrée (microphone) pour capter le son
   - Sélectionnez un périphérique de sortie (haut-parleurs) pour émettre le son inversé
   - Réglez le gain d'inversion selon vos besoins
   - Cliquez sur "Démarrer" pour activer l'annulation de bruit
   - Observez les visualisations qui s'animent en fonction du son capté

## Architecture Actuelle

```
📂 annulateur
├── 📂 asiosdk_2.3.3_2019-06-14     # SDK ASIO de Steinberg (optionnel)
├── 📂 backend
│   ├── 📂 asio
│   │   ├── binding.gyp           # Configuration node-gyp 
│   │   ├── asio_processor.cpp    # Module natif C++
│   │   ├── asiodrivers.h/cpp     # Gestion des pilotes ASIO
│   │   ├── asiolist.h/cpp        # Liste des pilotes ASIO
│   │   └── build/                # Dossier de compilation
│   ├── asio_adapter.js          # Adaptateur pour le module natif
│   └── server.js                # Serveur Express avec API REST
├── 📂 frontend
│   ├── 📂 src
│   │   ├── App.jsx              # Interface principale
│   │   └── main.jsx              # Point d'entrée React
│   ├── index.html               # Page HTML principale
│   └── vite.config.js           # Configuration Vite
└── README.md                     # Documentation du projet
```

## Fonctionnement

L'annulation de bruit par inversion de phase fonctionne selon le principe suivant :

1. **Captation du son** : Un microphone capte le son indésirable (le bruit à annuler)
2. **Inversion de phase** : Le signal audio est inversé (multiplication par -1)
3. **Émission du son inversé** : Le son en opposition de phase est diffusé par un haut-parleur
4. **Annulation physique** : Les ondes sonores originales et inversées s'annulent mutuellement

Pour une annulation efficace, la latence doit être minimale, d'où l'utilisation d'ASIO.

## Problèmes Rencontrés et Solutions

### 1. Problèmes d'Initialisation ASIO

- **Problème**: Erreur `TypeError: this.handler.initialize is not a function` lors de l'initialisation du pilote ASIO.
- **Cause**: Différence de casse entre la méthode exportée par le module C++ (`initialize`) et celle appelée dans l'adaptateur JavaScript (`Initialize`).
- **Solution**: Modification de l'adaptateur JavaScript pour détecter automatiquement la méthode disponible et l'utiliser avec la bonne casse.

### 2. Détection des Périphériques ASIO

- **Problème**: Le module natif détecte 4 pilotes ASIO mais renvoie une liste vide à JavaScript.
- **Cause**: Problème dans la fonction `getDevices()` du module natif C++ qui ne remplit pas correctement le tableau de périphériques.
- **Solution**: Implémentation de périphériques ASIO simulés dans le module natif pour garantir le fonctionnement de l'application.

### 3. Compilation du Module Natif

- **Problème**: Difficultés de compilation du module natif ASIO avec le SDK complet.
- **Cause**: Configuration complexe et dépendances du SDK ASIO.
- **Solution**: Utilisation de directives de préprocesseur pour permettre la compilation sans le SDK complet et implémentation de fonctions simulées.

## Travaux Futurs

### 1. Finalisation de l'intégration ASIO

Les bases du module ASIO sont maintenant implémentées en mode simulation. Pour finaliser l'intégration :

- Améliorer la détection des périphériques ASIO réels :
  - Corriger la fonction `getDevices()` pour détecter et utiliser correctement les pilotes ASIO installés
  - Implémenter un système de fallback plus robuste entre les périphériques réels et simulés

- Intégration avec le SDK ASIO réel :
  - Obtenir et installer le SDK ASIO complet de Steinberg
  - Adapter le code pour utiliser les fonctions ASIO réelles au lieu de la simulation
  - Tester avec des périphériques ASIO réels comme Focusrite Saffire Pro 24
  - Optimiser les performances et la latence
  - Implémenter la gestion des interruptions et des erreurs matérielles

- Optimisation du traitement audio :
  - Implémentation d'un buffer circulaire pour réduire les discontinuités audio
  - Utilisation de SIMD (SSE/AVX) pour accélérer le traitement des échantillons
  - Réduction de l'empreinte mémoire pour les systèmes limités

### 2. Améliorations fonctionnelles

- **Filtrage adaptatif** : Améliorer l'algorithme pour s'adapter aux variations de bruit
- **Sélection de canaux** : Permettre la sélection de canaux spécifiques par périphérique
- **Présets** : Ajouter la possibilité de sauvegarder et charger des configurations
- **Égaliseur** : Intégrer un égaliseur pour cibler des fréquences spécifiques
- **Détection automatique** : Système de détection automatique des sources de bruit
- **Calibration** : Assistant de calibration pour optimiser l'annulation selon l'environnement
- **Modes spécialisés** : Modes préréglés pour différents environnements (bureau, extérieur, etc.)
- **Contrôle à distance** : Application mobile compagnon pour le contrôle à distance

### 3. Améliorations techniques

- **Gestion d'erreurs** : Améliorer la gestion des erreurs et exceptions
- **Tests unitaires** : Implémenter des tests pour chaque composant
- **Mode autonome** : Créer une version standalone sans nécessiter un navigateur
- **Installation** : Fournir un installateur pour Windows
- **Multi-plateforme** : Support pour macOS et Linux
- **Conteneurisation** : Configuration Docker pour faciliter le déploiement
- **CI/CD** : Pipeline d'intégration continue pour les tests et le déploiement
- **Monitoring** : Système de télémétrie pour détecter les problèmes en production
- **Mise à jour OTA** : Système de mise à jour automatique pour les versions futures

### 4. Documentation et Formation

- **Documentation API** : Documenter l'API REST complète
- **Documentation utilisateur** : Créer un manuel utilisateur détaillé
- **Vidéos démonstratives** : Montrer des cas d'utilisation réels
- **Tutoriels interactifs** : Guide intégré à l'application pour les nouveaux utilisateurs
- **Documentation de développement** : Guide pour les contributeurs externes
- **Wiki** : Base de connaissances collaborative pour les cas d'usage et solutions

### 5. Recherche et Innovation

- **Algorithmes avancés** : Recherche sur les algorithmes d'annulation active plus sophistiqués
- **Apprentissage automatique** : Utilisation de modèles ML pour prédire et annuler les bruits complexes
- **Analyse prédictive** : Anticiper les variations de bruit pour une annulation plus efficace
- **Intégration IoT** : Connectivité avec d'autres appareils intelligents pour une annulation coordonnée
- **Réalité augmentée sonore** : Superposition de sons utiles tout en annulant les bruits indésirables

### 6. Commercialisation et Distribution

- **Modèle freemium** : Version de base gratuite et fonctionnalités premium payantes
- **Version entreprise** : Solution adaptée aux environnements professionnels
- **Partenariats** : Collaboration avec fabricants de matériel audio
- **API as a Service** : Offrir l'algorithme d'annulation comme service cloud pour d'autres applications
- **Marketplace** : Plateforme pour partager des préréglages et des configurations

## Optimisation des performances

Pour une latence minimale une fois l'intégration ASIO réelle terminée :

- Configurez une taille de buffer de 64-128 échantillons (1.45-2.9ms @ 44.1kHz)
- Désactivez les effets audio non essentiels
- Utilisez des périphériques audio de qualité professionnelle
- Exécutez l'application avec une priorité élevée
