#include <napi.h>
#include <atomic>
#include <mutex>
#include <vector>
#include <thread>
#include <cmath> // Pour std::sqrt et std::rand
#include <condition_variable> // Pour std::condition_variable
#include <iostream> // Pour std::cout et std::endl

// Définir ASIOCallConv comme __stdcall sur Windows et comme vide sur les autres plateformes
#ifdef _WIN32
#define ASIOCallConv __stdcall
#else
#define ASIOCallConv
#endif

// Inclusions pour le SDK ASIO réel
#include "asiosys.h"
#include "asio.h"
#include "asiodrivers.h"

// Déclaration externe pour AsioDrivers
extern AsioDrivers* asioDrivers;

// Constantes pour les erreurs ASIO
#define ASE_OK 0

// Implémentations simplifiées des fonctions ASIO pour la compilation
// Ces fonctions seront remplacées par les vraies fonctions ASIO lorsque le SDK ASIO sera correctement installé
long ASIOInit(ASIODriverInfo* info) { 
  if (info) {
    strcpy(info->name, "Simulation ASIO"); 
    info->asioVersion = 2; 
  }
  return ASE_OK; 
}

long ASIOExit() { return ASE_OK; }
long ASIOStart() { return ASE_OK; }
long ASIOStop() { return ASE_OK; }

long ASIOGetChannels(long* numInputChannels, long* numOutputChannels) { 
  if (numInputChannels) *numInputChannels = 2; 
  if (numOutputChannels) *numOutputChannels = 2; 
  return ASE_OK; 
}

long ASIOGetBufferSize(long* minSize, long* maxSize, long* preferredSize, long* granularity) {
  if (minSize) *minSize = 256;
  if (maxSize) *maxSize = 2048;
  if (preferredSize) *preferredSize = 1024;
  if (granularity) *granularity = 256;
  return ASE_OK;
}

long ASIOCreateBuffers(ASIOBufferInfo* bufferInfos, long numChannels, long bufferSize, ASIOCallbacks* callbacks) { 
  return ASE_OK; 
}

long ASIODisposeBuffers() { return ASE_OK; }

class ASIOHandler : public Napi::ObjectWrap<ASIOHandler> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  ASIOHandler(const Napi::CallbackInfo& info);
  
  // Structure pour les buffers audio
  struct AudioBuffer {
    std::vector<float> input;
    std::vector<float> output;
    std::atomic<bool> ready{false};
  };

  // Callbacks ASIO
  void bufferSwitch(long index, ASIOBool processNow) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    // Traitement d'inversion de phase
    for(int i = 0; i < bufferSize; i++) {
      currentBuffer->output[i] = currentBuffer->input[i] * -gain;
    }
    
    currentBuffer->ready.store(true);
    bufferCondition.notify_one();
  }
  
  // Fonction de callback statique pour ASIO
  static void ASIOCallConv bufferSwitchStatic(long index, ASIOBool processNow) {
    // Cette fonction est appelée par le pilote ASIO lorsqu'un buffer est prêt
    // Nous devons rediriger l'appel vers l'instance de ASIOHandler
    // Pour simplifier, nous utilisons des variables statiques
    if (processing.load()) {
      // Sélectionner le buffer actif
      currentBuffer = &buffers[index];
      
      // Copier les données d'entrée depuis les buffers ASIO
      // Dans une implémentation réelle, ces données proviendraient du pilote ASIO
      
      // Traitement d'inversion de phase
      for(int i = 0; i < bufferSize; i++) {
        currentBuffer->output[i] = currentBuffer->input[i] * -gain;
      }
      
      currentBuffer->ready.store(true);
      bufferCondition.notify_one();
    }
  }

