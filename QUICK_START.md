# OBD2 Emulator - Quick Start Guide

## 🚀 Get Started in 5 Minutes

### Step 1: Hardware Setup
1. **Connect your Waveshare RP2350-CAN board** to your computer via USB-C
2. **Optional**: Connect a button between GPIO 22 and GND for manual control

### Step 2: Flash the Firmware
1. **Download** the pre-built firmware: `build/obd2_emulator.uf2`
2. **Hold BOOTSEL** button on the RP2350-CAN board
3. **Connect USB** while holding BOOTSEL
4. **Copy** `obd2_emulator.uf2` to the RPI-RP2 drive that appears
5. **Board will reboot** automatically and start the OBD2 emulator

### Step 3: Connect Your OBD2 Scanner
```
RP2350-CAN Board    →    OBD2 Scanner/Tool
CAN_H               →    CAN_H
CAN_L               →    CAN_L  
GND                 →    GND
```

### Step 4: Test the Connection
1. **Open serial terminal** (115200 baud) to see real-time data
2. **Connect your OBD2 scanner** to the CAN bus
3. **Scan for vehicle** - the emulator will respond as a Honda Civic
4. **Read live data** - all parameters update in real-time

## 📊 What You'll See

### USB Serial Output (Automatic)
```
=== Real-Time Vehicle Data ===
Engine RPM:     2340 RPM
Vehicle Speed:    67 km/h
Coolant Temp:     92°C
Throttle Pos:     45%
Engine Load:      52%
Fuel Level:       74%
VIN:              1HGBH41JXMN109186
Engine Runtime:   1247 sec

--- Advanced Parameters ---
MAF Flow Rate:  15.23 g/s
Fuel Pressure:  325.4 kPa
Manifold Press:  45.2 kPa
O2 Sensor B1S1: 0.456 V
O2 Sensor B1S2: 0.423 V
Short Fuel Trim:  +3%
Long Fuel Trim:   +1%
Timing Advance:  +18°
==============================
```

### OBD2 Scanner Display
Your scanner will show:
- **Vehicle**: Honda Civic (VIN: 1HGBH41JXMN109186)
- **Engine Data**: RPM, Speed, Temperature, Load
- **Advanced Parameters**: MAF, Fuel Pressure, O2 Sensors, Fuel Trim
- **Diagnostic Codes**: Realistic DTCs based on driving conditions

## 🎮 Interactive Commands

Type these commands in the USB serial terminal:

| Command | Function |
|---------|----------|
| `v` | Show current vehicle data |
| `p` | List all available OBD2 PIDs |
| `d` | Display current DTCs |
| `s` | Show system statistics |
| `x` | Clear all DTCs |
| `h` | Help menu |

## 🚨 Simulate Faults

Test your diagnostic tools with realistic fault scenarios:

| Command | Fault Simulation |
|---------|------------------|
| `c` | Cold start issues (P0125, P0110) |
| `e` | Emissions failure (P0420, P0430) |
| `f` | Fuel system issues (P0171, P0172) |
| `i` | Ignition misfires (P0300-P0303) |

## 🔧 Automatic Features

The emulator runs completely automatically:

✅ **Real-time parameter updates** (50ms for critical data)  
✅ **Automatic DTC generation** based on vehicle conditions  
✅ **Continuous USB logging** every 3 seconds  
✅ **Professional-grade accuracy** for all parameters  
✅ **Compatible with all OBD2 scanners** (ELM327, professional tools)  

## 📈 Supported Parameters

### Basic OBD2 Data
- Engine RPM (0-6500 RPM)
- Vehicle Speed (0-200 km/h)
- Coolant Temperature (85-95°C)
- Throttle Position (0-100%)
- Engine Load (15-100%)
- Fuel Level (0-100%)

### Advanced Parameters
- **MAF Flow Rate** (0.5-50 g/s)
- **Fuel Pressure** (250-400 kPa)
- **Manifold Pressure** (20-105 kPa)
- **O2 Sensors** (0.1-0.9V, realistic switching)
- **Fuel Trim** (±25%, closed-loop control)
- **Timing Advance** (-5° to +35°)

## 🎯 Perfect For

- **Testing OBD2 scanners** and diagnostic tools
- **Developing automotive software** applications
- **Learning OBD2 protocols** and vehicle diagnostics
- **Quality assurance** for diagnostic equipment
- **Educational projects** in automotive technology

## ⚡ Troubleshooting

**Scanner not connecting?**
- Check CAN_H and CAN_L wiring
- Ensure scanner uses 500 kbps CAN speed
- Verify ground connection

**No data showing?**
- Confirm emulator is powered and running
- Check USB serial output for activity
- Try different OBD2 scanner software

**Build issues?**
- Ensure Pico SDK 2.2.0+ is installed
- Check CMake and ARM GCC toolchain
- Verify all source files are present

## 📞 Need Help?

1. **Check the full README.md** for detailed documentation
2. **Use the 'h' command** in serial terminal for help
3. **Monitor USB serial output** for real-time diagnostics
4. **Test with multiple scanners** to verify compatibility

---

**Ready to test your OBD2 tools with professional-grade vehicle simulation!** 🚗⚡
