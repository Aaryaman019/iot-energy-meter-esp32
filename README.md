# iot-energy-meter-esp32
# IoT Smart Energy Meter — ESP32 + Blynk 2.0

A real-time energy monitoring system that measures AC voltage, current, power, and cumulative energy (kWh) using analog sensors, processes data on an ESP32, and pushes live readings to a cloud dashboard for remote monitoring.

Built as a 1st semester mini project (IDT course) at DSCE, Bangalore. Scored 49/50.

## How It Works

1. **ZMPT101B** (voltage transformer) and **SCT-013** (current transformer) output analog signals proportional to mains voltage and load current
2. Signals are biased to mid-rail (~1.65V) using a resistor divider and filtered with a capacitor for stable ADC readings
3. ESP32 samples both channels — voltage is computed via manual RMS calculation, current via EmonLib
4. Energy (kWh) is integrated over time using `millis()` for accurate time-based accumulation
5. Readings are displayed on a 16×2 I2C LCD and pushed to Blynk 2.0 dashboard over Wi-Fi
6. Blynk connection is non-blocking — LCD and sensors work even without internet

## Hardware

| Component | Model | Role |
|-----------|-------|------|
| Microcontroller | ESP32 Dev Board | Central processing, ADC, Wi-Fi |
| Voltage Sensor | ZMPT101B | Steps down AC mains to measurable analog signal |
| Current Sensor | SCT-013-030 | Non-invasive CT sensor, outputs proportional voltage |
| Display | 16×2 LCD with I2C backpack | Local real-time display |
| Passive Components | 2× 10kΩ, 1× 100Ω resistors, 1× 10µF capacitor | Signal conditioning |

### Pin Mapping

| Signal | ESP32 Pin |
|--------|-----------|
| ZMPT101B analog out | GPIO34 (ADC1_CH6) |
| SCT-013 analog out | GPIO32 (ADC1_CH4) |
| LCD SDA | GPIO21 |
| LCD SCL | GPIO22 |
| Sensor VCC | Vin (5V) |

### Signal Conditioning

Both sensors output AC-coupled signals. A resistor divider (2× 10kΩ) biases the output to ~1.65V so the ESP32's 3.3V ADC can sample the full AC waveform. The 10µF capacitor filters high-frequency noise. The 100Ω resistor serves as a burden resistor for the CT sensor.

## Software

| Tool | Purpose |
|------|---------|
| Arduino IDE | Firmware development and flashing |
| EmonLib | RMS current computation from raw ADC samples |
| Blynk 2.0 | Cloud dashboard — gauges, charts, remote monitoring |
| BlynkTimer | Non-blocking task scheduling (replaces `delay()`) |

### Key Design Decisions

- **Non-blocking Blynk:** Uses `Blynk.config()` instead of `Blynk.begin()` so the system doesn't freeze when Wi-Fi is unavailable
- **Averaged current readings:** 5 consecutive `calcIrms()` calls are averaged to reduce noise
- **Noise floor cutoff:** Current readings below 0.02A are zeroed out (SCT-013-030 noise floor)
- **True kWh calculation:** Energy is computed using `millis()` elapsed time, not arbitrary constants

## Blynk Dashboard

The Blynk 2.0 web/mobile dashboard displays:
- **V0** — Voltage (V) — Gauge
- **V1** — Current (A) — Gauge
- **V2** — Power (W) — Gauge
- **V3** — Energy (kWh) — Gauge
- Time-series charts for all four parameters

## Setup Instructions

1. Clone this repo
2. Open `src/smart_energy_meter.ino` in Arduino IDE
3. Install required libraries: `Blynk`, `EmonLib`, `LiquidCrystal I2C`
4. Select board: **ESP32 Dev Module**
5. Create a Blynk 2.0 template at [blynk.cloud](https://blynk.cloud) with 4 virtual pin datastreams (V0–V3)
6. Replace placeholder credentials in the code:
   ```cpp
   #define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
   #define BLYNK_AUTH_TOKEN     "YOUR_AUTH_TOKEN"
   char ssid[] = "YOUR_WIFI_SSID";
   char pass[] = "YOUR_WIFI_PASSWORD";
   ```
7. Flash to ESP32
8. If LCD is blank: try changing `I2C_ADDR` from `0x27` to `0x3F`

## Repo Structure

```
├── src/
│   └── smart_energy_meter.ino
├── hardware/
│   ├── setup.jpg                 # Breadboard wiring photo
│   ├── lcd-display.jpg           # LCD showing live readings
│   └── blynk-dashboard.png       # Blynk dashboard screenshot
└── README.md
```

## Calibration

The calibration factor in `emon1.current(SCT_PIN, 0.55)` is tuned for a low-power load (~9W LED bulb). For higher loads:

1. Connect a known load (e.g., 100W appliance)
2. Expected current = Wattage / 230
3. Read displayed current from Serial Monitor
4. New factor = `0.55 × (Expected / Displayed)`

## Limitations

- SCT-013-030 is rated for 30A — accuracy drops significantly below ~0.5A
- ESP32 ADC has inherent non-linearity (no software correction applied)
- Energy reading resets on power cycle (no EEPROM persistence in this version)
- Tested with low-power loads only; not validated for high-current applications


## Credits

- EmonLib: [OpenEnergyMonitor](https://github.com/openenergymonitor/EmonLib)
- Blynk Platform: [blynk.io](https://blynk.io)
- University: Dayananda Sagar College of Engineering, Dept. of ECE, Bangalore