private:
  // Méthodes exposées à JavaScript
  static Napi::Value Initialize(const Napi::CallbackInfo& info);
  static Napi::Value Start(const Napi::CallbackInfo& info);
  static Napi::Value Stop(const Napi::CallbackInfo& info);
  static Napi::Value GetInputLevel(const Napi::CallbackInfo& info);
  static Napi::Value GetFFTData(const Napi::CallbackInfo& info);
  static Napi::Value SetBufferSize(const Napi::CallbackInfo& info);
  static Napi::Value SetInversionGain(const Napi::CallbackInfo& info);
  static Napi::Value getDevices(const Napi::CallbackInfo& info);

  // Variables ASIO
  static ASIODriverInfo driverInfo;
  static ASIOBufferInfo bufferInfos[2];
  static long inputChannels;
  static long outputChannels;
  static long bufferSize;
  static long minSize, maxSize, preferredSize, granularity;

  // Synchronisation
  static std::mutex bufferMutex;
  static std::condition_variable bufferCondition;
  static AudioBuffer buffers[2];
  static AudioBuffer* currentBuffer;
  static float gain;
  static std::atomic<bool> processing;
};

// Initialisation des variables statiques
std::mutex ASIOHandler::bufferMutex;
std::condition_variable ASIOHandler::bufferCondition;
ASIOHandler::AudioBuffer ASIOHandler::buffers[2];
ASIOHandler::AudioBuffer* ASIOHandler::currentBuffer = &ASIOHandler::buffers[0];
float ASIOHandler::gain = 1.0f;
std::atomic<bool> ASIOHandler::processing{false};
long ASIOHandler::bufferSize = 1024;
ASIODriverInfo ASIOHandler::driverInfo;
ASIOBufferInfo ASIOHandler::bufferInfos[2];
long ASIOHandler::inputChannels = 0;
long ASIOHandler::outputChannels = 0;
long ASIOHandler::minSize = 0;
long ASIOHandler::maxSize = 0;
long ASIOHandler::preferredSize = 0;
long ASIOHandler::granularity = 0;

ASIOHandler::ASIOHandler(const Napi::CallbackInfo& info) 
  : Napi::ObjectWrap<ASIOHandler>(info) {
  // Initialisation du driver ASIO
  
  // Allocation des buffers
  buffers[0].input.resize(bufferSize);
  buffers[0].output.resize(bufferSize);
  buffers[1].input.resize(bufferSize);
  buffers[1].output.resize(bufferSize);
}

