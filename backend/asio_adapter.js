/**
 * Adaptateur pour le module natif ASIO
 * Fait l'interface entre le serveur Express et le module C++ ASIO
 */

const path = require('path');
let asioAddon;

try {
  // Essayer de charger le module natif compilé
  asioAddon = require(path.join(__dirname, 'asio/build/Release/asio_addon'));
  console.log('Module ASIO natif chargé avec succès');
  
  // Vérifier si le module natif a toutes les méthodes nécessaires
  const dummyHandler = new asioAddon.ASIOHandler();
  const ASIOHandlerClass = asioAddon.ASIOHandler;
  const requiredMethods = ['initialize', 'start', 'stop', 'getInputLevel', 'getFFTData', 'getDevices'];
  
  let missingMethods = [];
  for (const method of requiredMethods) {
    // Vérifier si la méthode existe soit comme méthode d'instance, soit comme méthode statique
    if (typeof dummyHandler[method] !== 'function' && typeof ASIOHandlerClass[method] !== 'function') {
      missingMethods.push(method);
    }
  }
  
  if (missingMethods.length > 0) {
    console.warn(`Le module ASIO natif ne contient pas toutes les méthodes requises: ${missingMethods.join(', ')}`);
    console.warn('Utilisation de la simulation ASIO à la place');
    asioAddon = null;
  }
} catch (err) {
  console.error('Erreur de chargement du module ASIO natif:', err);
  console.warn('Utilisation de la simulation ASIO à la place');
  asioAddon = null;
}

if (asioAddon === null) {
  console.log('API ASIO simulée en cours d\'exécution (pas de vrai module ASIO chargé)');
  console.log('Pour utiliser le vrai module ASIO, veuillez compiler le module natif');
}

// Simulation de secours si le module natif ne peut pas être chargé
const asioSimulation = {
  initialized: false,
  processing: false,
  gain: 1.0,
  currentInputDevice: null,
  currentOutputDevice: null,
  availableDevices: [
    { id: "input1", name: "Microphone intégré", isInput: true },
    { id: "input2", name: "Entrée ligne", isInput: true },
    { id: "input3", name: "Microphone (Realtek Audio)", isInput: true },
    { id: "input4", name: "Microphone (USB)", isInput: true },
    { id: "output1", name: "Haut-parleurs intégrés", isInput: false },
    { id: "output2", name: "Sortie HDMI", isInput: false },
    { id: "output3", name: "Haut-parleurs (Realtek Audio)", isInput: false },
    { id: "output4", name: "Casque (USB)", isInput: false }
  ],
  initialize: () => {
    asioSimulation.initialized = true;
    console.log('ASIO simulation initialisée');
    return { success: true };
  },
  start: (options) => {
    if (!asioSimulation.initialized) return { success: false, error: 'ASIO non initialisé' };
    asioSimulation.processing = true;
    asioSimulation.gain = options.gain || 1.0;
    asioSimulation.currentInputDevice = options.inputDeviceId;
    asioSimulation.currentOutputDevice = options.outputDeviceId;
    console.log(`ASIO simulation démarrée avec le gain ${asioSimulation.gain}, entrée: ${asioSimulation.currentInputDevice}, sortie: ${asioSimulation.currentOutputDevice}`);
    return { success: true };
  },
  stop: () => {
    asioSimulation.processing = false;
    console.log('ASIO simulation arrêtée');
    return { success: true };
  },
  getInputLevel: () => {
    if (!asioSimulation.processing) return 0;
    // Simuler un niveau d'entrée entre 0 et 1
    return Math.random();
  },
  getFFTData: () => {
    if (!asioSimulation.processing) return Array(32).fill(0);
    // Simuler des données FFT (32 bandes)
    return Array(32).fill(0).map(() => Math.random());
  }
};

/**
 * Interface unifiée pour le module ASIO (réel ou simulé)
 */
class ASIOInterface {
  constructor() {
    // Déterminer si on utilise le module natif ou la simulation
    this.useNative = !!asioAddon;
    this.handler = this.useNative ? new asioAddon.ASIOHandler() : asioSimulation;
    
    // Initialiser l'état
    this.initialized = false;
    this.processing = false;
    this.currentInputDevice = null;
    this.currentOutputDevice = null;
    this.gain = 1.0;
  }

