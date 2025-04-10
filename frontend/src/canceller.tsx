import React, { useState, useEffect, useRef, useCallback } from 'react';
import { VUMeter } from './components/VUMeter';

// Importation du module natif (sera disponible après compilation)
// const { ASIOHandler } = require('../../backend/build/Release/asio_addon.node');

// Simulation des périphériques ASIO pour le développement
const MOCK_ASIO_DEVICES = [
  { id: 'asio_device_1', name: 'Focusrite USB ASIO' },
  { id: 'asio_device_2', name: 'ASIO4ALL v2' },
  { id: 'asio_device_3', name: 'Realtek ASIO' },
];

// Simulation des nombres de canaux par périphérique
const MOCK_CHANNEL_COUNTS: { [key: string]: { inputs: number; outputs: number } } = {
  asio_device_1: { inputs: 8, outputs: 8 },
  asio_device_2: { inputs: 2, outputs: 2 },
  asio_device_3: { inputs: 2, outputs: 2 },
};

const PhaseReversalNoiseCanceller: React.FC = () => {
  // État de l'application
  const [isEnabled, setIsEnabled] = useState<boolean>(false);
  const [selectedAsioDevice, setSelectedAsioDevice] = useState<string | null>(null);
  const [inputChannel, setInputChannel] = useState<number>(1);
  const [outputChannel, setOutputChannel] = useState<number>(1);
  const [noiseReductionLevel, setNoiseReductionLevel] = useState<number>(100);
  const [status, setStatus] = useState<'Idle' | 'Initializing' | 'Active' | 'Error'>('Idle');
  const [availableChannels, setAvailableChannels] = useState<{ inputs: number; outputs: number } | null>(null);
  const [error, setError] = useState<string | null>(null);
  
  // Nouveaux états pour la visualisation
  const [inputLevel, setInputLevel] = useState<number>(0);
  const [latency, setLatency] = useState<number>(5.8);
  const [bufferSize, setBufferSize] = useState<number>(5.8);
  const [showFrequencyAnalysis, setShowFrequencyAnalysis] = useState<boolean>(false);
  
  // Références
  const fftCanvasRef = useRef<HTMLCanvasElement>(null);
  const asioHandlerRef = useRef<any>(null);
  const animationFrameRef = useRef<number | null>(null);
  
  // Initialisation du module ASIO
  useEffect(() => {
    // Dans une implémentation réelle, on initialiserait le module natif ici
    // asioHandlerRef.current = new ASIOHandler();
    
    // Nettoyage à la fermeture
    return () => {
      if (animationFrameRef.current) {
        cancelAnimationFrame(animationFrameRef.current);
      }
      // if (asioHandlerRef.current) {
      //   asioHandlerRef.current.stop();
      // }
    };
  }, []);
  
  // Simulation de la récupération des informations ASIO
  useEffect(() => {
    if (selectedAsioDevice) {
      setStatus('Initializing');
      setError(null);
      
      // Simulation d'opération asynchrone
      const timer = setTimeout(() => {
        const channels = MOCK_CHANNEL_COUNTS[selectedAsioDevice];
        if (channels) {
          setAvailableChannels(channels);
          // Réinitialisation des canaux s'ils dépassent les limites du périphérique
          setInputChannel((prev) => Math.min(prev, channels.inputs));
          setOutputChannel((prev) => Math.min(prev, channels.outputs));
          if (isEnabled) {
            setStatus('Active');
          } else {
            setStatus('Idle');
          }
        } else {
          setStatus('Error');
          setError('Impossible d\'obtenir les informations de canaux pour le périphérique sélectionné.');
          setAvailableChannels(null);
          setIsEnabled(false);
        }
      }, 500);
      
      return () => clearTimeout(timer);
    } else {
      setAvailableChannels(null);
      setStatus('Idle');
      setIsEnabled(false);
      setError(null);
    }
  }, [selectedAsioDevice, isEnabled]);
  
  // Animation des niveaux audio
  useEffect(() => {
    if (isEnabled && status === 'Active') {
      const animate = () => {
        // Simulation de niveau d'entrée
        setInputLevel(Math.random() * 80 + 10);
        
        // Dans une implémentation réelle :
        // if (asioHandlerRef.current) {
        //   setInputLevel(asioHandlerRef.current.getInputLevel());
        //   
        //   // Dessin FFT
        //   if (showFrequencyAnalysis && fftCanvasRef.current) {
        //     const ctx = fftCanvasRef.current.getContext('2d');
        //     if (ctx) {
        //       const fftData = asioHandlerRef.current.getFFTData();
        //       drawFFT(ctx, fftData);
        //     }
        //   }
        // }
        
        animationFrameRef.current = requestAnimationFrame(animate);
      };
      
      animate();
      
      return () => {
        if (animationFrameRef.current) {
          cancelAnimationFrame(animationFrameRef.current);
        }
      };
    }
  }, [isEnabled, status, showFrequencyAnalysis]);
  
  // Gestion de l'activation/désactivation
  const handleToggleEnable = () => {
    if (!selectedAsioDevice && !isEnabled) {
      setError("Veuillez d'abord sélectionner un périphérique ASIO.");
      setStatus('Error');
      return;
    }
    
    const newState = !isEnabled;
    setIsEnabled(newState);
    
    if (newState) {
      if (availableChannels) {
        setStatus('Active');
        setError(null);
        // Dans une implémentation réelle :
        // if (asioHandlerRef.current) {
        //   asioHandlerRef.current.start(noiseReductionLevel / 100);
        // }
      } else if (selectedAsioDevice) {
        setStatus('Initializing');
      } else {
        setIsEnabled(false);
        setStatus('Error');
        setError("Impossible d'activer sans périphérique sélectionné.");
      }
    } else {
      setStatus('Idle');
      setError(null);
      // Dans une implémentation réelle :
      // if (asioHandlerRef.current) {
      //   asioHandlerRef.current.stop();
      // }
    }
  };
  
  // Gestion du changement de périphérique
  const handleDeviceChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    const deviceId = event.target.value;
    setSelectedAsioDevice(deviceId === "" ? null : deviceId);
    setIsEnabled(false);
    setInputChannel(1);
    setOutputChannel(1);
    setStatus('Idle');
  };
  
  // Gestion du changement de canal d'entrée
  const handleInputChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    setInputChannel(parseInt(event.target.value, 10));
  };
  
  // Gestion du changement de canal de sortie
  const handleOutputChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    setOutputChannel(parseInt(event.target.value, 10));
  };
  
  // Rendu des options de canaux
  const renderChannelOptions = (count: number | undefined) => {
    if (!count) return null;
    return Array.from({ length: count }, (_, i) => i + 1).map((channelNum) => (
      <option key={channelNum} value={channelNum}>
        Canal {channelNum}
      </option>
    ));
  };
  
  // Couleur du statut
  const getStatusColor = () => {
    switch(status) {
      case 'Active': return 'text-green-500';
      case 'Initializing': return 'text-yellow-500';
      case 'Error': return 'text-red-500';
      case 'Idle':
      default: return 'text-gray-500';
    }
  };
  
  // Dessin FFT (dans une implémentation réelle)
  const drawFFT = (ctx: CanvasRenderingContext2D, data: number[]) => {
    const width = ctx.canvas.width;
    const height = ctx.canvas.height;
    
    ctx.clearRect(0, 0, width, height);
    ctx.fillStyle = '#1e40af';
    
    const barWidth = width / data.length;
    
    for (let i = 0; i < data.length; i++) {
      const barHeight = (data[i] / 100) * height;
      ctx.fillRect(i * barWidth, height - barHeight, barWidth - 1, barHeight);
    }
  };
  
  // Changement de taille de buffer
  const handleBufferSizeChange = (size: number) => {
    setBufferSize(size);
    setLatency(size);
    // Dans une implémentation réelle :
    // if (asioHandlerRef.current) {
    //   asioHandlerRef.current.setBufferSize(size);
    // }
  };
  
  return (
    <div className="min-h-screen bg-slate-900 text-slate-200 p-4 sm:p-6 lg:p-8 flex flex-col items-center">
      <div className="w-full max-w-2xl bg-slate-800 rounded-xl shadow-lg p-6 space-y-6">
        {/* En-tête */}
        <div className="text-center mb-6">
          <h1 className="text-2xl font-bold text-blue-400">Annulateur de Bruit par Inversion de Phase</h1>
          <p className="text-sm text-slate-400 mt-1">(Simulation d'interface - Nécessite un backend ASIO)</p>
        </div>
        
        {/* Indicateur de statut */}
        <div className="flex justify-center items-center space-x-2 p-3 bg-slate-700 rounded-lg">
          <span className="font-semibold">Statut:</span>
          <span className={`font-bold ${getStatusColor()}`}>{status}</span>
          <div className={`w-3 h-3 rounded-full ${
            status === 'Active' ? 'bg-green-500 animate-pulse' :
            status === 'Initializing' ? 'bg-yellow-500 animate-pulse' :
            status === 'Error' ? 'bg-red-500' :
            'bg-gray-500'
          }`}></div>
        </div>
        
        {/* Message d'erreur */}
        {error && (
          <div className="p-3 bg-red-900 border border-red-700 text-red-300 rounded-md text-center text-sm">
            {error}
          </div>
        )}
        
        {/* Contrôles principaux */}
        <div className="space-y-4">
          {/* Activation */}
          <div className="flex items-center justify-between p-4 bg-slate-700 rounded-lg">
            <label htmlFor="enable-toggle" className="font-medium text-lg text-slate-100">
              Annulation de Bruit
            </label>
            <button
              id="enable-toggle"
              onClick={handleToggleEnable}
              className={`relative inline-flex items-center h-6 rounded-full w-11 transition-colors duration-200 ease-in-out focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-offset-slate-800 focus:ring-blue-500 ${
                isEnabled ? 'bg-blue-600' : 'bg-slate-600'
              }`}
              aria-pressed={isEnabled}
            >
              <span
                className={`inline-block w-4 h-4 transform bg-white rounded-full transition-transform duration-200 ease-in-out ${
                  isEnabled ? 'translate-x-6' : 'translate-x-1'
                }`}
              />
            </button>
          </div>
          
          {/* Sélection du périphérique ASIO */}
          <div className="space-y-2">
            <label htmlFor="asio-device" className="block text-sm font-medium text-slate-300">
              Périphérique ASIO
            </label>
            <select
              id="asio-device"
              value={selectedAsioDevice ?? ""}
              onChange={handleDeviceChange}
              className="w-full p-2 rounded-md bg-slate-700 border border-slate-600 focus:ring-2 focus:ring-blue-500 focus:border-blue-500 text-slate-100"
            >
              <option value="" disabled={MOCK_ASIO_DEVICES.length > 0}>-- Sélectionner un périphérique --</option>
              {MOCK_ASIO_DEVICES.map((device) => (
                <option key={device.id} value={device.id}>
                  {device.name}
                </option>
              ))}
              {MOCK_ASIO_DEVICES.length === 0 && <option disabled>Aucun périphérique ASIO trouvé (Simulé)</option>}
            </select>
          </div>
          
          {/* Sélection des canaux */}
          <div className="grid grid-cols-2 gap-4">
            <div className="space-y-2">
              <label htmlFor="input-channel" className="block text-sm font-medium text-slate-300">
                Canal d'entrée (Bruit)
              </label>
              <select
                id="input-channel"
                value={inputChannel}
                onChange={handleInputChange}
                disabled={!availableChannels}
                className="w-full p-2 rounded-md bg-slate-700 border border-slate-600 focus:ring-2 focus:ring-blue-500 focus:border-blue-500 text-slate-100 disabled:opacity-50"
              >
                {renderChannelOptions(availableChannels?.inputs)}
              </select>
            </div>
            <div className="space-y-2">
              <label htmlFor="output-channel" className="block text-sm font-medium text-slate-300">
                Canal de sortie (Anti-bruit)
              </label>
              <select
                id="output-channel"
                value={outputChannel}
                onChange={handleOutputChange}
                disabled={!availableChannels}
                className="w-full p-2 rounded-md bg-slate-700 border border-slate-600 focus:ring-2 focus:ring-blue-500 focus:border-blue-500 text-slate-100 disabled:opacity-50"
              >
                {renderChannelOptions(availableChannels?.outputs)}
              </select>
            </div>
          </div>
          
          {/* Niveau de réduction de bruit */}
          <div className="space-y-2">
            <label htmlFor="noise-reduction" className="block text-sm font-medium text-slate-300">
              Niveau de réduction de bruit: {noiseReductionLevel}%
            </label>
            <input
              id="noise-reduction"
              type="range"
              min="0"
              max="100"
              value={noiseReductionLevel}
              onChange={(e) => setNoiseReductionLevel(parseInt(e.target.value, 10))}
              className="w-full h-2 bg-slate-700 rounded-lg appearance-none cursor-pointer"
            />
          </div>
          
          {/* Visualisation */}
          <div className="grid grid-cols-2 gap-4 mt-6">
            <div className="space-y-4">
              {/* VU-mètre d'entrée */}
              <div className="h-32 bg-slate-700 rounded-lg relative overflow-hidden">
                <div 
                  className="absolute bottom-0 w-full bg-blue-400 transition-all duration-75"
                  style={{ height: `${inputLevel}%` }}
                />
              </div>
              
              {/* Contrôle de latence */}
              <div className="flex items-center justify-between">
                <span className="text-sm text-slate-300">Latence: {latency}ms</span>
                <div className="flex space-x-1">
                  {[1.45, 3.0, 5.8].map((ms) => (
                    <button 
                      key={ms}
                      onClick={() => handleBufferSizeChange(ms)}
                      className={`px-2 py-1 rounded text-xs ${bufferSize === ms ? 'bg-blue-600' : 'bg-slate-600'}`}
                    >
                      {ms}ms
                    </button>
                  ))}
                </div>
              </div>
            </div>
            
            {/* Visualiseur de spectre */}
            <div className="h-32 bg-slate-700 rounded-lg relative">
              <canvas 
                ref={fftCanvasRef}
                className="w-full h-full"
                onClick={() => setShowFrequencyAnalysis(!showFrequencyAnalysis)}
              />
            </div>
          </div>
        </div>
        
        {/* Note de bas de page */}
        <div className="text-center text-xs text-slate-500 pt-4 border-t border-slate-700">
          Note: Ceci est une simulation d'interface. Le traitement audio réel et l'interaction ASIO nécessitent un composant backend ou natif dédié.
        </div>
      </div>
    </div>
  );
};

export default PhaseReversalNoiseCanceller;
