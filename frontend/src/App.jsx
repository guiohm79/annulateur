import React, { useState, useEffect } from 'react';

function App() {
  const [status, setStatus] = useState({ server: 'disconnected', asio: { initialized: false, processing: false } });
  const [gain, setGain] = useState(1.0);
  const [error, setError] = useState('');
  const [devices, setDevices] = useState([]);
  const [selectedInputDevice, setSelectedInputDevice] = useState('');
  const [selectedOutputDevice, setSelectedOutputDevice] = useState('');
  const [inputLevel, setInputLevel] = useState(0);
  const [fftData, setFftData] = useState(Array(32).fill(0));

  const API_URL = 'http://localhost:3002/api';

  // Vérifier l'état du serveur et charger les périphériques
  useEffect(() => {
    checkServerStatus();
    loadDevices();
  }, []);

  // Polling pour les mises à jour des niveaux audio quand le traitement est actif
  useEffect(() => {
    let interval;
    if (status.asio?.processing) {
      interval = setInterval(() => {
        getInputLevel();
        getFFTData();
      }, 100);
    }

    return () => {
      if (interval) clearInterval(interval);
    };
  }, [status.asio?.processing]);

  const checkServerStatus = async () => {
    try {
      const response = await fetch(`${API_URL}/status`);
      const data = await response.json();
      setStatus(data);
      console.log('Statut du serveur:', data);
    } catch (err) {
      setError('Erreur de connexion au serveur. Assurez-vous que le backend est démarré.');
      console.error('Erreur de statut:', err);
    }
  };

  const loadDevices = async () => {
    try {
      const response = await fetch(`${API_URL}/devices`);
      const data = await response.json();
      console.log('Réponse API périphériques:', data);
      
      if (data.devices && Array.isArray(data.devices)) {
        setDevices(data.devices);
        
        // Sélectionner par défaut le premier périphérique d'entrée et de sortie
        const inputDevices = data.devices.filter(device => device.isInput);
        const outputDevices = data.devices.filter(device => !device.isInput);
        
        if (inputDevices.length > 0 && !selectedInputDevice) {
          setSelectedInputDevice(inputDevices[0].id);
        }
        
        if (outputDevices.length > 0 && !selectedOutputDevice) {
          setSelectedOutputDevice(outputDevices[0].id);
        }
      } else {
        console.warn('Aucun périphérique audio reçu ou format incorrect');
      }
    } catch (err) {
      console.error('Erreur de chargement des périphériques audio:', err);
      // En cas d'erreur, définir des périphériques par défaut pour démonstration
      const defaultDevices = [
        { id: "input1", name: "Microphone intégré", isInput: true },
        { id: "input2", name: "Entrée ligne", isInput: true },
        { id: "input3", name: "Microphone (Realtek Audio)", isInput: true },
        { id: "input4", name: "Microphone (USB)", isInput: true },
        { id: "output1", name: "Haut-parleurs intégrés", isInput: false },
        { id: "output2", name: "Sortie HDMI", isInput: false },
        { id: "output3", name: "Haut-parleurs (Realtek Audio)", isInput: false },
        { id: "output4", name: "Casque (USB)", isInput: false }
      ];
      setDevices(defaultDevices);
      
      // Sélectionner par défaut le premier périphérique d'entrée et de sortie
      const inputDevices = defaultDevices.filter(device => device.isInput);
      const outputDevices = defaultDevices.filter(device => !device.isInput);
      
      if (inputDevices.length > 0 && !selectedInputDevice) {
        setSelectedInputDevice(inputDevices[0].id);
      }
      
      if (outputDevices.length > 0 && !selectedOutputDevice) {
        setSelectedOutputDevice(outputDevices[0].id);
      }
    }
  };

  const initializeASIO = async () => {
    try {
      const response = await fetch(`${API_URL}/initialize`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' }
      });
      const data = await response.json();
      if (data.success) {
        checkServerStatus();
      }
    } catch (err) {
      setError('Erreur d\'initialisation ASIO');
      console.error('Erreur ASIO:', err);
    }
  };

  const startProcessing = async () => {
    try {
      const response = await fetch(`${API_URL}/start`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ 
          gain, 
          inputDeviceId: selectedInputDevice,
          outputDeviceId: selectedOutputDevice 
        })
      });
      const data = await response.json();
      if (data.success) {
        checkServerStatus();
      }
    } catch (err) {
      setError('Erreur de démarrage du traitement');
      console.error('Erreur de démarrage:', err);
    }
  };

  const stopProcessing = async () => {
    try {
      const response = await fetch(`${API_URL}/stop`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' }
      });
      const data = await response.json();
      if (data.success) {
        checkServerStatus();
      }
    } catch (err) {
      setError('Erreur d\'arrêt du traitement');
      console.error('Erreur d\'arrêt:', err);
    }
  };

  const getInputLevel = async () => {
    try {
      const response = await fetch(`${API_URL}/input-level`);
      const data = await response.json();
      setInputLevel(data.level);
    } catch (err) {
      console.error('Erreur de lecture du niveau:', err);
    }
  };

  const getFFTData = async () => {
    try {
      const response = await fetch(`${API_URL}/fft-data`);
      const data = await response.json();
      setFftData(data.data);
    } catch (err) {
      console.error('Erreur de lecture FFT:', err);
    }
  };

  return (
    <div className="min-h-screen flex items-center justify-center p-4">
      <div className="max-w-2xl w-full bg-slate-800 rounded-xl shadow-lg p-6 space-y-4">
        <h1 className="text-2xl font-bold text-blue-400 text-center">Annulateur de Bruit</h1>
        
        {error && (
          <div className="bg-red-900 text-white p-3 rounded-md text-center">
            {error}
            <button 
              className="ml-2 underline" 
              onClick={() => setError('')}
            >
              Fermer
            </button>
          </div>
        )}

        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div className="space-y-4">
            <div className="p-4 bg-slate-700 rounded-lg flex items-center justify-between">
              <span className="text-white">Statut du serveur:</span>
              <span className={status.server === 'running' ? 'text-green-500' : 'text-red-500'}>
                {status.server === 'running' ? 'En ligne' : 'Hors ligne'}
              </span>
            </div>

            <div className="p-4 bg-slate-700 rounded-lg flex items-center justify-between">
              <span className="text-white">Statut ASIO:</span>
              <span className={status.asio?.initialized ? 'text-green-500' : 'text-yellow-500'}>
                {status.asio?.initialized ? 'Initialisé' : 'Non initialisé'}
              </span>
            </div>

            <div className="p-4 bg-slate-700 rounded-lg flex items-center justify-between">
              <span className="text-white">Traitement:</span>
              <span className={status.asio?.processing ? 'text-green-500' : 'text-red-500'}>
                {status.asio?.processing ? 'Actif' : 'Inactif'}
              </span>
            </div>

            {/* Sélection des périphériques audio */}
            <div className="p-4 bg-slate-700 rounded-lg space-y-4">
              {/* Périphérique d'entrée */}
              <div className="space-y-2">
                <label className="text-white block font-medium">Périphérique d'entrée:</label>
                <select 
                  className="w-full p-2 rounded bg-slate-600 text-white"
                  value={selectedInputDevice}
                  onChange={(e) => setSelectedInputDevice(e.target.value)}
                  disabled={status.asio?.processing}
                >
                  {devices
                    .filter(device => device.isInput)
                    .map(device => (
                      <option key={device.id} value={device.id}>
                        {device.name}
                      </option>
                    ))}
                </select>
              </div>
              
              {/* Périphérique de sortie */}
              <div className="space-y-2">
                <label className="text-white block font-medium">Périphérique de sortie:</label>
                <select 
                  className="w-full p-2 rounded bg-slate-600 text-white"
                  value={selectedOutputDevice}
                  onChange={(e) => setSelectedOutputDevice(e.target.value)}
                  disabled={status.asio?.processing}
                >
                  {devices
                    .filter(device => !device.isInput)
                    .map(device => (
                      <option key={device.id} value={device.id}>
                        {device.name}
                      </option>
                    ))}
                </select>
              </div>
              
              <div className="text-xs text-slate-400 mt-1">
                Pour l'annulation de bruit, choisissez une entrée pour capter le son et une sortie pour émettre le son inversé.
              </div>
            </div>
          </div>

          <div className="space-y-4">
            {/* Visualisation du niveau d'entrée */}
            <div className="p-4 bg-slate-700 rounded-lg space-y-2">
              <label className="text-white block">Niveau d'entrée:</label>
              <div className="w-full bg-slate-600 rounded-full h-4">
                <div 
                  className="bg-blue-500 h-4 rounded-full transition-all duration-100" 
                  style={{ width: `${inputLevel}%` }}
                ></div>
              </div>
              <div className="text-right text-xs text-slate-400 mt-1">{Math.round(inputLevel)}%</div>
            </div>

            {/* Visualisation FFT */}
            <div className="p-4 bg-slate-700 rounded-lg space-y-2">
              <label className="text-white block">Analyse spectrale:</label>
              <div className="w-full h-32 flex items-end space-x-0.5">
                {fftData.map((value, index) => (
                  <div 
                    key={index}
                    className="bg-gradient-to-t from-blue-700 to-purple-500 w-full transition-all duration-100"
                    style={{ height: `${value}%` }}
                  ></div>
                ))}
              </div>
            </div>
          </div>
        </div>

        <div className="flex flex-col space-y-2">
          <label className="text-white">Gain d'inversion (amplitude):</label>
          <input 
            type="range" 
            min="0" 
            max="2" 
            step="0.1"
            value={gain}
            onChange={(e) => setGain(parseFloat(e.target.value))}
            className="w-full"
          />
          <div className="text-white text-center">{gain.toFixed(1)}</div>
        </div>

        <div className="flex justify-center space-x-4 flex-wrap">
          <button 
            className="px-4 py-2 bg-blue-600 rounded-md text-white hover:bg-blue-700 disabled:opacity-50"
            onClick={checkServerStatus}
          >
            Rafraîchir
          </button>

          <button 
            className="px-4 py-2 bg-blue-600 rounded-md text-white hover:bg-blue-700 disabled:opacity-50"
            onClick={initializeASIO}
            disabled={status.asio?.initialized || status.server !== 'running'}
          >
            Initialiser ASIO
          </button>
          
          {!status.asio?.processing ? (
            <button 
              className="px-4 py-2 bg-green-600 rounded-md text-white hover:bg-green-700 disabled:opacity-50"
              onClick={startProcessing}
              disabled={!status.asio?.initialized || status.server !== 'running'}
            >
              Démarrer
            </button>
          ) : (
            <button 
              className="px-4 py-2 bg-red-600 rounded-md text-white hover:bg-red-700"
              onClick={stopProcessing}
            >
              Arrêter
            </button>
          )}
        </div>

        <div className="text-center text-sm text-slate-400 pt-4">
          <p>Annulateur de bruit par inversion de phase - Version 1.0</p>
        </div>
      </div>
    </div>
  );
}

export default App;
