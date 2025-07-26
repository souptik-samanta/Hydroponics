# ESP32 Hydroponics Node.js API Server

A Node.js server that collects sensor data from ESP32 and provides a simple API for other applications to access real-time hydroponics data.

## What This Does

- **Automatically fetches** sensor data from ESP32 every 10 seconds
- **Saves data** to a JSON file for backup
- **Provides API endpoint** at port 6969 for other programs to get current readings
- **Controls water pump** through ESP32 commands

## Setup Instructions

### Step 1: Install Node.js
1. Download and install Node.js from [nodejs.org](https://nodejs.org)
2. Verify installation by opening terminal/command prompt and typing:
   ```bash
   node --version
   npm --version
   ```

### Step 2: Setup Project
1. Create a new folder for your project
2. Save the Node.js server code as `server.js`
3. Open terminal/command prompt in that folder
4. Install required packages:
   ```bash
   npm install express axios
   ```

### Step 3: Connect to ESP32
1. **Power on your ESP32** - It creates a WiFi network called "ESP32-Hydroponics"
2. **Connect your computer** to this WiFi network:
   - Network: `ESP32-Hydroponics`
   - Password: `12345678`

### Step 4: Start the Server
1. In terminal, run:
   ```bash
   node server.js
   ```
2. You should see:
   ```
   Server running at http://localhost:6969
   API endpoint available at http://localhost:6969/api
   Fetched from ESP32: {sensor data}
   ```

## API Usage

### 1. Get Current Sensor Data
**Endpoint:** `GET http://localhost:6969/api`

**Example using browser:**
- Open browser and go to: `http://localhost:6969/api`

**Example using curl:**
```bash
curl http://localhost:6969/api
```

**Example using JavaScript:**
```javascript
fetch('http://localhost:6969/api')
  .then(response => response.json())
  .then(data => {
    console.log('pH Level:', data.ph);
    console.log('Water Quality (TDS):', data.tds);
    console.log('Temperature:', data.temp + '°C');
    console.log('Humidity:', data.humidity + '%');
    console.log('Pump Status:', data.pump ? 'ON' : 'OFF');
  });
```

**Response Example:**
```json
{
  "success": true,
  "ph": 6.8,
  "tds": 350.5,
  "temp": 24.2,
  "humidity": 65.0,
  "pump": true,
  "time": "2025-07-27T10:30:15.000Z"
}
```

### 2. Control Water Pump
**Endpoint:** `POST http://192.168.4.1/pump` (Direct to ESP32)

**Turn Pump ON:**
```bash
curl -X POST "http://192.168.4.1/pump" -d "status=on"
```

**Turn Pump OFF:**
```bash
curl -X POST "http://192.168.4.1/pump" -d "status=off"
```

**JavaScript Function Example:**
```javascript
const { exec } = require('child_process');

function controlPump(status) {
  if (status !== 'on' && status !== 'off') {
    console.error("Invalid input. Use 'on' or 'off'.");
    return;
  }
  
  const command = `curl -X POST "http://192.168.4.1/pump" -d "status=${status}"`;
  
  exec(command, (error, stdout, stderr) => {
    if (error) {
      console.error(`Error: ${error.message}`);
      return;
    }
    if (stderr) {
      console.error(`stderr: ${stderr}`);
      return;
    }
    console.log(`Pump ${status.toUpperCase()}: ${stdout}`);
  });
}

// Usage examples:
controlPump('on');   // Turn pump ON
controlPump('off');  // Turn pump OFF
```



### Understanding the Data

**What the heack does each values Means:**
- **success**: Whether the request was successful (true/false)
- **ph**: Water acidity level (6.0-7.0 is ideal for most plants)
- **tds**: Total dissolved solids in water (lower = cleaner water)
- **temp**: Air temperature in Celsius (20-25°C is optimal)
- **humidity**: Air humidity percentage (50-70% is good)
- **pump**: Whether water pump is running (true/false)
- **time**: When the reading was taken (ISO format)

## Troubleshooting

### Common Issues:

**Server Won't Start:**
- Make sure Node.js is installed: `node --version`
- Install dependencies: `npm install express axios`
- Check if port 6969 is already in use

**Can't Get Sensor Data:**
- Ensure you're connected to "ESP32-Hydroponics" WiFi network
- Check ESP32 is powered on and running
- Wait 10 seconds for first reading after starting server

**Pump Control Not Working:**
- Make sure you're connected to ESP32's WiFi network
- ESP32 must be running and accessible at 192.168.4.1
- Check ESP32 serial monitor for error messages

**API Returns Error:**
- Server automatically retries every 10 seconds
- Check ESP32 WiFi connection and power
- Look at server console for error messages

## Quick Test Commands

**Test sensor data:**
```bash
curl http://localhost:6969/api
```

**Test pump control:**
```bash
# Turn pump ON
curl -X POST "http://192.168.4.1/pump" -d "status=on"

# Turn pump OFF  
curl -X POST "http://192.168.4.1/pump" -d "status=off"
```

## Files Created
- `sensor_data.json` - Automatically created to store latest readings
- Server updates this file every 10 seconds with fresh data# Hydroponics
