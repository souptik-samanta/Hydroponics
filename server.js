const express = require('express'); 
const axios = require('axios'); 
const fs = require('fs'); 
const app = express(); 
const port = 6969; 
 
const esp32URL = 'http://192.168.4.1/get'; // ESP32 Access Point IP 
const jsonPath = './sensor_data.json'; 
 
let sensorData = { 
  ph: null, 
  tds: null, 
  temp: null, 
  humidity: null, 
  pump: false, 
  time: new Date().toISOString() 
}; 
 
//every 10 sex
async function pollESP32() { 
  try { 
    const response = await axios.get(esp32URL); 
    const data = response.data; 
 
    sensorData = { 
      ...data, 
      time: new Date().toISOString() 
    }; 
 
    fs.writeFileSync(jsonPath, JSON.stringify(sensorData, null, 2)); 
    console.log(' Fetched from ESP32:', sensorData); 
  } catch (error) { 
    console.error(' Error fetching from ESP32:', error.message); 
  } 
} 
 
//10 sec intrval
setInterval(pollESP32, 10 * 1000); 
 
// /api / get both give same shiz
// but recommended to use /api for API calls /get was for me  
app.get('/get', (req, res) => { 
  res.json(sensorData); 
}); 

app.get('/api', (req, res) => {
  try {
    // Read the latest data from JSON file
    if (fs.existsSync(jsonPath)) {
      const fileData = fs.readFileSync(jsonPath, 'utf8');
      const currentData = JSON.parse(fileData);
      res.json({
        success: true,
        ph: currentData.ph,
        tds: currentData.tds,
        temp: currentData.temp,
        humidity: currentData.humidity,
        pump: currentData.pump,
        time: currentData.time
      });
    } else {
      // If file doesn't exist, return in-memory data
      res.json({
        success: true,
        ph: sensorData.ph,
        tds: sensorData.tds,
        temp: sensorData.temp,
        humidity: sensorData.humidity,
        pump: sensorData.pump,
        time: sensorData.time
      });
    }
  } catch (error) {
    console.error(' Error reading sensor data:', error.message);
    res.status(500).json({
      success: false,
      error: 'Failed to read sensor data',
      time: new Date().toISOString()
    });
  }
});
 
app.listen(port, () => { 
  console.log(` Server running at http://localhost:${port}`); 
  console.log(` API endpoint available at http://localhost:${port}/api`);
  pollESP32(); // firs call to esp32
});