  /**
   * Récupérer la liste des périphériques audio disponibles
   */
  getDevices() {
    if (this.useNative) {
      try {
        // Utiliser le module natif pour récupérer les périphériques ASIO
        console.log('Utilisation du module natif pour récupérer les périphériques ASIO');
        
        // Vérifier si la méthode getDevices existe
        if (typeof asioAddon.ASIOHandler.getDevices !== 'function') {
          console.warn('Méthode getDevices non trouvée dans le module natif');
          throw new Error('Méthode getDevices non disponible');
        }
        
        const nativeDevices = asioAddon.ASIOHandler.getDevices();
        console.log('Périphériques ASIO détectés par le module natif:', nativeDevices);
        
        // Si aucun périphérique n'est détecté ou si la liste est vide, utiliser des périphériques simulés
        if (!nativeDevices || nativeDevices.length === 0) {
          console.log('Aucun périphérique ASIO détecté, utilisation de périphériques simulés');
          
          // Créer des périphériques simulés
          return [
            {
              id: 'input_sim',
              name: 'Simulation ASIO (Entrée)',
              isInput: true,
              driverId: 'sim',
              driverName: 'Simulation ASIO',
              isSimulated: true
            },
            {
              id: 'output_sim',
              name: 'Simulation ASIO (Sortie)',
              isInput: false,
              driverId: 'sim',
              driverName: 'Simulation ASIO',
              isSimulated: true
            }
          ];
        }
        
        // Créer deux périphériques (entrée et sortie) pour chaque pilote ASIO détecté
        const devices = [];
        for (let i = 0; i < nativeDevices.length; i++) {
          const device = nativeDevices[i];
          if (!device || !device.name) {
            console.warn(`Périphérique #${i} invalide, ignoré`);
            continue;
          }
          
          // Ajouter un périphérique d'entrée
          devices.push({
            id: `input_${device.id || i}`,
            name: `${device.name} (Entrée)`,
            isInput: true,
            driverId: device.id || i,
            driverName: device.name,
            isSimulated: device.isSimulated || false
          });
          
          // Ajouter un périphérique de sortie
          devices.push({
            id: `output_${device.id || i}`,
            name: `${device.name} (Sortie)`,
            isInput: false,
            driverId: device.id || i,
            driverName: device.name,
            isSimulated: device.isSimulated || false
          });
        }
        
        return devices;
      } catch (err) {
        console.error('Erreur lors de la récupération des périphériques ASIO:', err);
        console.warn('Utilisation de la simulation ASIO à la place');
      }
    }
    
    // Utiliser la liste simulée si le module natif n'est pas disponible ou en cas d'erreur
    return asioSimulation.availableDevices;
  }

  /**
   * Initialiser ASIO
   * @param {string} driverName - Nom du pilote ASIO à initialiser
   */
  initialize(driverName) {
    try {
      if (this.useNative) {
        try {
          // Extraire le nom du pilote à partir de l'ID du périphérique
          // Format attendu: input_X ou output_X où X est l'ID du pilote
          let actualDriverName = driverName;
          
          if (driverName && (driverName.startsWith('input_') || driverName.startsWith('output_'))) {
            // Extraire l'ID du pilote à partir de l'ID du périphérique
            const driverId = driverName.split('_')[1];
            
            // Récupérer tous les périphériques
            const devices = this.getDevices();
            
            // Trouver le périphérique correspondant à l'ID
            const device = devices.find(d => d.driverId == driverId);
            
            if (device && device.driverName) {
              actualDriverName = device.driverName;
              console.log(`Utilisation du pilote ASIO: ${actualDriverName}`);
            }
          }
          
          // Initialiser le pilote ASIO
          console.log(`Initialisation du pilote ASIO: ${actualDriverName}`);
          // Vérifier la méthode disponible (initialize ou Initialize)
          const initMethod = typeof this.handler.initialize === 'function' ? 'initialize' : 'Initialize';
          console.log(`Méthode d'initialisation utilisée: ${initMethod}`);
          const result = this.handler[initMethod](actualDriverName);
          
          if (result.success) {
            this.initialized = true;
            console.log('Pilote ASIO initialisé avec succès');
          }
          
          return result;
        } catch (err) {
          console.error('Erreur lors de l\'initialisation du pilote ASIO:', err);
          console.warn('Utilisation de la simulation ASIO à la place');
        }
      }
      
      // Utiliser la simulation si le module natif n'est pas disponible ou en cas d'erreur
      const result = asioSimulation.initialize();
      if (result.success) {
        this.initialized = true;
      }
      return result;
    } catch (err) {
      console.error('Erreur lors de l\'initialisation d\'ASIO:', err);
      return { success: false, error: err.message };
    }
  }

