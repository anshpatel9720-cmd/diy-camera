# diy-camera
DIGICAM-8000: A standalone ESP32-CAM retro digital camera featuring a custom 2S Li-ion power system, physical hardware shutter, live MJPEG viewfinder, and an embedded web-based gallery OS.

DIGICAM-8000

DIGICAM-8000 is a custom-built, retro-style digital point-and-shoot camera powered by the ESP32-CAM (OV2640) microcontroller. Operating as a completely standalone device, it broadcasts its own Wi-Fi access point and hosts a local web application to manage a live viewfinder, real-time sensor configurations, and an onboard media gallery.

The hardware architecture bypasses standard power limitations by utilizing a 2S (7.4V) lithium-ion battery pack, a dedicated Type-C BMS charging module, and a step-down buck converter to deliver clean, uninterrupted power during high-current WiFi and SD card operations.
Core Features
Standalone Local Network: Operates entirely offline by broadcasting an independent WPA2 Wi-Fi access point.
Dual-Server Architecture: Runs two asynchronous HTTP servers—Port 81 dedicated exclusively to the live MJPEG stream (preventing UI blocking), and Port 80 handling the control API and web application.
Embedded Web OS: A fully responsive HTML/CSS/JS interface stored directly in flash memory (index.h), allowing users to adjust resolution, contrast, saturation, and exposure, alongside toggling the onboard LED flash.
Over-the-Air Gallery: Built-in SD card playback menu that parses directories, generates image thumbnails, and enables direct downloads or deletions via the browser.
Physical Hardware Shutter: A tactile mechanical switch wired with a pull-up resistor to trigger hardware interrupts, operating cleanly alongside the SD card's 1-bit mode.

Hardware Configuration
Microcontroller: AI-Thinker ESP32-CAM (with PSRAM)
Image Sensor: OV2640 (2 Megapixel)
Storage: MicroSD Card (Mounted in 1-bit mode to free up GPIOs)
Power Source: 2x Lithium-ion cells wired in series (7.4V / 8.4V Peak)
Charging & Protection: 2S Type-C Boost/BMS Charger (e.g., TP5100 / IP2326)
Voltage Regulation: Adjustable Buck Converter (Stepping 7.4V down to 5.0V)
Physical Shutter: 4-pin tactile switch with a 10kΩ pull-up resistor


Circuit Wiring
Power Delivery System
Component A	Connection	Component B
Cell 1 (+)	Series Link	Cell 2 (-)
Cell 2 (+)	Pack Positive (7.4V)	2S Charger Board (B+)
Cell 1 (-)	Pack Negative (GND)	2S Charger Board (B-)
Pack Positive (7.4V)	Input Power	Buck Converter (IN+)
Pack Negative (GND)	Input Ground	Buck Converter (IN-)
Buck Converter (OUT+)	Adjusted to 5.0V	ESP32-CAM (5V Pin)
Buck Converter (OUT-)	Common Ground	ESP32-CAM (GND Pin)
Hardware Shutter (GPIO 13)
Component A	Connection	Component B
10kΩ Resistor (End 1)	Pull-Up Source	ESP32-CAM (3.3V Pin)
10kΩ Resistor (End 2)	Signal Line	ESP32-CAM (GPIO 13)
Tactile Switch (Leg 1)	Signal Line	ESP32-CAM (GPIO 13)
Tactile Switch (Leg 2)	Diagonal to Leg 1	ESP32-CAM (GND Pin)
