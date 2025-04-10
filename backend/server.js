const express = require('express');
const cors = require('cors');
const path = require('path');

// Création du serveur Express
const app = express();
const PORT = 3002; // Changement du port pour éviter les conflits

// Middleware
app.use(cors());
app.use(express.json());

// Utiliser notre adaptateur ASIO (qui fournit le module natif ou la simulation)
const asioInterface = require('./asio_adapter');

// Routes API
app.get('/api/status', (req, res) => {
  res.json({
    server: 'running',
    asio: asioInterface.getStatus()
  });
});

app.get('/api/devices', (req, res) => {
  try {
    const devices = asioInterface.getDevices();
    res.json({ devices });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.post('/api/initialize', (req, res) => {
  try {
    const result = asioInterface.initialize();
    res.json(result);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.post('/api/start', (req, res) => {
  try {
    const { gain, inputDeviceId, outputDeviceId } = req.body;
    const result = asioInterface.start({
      gain,
      inputDeviceId,
      outputDeviceId
    });
    res.json(result);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.post('/api/stop', (req, res) => {
  try {
    const result = asioInterface.stop();
    res.json(result);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.get('/api/input-level', (req, res) => {
  try {
    const level = asioInterface.getInputLevel();
    res.json({ level });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.get('/api/fft-data', (req, res) => {
  try {
    const data = asioInterface.getFFTData();
    res.json({ data });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// Démarrage du serveur
app.listen(PORT, () => {
  console.log(`Serveur backend démarré sur http://localhost:${PORT}`);
  console.log('API ASIO simulée en cours d\'exécution (pas de vrai module ASIO chargé)');
  console.log('Pour utiliser le vrai module ASIO, veuillez compiler le module natif');
});