  /**
   * Démarrer le traitement audio
   */
  start(options = {}) {
    if (!this.initialized) {
      return { success: false, error: 'ASIO n\'est pas initialisé' };
    }

    try {
      if (this.useNative) {
        try {
          // Utiliser le module natif pour démarrer le traitement audio
          console.log('Démarrage du traitement audio avec le module natif ASIO');
          console.log('Options:', options);
          
          // Démarrer le traitement audio avec le gain spécifié
          const result = this.handler.start(options.gain || 1.0);
          
          if (result.success) {
            this.processing = true;
            this.currentInputDevice = options.inputDeviceId || null;
            this.currentOutputDevice = options.outputDeviceId || null;
            this.gain = options.gain || 1.0;
            console.log('Traitement audio démarré avec succès');
          }
          
          return result;
        } catch (err) {
          console.error('Erreur lors du démarrage du traitement audio avec le module natif:', err);
          console.warn('Utilisation de la simulation ASIO à la place');
        }
      }
      
      // Utiliser la simulation si le module natif n'est pas disponible ou en cas d'erreur
      const result = asioSimulation.start(options);
      if (result.success) {
        this.processing = true;
        this.currentInputDevice = options.inputDeviceId || null;
        this.currentOutputDevice = options.outputDeviceId || null;
        this.gain = options.gain || 1.0;
      }
      return result;
    } catch (err) {
      console.error('Erreur lors du démarrage du traitement audio:', err);
      return { success: false, error: err.message };
    }
  }

  /**
   * Arrêter le traitement audio
   */
  stop() {
    if (!this.processing) {
      return { success: true, message: 'Le traitement audio est déjà arrêté' };
    }

    try {
      if (this.useNative) {
        try {
          // Utiliser le module natif pour arrêter le traitement audio
          console.log('Arrêt du traitement audio avec le module natif ASIO');
          
          const result = this.handler.stop();
          
          if (result.success) {
            this.processing = false;
            console.log('Traitement audio arrêté avec succès');
          }
          
          return result;
        } catch (err) {
          console.error('Erreur lors de l\'arrêt du traitement audio avec le module natif:', err);
          console.warn('Utilisation de la simulation ASIO à la place');
        }
      }
      
      // Utiliser la simulation si le module natif n'est pas disponible ou en cas d'erreur
      const result = asioSimulation.stop();
      if (result.success) {
        this.processing = false;
      }
      return result;
    } catch (err) {
      console.error('Erreur lors de l\'arrêt du traitement audio:', err);
      return { success: false, error: err.message };
    }
  }

  /**
   * Obtenir le niveau d'entrée audio actuel
   */
  getInputLevel() {
    if (!this.processing) return 0;

    try {
      // Utiliser directement la simulation pour l'instant
      return asioSimulation.getInputLevel();
    } catch (err) {
      console.error('Erreur lors de la récupération du niveau d\'entrée:', err);
      return 0;
    }
  }

  /**
   * Obtenir les données FFT pour visualisation
   */
  getFFTData() {
    if (!this.processing) return Array(32).fill(0);

    try {
      // Utiliser directement la simulation pour l'instant
      return asioSimulation.getFFTData();
    } catch (err) {
      console.error('Erreur lors de la récupération des données FFT:', err);
      return Array(32).fill(0);
    }
  }

  /**
   * Obtenir le statut actuel d'ASIO
   */
  getStatus() {
    return {
      initialized: this.initialized,
      processing: this.processing,
      currentInputDevice: this.currentInputDevice,
      currentOutputDevice: this.currentOutputDevice,
      gain: this.gain,
      useNative: this.useNative
    };
  }
}

// Exporter une instance unique de l'interface ASIO
module.exports = new ASIOInterface();