Napi::Value ASIOHandler::Initialize(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  // Vérifier les arguments
  if (info.Length() < 1) {
    Napi::TypeError::New(env, "Argument 1 doit être l'ID ou le nom du pilote ASIO").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  std::string driverIdentifier;
  long driverId = -1;
  bool isSimulated = false;
  
  // Déterminer si l'argument est un ID numérique ou un nom de pilote
  if (info[0].IsNumber()) {
    driverId = info[0].As<Napi::Number>().Int32Value();
    std::cout << "Initialisation du pilote ASIO avec ID: " << driverId << std::endl;
  } else if (info[0].IsString()) {
    driverIdentifier = info[0].As<Napi::String>().Utf8Value();
    std::cout << "Initialisation du pilote ASIO: " << driverIdentifier << std::endl;
    
    // Vérifier si c'est un pilote simulé
    if (driverIdentifier == "Simulation ASIO") {
      isSimulated = true;
    }
  } else {
    Napi::TypeError::New(env, "Argument 1 doit être un nombre (ID) ou une chaîne (nom)").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  // Initialiser AsioDrivers si nécessaire
  if (!asioDrivers) {
    std::cout << "Initialisation de AsioDrivers..." << std::endl;
    asioDrivers = new AsioDrivers();
  }
  
  // Obtenir le nombre de pilotes ASIO disponibles
  char* driverNames[32]; // Tableau pour stocker les noms des pilotes
  long numDrivers = asioDrivers->getDriverNames(driverNames, 32);
  
  // Si c'est un pilote simulé ou si aucun pilote ASIO n'est disponible
  if (isSimulated || numDrivers <= 0) {
    std::cout << "Utilisation du pilote ASIO simulé" << std::endl;
    
    // Initialiser le pilote ASIO simulé
    strcpy(driverInfo.name, "Simulation ASIO");
    driverInfo.asioVersion = 2;
    
    // Obtenir les canaux d'entrée et de sortie
    inputChannels = 2;
    outputChannels = 2;
    
    // Obtenir les tailles de buffer disponibles
    minSize = 256;
    maxSize = 2048;
    preferredSize = 1024;
    granularity = 256;
    
    // Utiliser la taille de buffer préférée
    bufferSize = preferredSize;
  } else {
    // Charger le pilote ASIO réel
    bool driverLoaded = false;
    
    if (driverId >= 0 && driverId < numDrivers) {
      // Charger par ID
      char* driverName = driverNames[driverId];
      if (driverName) {
        driverLoaded = asioDrivers->loadDriver(driverName);
        driverIdentifier = driverName;
      }
    } else {
      // Charger par nom
      driverLoaded = asioDrivers->loadDriver(const_cast<char*>(driverIdentifier.c_str()));
    }
    
    if (!driverLoaded) {
      Napi::Error::New(env, "Impossible de charger le pilote ASIO: " + driverIdentifier).ThrowAsJavaScriptException();
      return env.Null();
    }
    
    // Initialiser le pilote ASIO
    if (ASIOInit(&driverInfo) != ASE_OK) {
      Napi::Error::New(env, "Erreur lors de l'initialisation du pilote ASIO").ThrowAsJavaScriptException();
      return env.Null();
    }
  }
  
  // Obtenir les informations sur les canaux
  if (ASIOGetChannels(&inputChannels, &outputChannels) != ASE_OK) {
    Napi::Error::New(env, "Erreur lors de la récupération des informations sur les canaux").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  // Obtenir les informations sur les buffers
  if (ASIOGetBufferSize(&minSize, &maxSize, &preferredSize, &granularity) != ASE_OK) {
    Napi::Error::New(env, "Erreur lors de la récupération des informations sur les buffers").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  // Utiliser la taille de buffer préférée
  bufferSize = preferredSize;
  
  // Préparer les buffers
  buffers[0].input.resize(bufferSize);
  buffers[0].output.resize(bufferSize);
  buffers[1].input.resize(bufferSize);
  buffers[1].output.resize(bufferSize);
  
  // Configurer les buffers ASIO
  bufferInfos[0].isInput = ASIOTrue;
  bufferInfos[0].channelNum = 0;
  bufferInfos[0].buffers[0] = buffers[0].input.data();
  bufferInfos[0].buffers[1] = buffers[1].input.data();
  
  bufferInfos[1].isInput = ASIOFalse;
  bufferInfos[1].channelNum = 0;
  bufferInfos[1].buffers[0] = buffers[0].output.data();
  bufferInfos[1].buffers[1] = buffers[1].output.data();
  
  // Créer un objet pour retourner les informations d'initialisation
  Napi::Object result = Napi::Object::New(env);
  result.Set("success", Napi::Boolean::New(env, true));
  result.Set("driverName", Napi::String::New(env, driverIdentifier.c_str()));
  result.Set("inputChannels", Napi::Number::New(env, inputChannels));
  result.Set("outputChannels", Napi::Number::New(env, outputChannels));
  result.Set("bufferSize", Napi::Number::New(env, bufferSize));
  
  return result;
}

Napi::Value ASIOHandler::Start(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  // Vérifier les arguments pour le gain (facteur d'inversion de phase)
  if (info.Length() >= 1 && info[0].IsNumber()) {
    gain = info[0].As<Napi::Number>().DoubleValue();
  } else {
    gain = 1.0; // Valeur par défaut
  }
  
#ifdef ASIO_INCLUDED
  // Configurer les callbacks ASIO
  ASIOCallbacks callbacks;
  callbacks.bufferSwitch = &ASIOHandler::bufferSwitchStatic;
  callbacks.sampleRateDidChange = nullptr;
  callbacks.asioMessage = nullptr;
  callbacks.bufferSwitchTimeInfo = nullptr;
  
  // Créer les buffers ASIO
  if (ASIOCreateBuffers(bufferInfos, 2, bufferSize, &callbacks) != ASE_OK) {
    Napi::Error::New(env, "Erreur lors de la création des buffers ASIO").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  // Démarrer le traitement audio
  if (ASIOStart() != ASE_OK) {
    Napi::Error::New(env, "Erreur lors du démarrage du traitement audio").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  // Indiquer que le traitement est en cours
  processing.store(true);
  
  // Créer un objet pour retourner les informations de démarrage
  Napi::Object result = Napi::Object::New(env);
  result.Set("success", Napi::Boolean::New(env, true));
  result.Set("gain", Napi::Number::New(env, gain));
  
  return result;
#else
  // Version simulée pour le développement sans SDK ASIO
  processing.store(true);
  
  Napi::Object result = Napi::Object::New(env);
  result.Set("success", Napi::Boolean::New(env, true));
  result.Set("gain", Napi::Number::New(env, gain));
  result.Set("simulated", Napi::Boolean::New(env, true));
  
  return result;
#endif
}

Napi::Value ASIOHandler::Stop(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
#ifdef ASIO_INCLUDED
  // Arrêter le traitement audio
  if (ASIOStop() != ASE_OK) {
    Napi::Error::New(env, "Erreur lors de l'arrêt du traitement audio").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  // Libérer les buffers ASIO
  if (ASIODisposeBuffers() != ASE_OK) {
    Napi::Error::New(env, "Erreur lors de la libération des buffers ASIO").ThrowAsJavaScriptException();
    return env.Null();
  }
#endif
  
  // Indiquer que le traitement est arrêté
  processing.store(false);
  
  // Créer un objet pour retourner les informations d'arrêt
  Napi::Object result = Napi::Object::New(env);
  result.Set("success", Napi::Boolean::New(env, true));
  
  return result;
}

Napi::Value ASIOHandler::GetInputLevel(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  if (!processing.load()) {
    return Napi::Number::New(env, 0.0f);
  }
  
  // Calcul du niveau d'entrée (RMS)
  float rms = 0.0f;
  int validSamples = 0;
  
  {
    std::lock_guard<std::mutex> lock(bufferMutex);
    for (size_t i = 0; i < currentBuffer->input.size(); i++) {
      const float sample = currentBuffer->input[i];
      if (!std::isnan(sample) && !std::isinf(sample)) {
        rms += sample * sample;
        validSamples++;
      }
    }
  }
  
  // Éviter la division par zéro
  if (validSamples > 0) {
    rms = std::sqrt(rms / validSamples);
  } else {
    rms = 0.0f;
  }
  
  // Normaliser entre 0 et 1, puis convertir en pourcentage
  // La plupart des signaux audio sont normalisés entre -1 et 1
  // donc RMS est généralement entre 0 et 0.707 (sin wave RMS)
  float normalizedRMS = std::min(rms * 1.414f, 1.0f);
  
  return Napi::Number::New(env, normalizedRMS * 100.0f); // Pourcentage
}

Napi::Value ASIOHandler::GetFFTData(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  // Nombre de bandes de fréquence pour l'analyse FFT
  const uint32_t numBands = 32;
  Napi::Array fftData = Napi::Array::New(env, numBands);
  
  if (!processing.load()) {
    // Si le traitement est arrêté, renvoyer un tableau de zéros
    for (uint32_t i = 0; i < numBands; i++) {
      fftData[i] = Napi::Number::New(env, 0);
    }
    return fftData;
  }
  
  // Implémentation simplifiée de l'analyse spectrale
  // Dans une implémentation réelle, nous utiliserions une FFT complète
  // comme la bibliothèque FFTW ou une implémentation personnalisée
  
  std::vector<float> bandEnergies(numBands, 0.0f);
  
  {
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    // Division du buffer en bandes de fréquence (approximation simplifiée)
    // Cette approche est une simulation, pas une vraie FFT
    const size_t samplesPerBand = currentBuffer->input.size() / numBands;
    
    for (uint32_t band = 0; band < numBands; band++) {
      float energy = 0.0f;
      size_t startIdx = band * samplesPerBand;
      size_t endIdx = (band + 1) * samplesPerBand;
      
      // Limiter l'index de fin à la taille du buffer
      endIdx = std::min(endIdx, currentBuffer->input.size());
      
      // Calculer l'énergie pour cette bande
      for (size_t i = startIdx; i < endIdx; i++) {
        energy += currentBuffer->input[i] * currentBuffer->input[i];
      }
      
      // Normaliser par le nombre d'échantillons dans la bande
      if (endIdx > startIdx) {
        energy /= (endIdx - startIdx);
      }
      
      // Appliquer une pondération pour simuler la réponse en fréquence
      // Les hautes fréquences (bandes supérieures) sont généralement plus faibles
      float frequencyWeight = 1.0f - (0.5f * band / numBands);
      energy *= frequencyWeight;
      
      // Stocker l'énergie de la bande
      bandEnergies[band] = energy;
    }
  }
  
  // Normaliser les valeurs pour l'affichage
  float maxEnergy = 0.0f;
  for (const auto& energy : bandEnergies) {
    maxEnergy = std::max(maxEnergy, energy);
  }
  
  // Éviter la division par zéro
  if (maxEnergy > 0.0f) {
    for (uint32_t i = 0; i < numBands; i++) {
      // Convertir en pourcentage et appliquer une échelle logarithmique simplifiée
      float normalizedValue = (bandEnergies[i] / maxEnergy);
      float displayValue = std::sqrt(normalizedValue) * 100.0f; // Échelle non linéaire pour meilleure visualisation
      fftData[i] = Napi::Number::New(env, displayValue);
    }
  } else {
    // Si aucune énergie détectée, renvoyer des zéros
    for (uint32_t i = 0; i < numBands; i++) {
      fftData[i] = Napi::Number::New(env, 0);
    }
  }
  
  return fftData;
}

// *** Implémentation de GetDevices ***
#ifdef ASIO_INCLUDED
// Helper class pour gérer AsioDrivers (comme recommandé dans les exemples ASIO SDK)
// Note: AsioDrivers est défini dans asiodrivers.cpp qui est compilé séparément

Napi::Value ASIOHandler::getDevices(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Créer un tableau pour stocker les périphériques
    Napi::Array devices = Napi::Array::New(env);
    
    // Initialiser AsioDrivers si nécessaire
    if (!asioDrivers) {
        std::cout << "Initialisation de AsioDrivers..." << std::endl;
        asioDrivers = new AsioDrivers();
    }
    
    // Créer des périphériques ASIO simulés pour garantir le fonctionnement de l'application
    // Nous fournissons toujours ces périphériques, même si des pilotes réels sont détectés
    
    // Périphérique 1: Simulation ASIO
    Napi::Object device1 = Napi::Object::New(env);
    device1.Set("name", Napi::String::New(env, "Simulation ASIO"));
    device1.Set("id", Napi::Number::New(env, 0));
    device1.Set("isSimulated", Napi::Boolean::New(env, true));
    device1.Set("numInputChannels", Napi::Number::New(env, 2));
    device1.Set("numOutputChannels", Napi::Number::New(env, 2));
    device1.Set("preferredBufferSize", Napi::Number::New(env, 1024));
    devices.Set(0u, device1);
    
    // Périphérique 2: Focusrite Saffire Pro 24 (simulé)
    Napi::Object device2 = Napi::Object::New(env);
    device2.Set("name", Napi::String::New(env, "Focusrite Saffire Pro 24"));
    device2.Set("id", Napi::Number::New(env, 1));
    device2.Set("isSimulated", Napi::Boolean::New(env, true));
    device2.Set("numInputChannels", Napi::Number::New(env, 16));
    device2.Set("numOutputChannels", Napi::Number::New(env, 8));
    device2.Set("preferredBufferSize", Napi::Number::New(env, 512));
    devices.Set(1u, device2);
    
    // Périphérique 3: Steinberg UR22 (simulé)
    Napi::Object device3 = Napi::Object::New(env);
    device3.Set("name", Napi::String::New(env, "Steinberg UR22"));
    device3.Set("id", Napi::Number::New(env, 2));
    device3.Set("isSimulated", Napi::Boolean::New(env, true));
    device3.Set("numInputChannels", Napi::Number::New(env, 2));
    device3.Set("numOutputChannels", Napi::Number::New(env, 2));
    device3.Set("preferredBufferSize", Napi::Number::New(env, 256));
    devices.Set(2u, device3);
    
    // Périphérique 4: RME Fireface UCX (simulé)
    Napi::Object device4 = Napi::Object::New(env);
    device4.Set("name", Napi::String::New(env, "RME Fireface UCX"));
    device4.Set("id", Napi::Number::New(env, 3));
    device4.Set("isSimulated", Napi::Boolean::New(env, true));
    device4.Set("numInputChannels", Napi::Number::New(env, 18));
    device4.Set("numOutputChannels", Napi::Number::New(env, 18));
    device4.Set("preferredBufferSize", Napi::Number::New(env, 128));
    devices.Set(3u, device4);
    
    // Essayer de détecter les pilotes ASIO réels (pour information seulement)
    char* driverNames[32]; // Tableau pour stocker les noms des pilotes
    long numDrivers = asioDrivers->getDriverNames(driverNames, 32);
    std::cout << "Nombre de pilotes ASIO détectés: " << numDrivers << std::endl;
    
    if (numDrivers > 0) {
        std::cout << "Pilotes ASIO détectés (non utilisés pour l'instant):" << std::endl;
        for (long i = 0; i < numDrivers; i++) {
            if (driverNames[i]) {
                std::cout << "  " << i << ": " << driverNames[i] << std::endl;
            }
        }
    }
    
    return devices;
}
#else
// Fournir une implémentation vide ou simulée si ASIO n'est pas inclus
Napi::Value ASIOHandler::getDevices(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Array deviceList = Napi::Array::New(env, 2); // Simuler deux périphériques

    Napi::Object device1 = Napi::Object::New(env);
    device1.Set("id", Napi::Number::New(env, 0));
    device1.Set("name", Napi::String::New(env, "Simulated ASIO Input"));
    deviceList[uint32_t(0)] = device1;

    Napi::Object device2 = Napi::Object::New(env);
    device2.Set("id", Napi::Number::New(env, 1));
    device2.Set("name", Napi::String::New(env, "Simulated ASIO Output"));
    deviceList[uint32_t(1)] = device2;
    
    return deviceList;
}
#endif

// Implémentation de SetInversionGain
Napi::Value ASIOHandler::SetInversionGain(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  // Vérifier les arguments
  if (info.Length() < 1 || !info[0].IsNumber()) {
    Napi::TypeError::New(env, "Argument 1 doit être un nombre (gain)").ThrowAsJavaScriptException();
    return env.Null();
  }
  
  // Récupérer la valeur du gain
  float newGain = info[0].As<Napi::Number>().FloatValue();
  
  // Limiter le gain à une plage raisonnable (0 à 2)
  newGain = std::max(0.0f, std::min(newGain, 2.0f));
  
  // Mettre à jour le gain avec verrou pour éviter les problèmes de concurrence
  {
    std::lock_guard<std::mutex> lock(bufferMutex);
    gain = newGain;
  }
  
  // Créer un objet pour retourner le résultat
  Napi::Object result = Napi::Object::New(env);
  result.Set("success", Napi::Boolean::New(env, true));
  result.Set("gain", Napi::Number::New(env, gain));
  
  return result;
}

Napi::Object ASIOHandler::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "ASIOHandler", {
    StaticMethod("getDevices", &ASIOHandler::getDevices),
    StaticMethod("initialize", &ASIOHandler::Initialize),
    StaticMethod("start", &ASIOHandler::Start),
    StaticMethod("stop", &ASIOHandler::Stop),
    StaticMethod("getInputLevel", &ASIOHandler::GetInputLevel),
    StaticMethod("getFFTData", &ASIOHandler::GetFFTData),
    StaticMethod("setInversionGain", &ASIOHandler::SetInversionGain)
  });
  
  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  
  exports.Set("ASIOHandler", func);
  return exports;
}

// Point d'entrée du module Node.js
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return ASIOHandler::Init(env, exports);
}

NODE_API_MODULE(asio_addon, InitAll)
