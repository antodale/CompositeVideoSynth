# Retro Video Synthesizer: ESP32 & Pure Data

Welcome to the Retro Video Synthesizer project. This tool allows you to generate live, dynamic composite video art directly from an ESP32 microcontroller, controlled in real-time by a custom Pure Data interface or by web interface. 

Whether you are performing live visuals, circuit bending, or just exploring retro hardware graphics, this project provides a lightweight, highly responsive bridge between your computer and a classic CRT television and a user friendly interface for generating your visuals and syncing them to your music.

## Project Overview

This system is divided into two main components:
1. **The Hardware (ESP32):** Runs a custom C++ firmware based on Bitluni's Composite Video library. It acts as the graphics engine, rendering 3D wireframes, starfields, tunnels, and cascading text directly to a composite video signal. If web interface is preferred, a second esp32 is needed, in my case i used an esp32C3.
2. **The Software (Pure Data):** Acts as the control surface. It takes your physical mouse movements (or physical MIDI controller faders) and translates them into a highly optimized stream of raw ASCII bytes sent over USB to command the ESP32 in real-time.

## Hardware Requirements

To get this project running, you will need:
* **An ESP32 Development Board:** The standard ESP32 WROOM-32 (30-pin or 38-pin) is highly recommended. ESP32 C3 for server.
* **A Composite Video Cable (RCA):** To connect the ESP32 to your screen, you need a chip version that still has the DAC (NODE MCU, WROOM 32, ...).
* **A Display:** A vintage CRT television or any monitor with an analog composite video input.
* **A MIDI Controller (Optional but recommended):** For tactile, hands-on control of the visual parameters.

*Note: You will need to wire the specific composite video output pin (pin 25) of your ESP32 to the center pin of the RCA cable, and connect the grounds together. Please refer to the Bitluni Composite Video documentation for the safest resistor/DAC wiring schematic for your specific board.*

## Software Dependencies

Before opening the files, ensure you have the following installed:
* **Platform.io** .
* **Pure Data:** The standard Vanilla version works perfectly.

## The Scenes

The ESP32 is programmed with several distinct visual modes, known as Scenes. Each scene reacts differently to the sliders in Pure Data.

* **Scene A (Solid Colors):** Generates full-screen solid blocks of color or grayscale.
* **Scene B (Starfield):** A classic warp-speed 3D starfield simulation.
* **Scene C (3D Wireframes):** Renders rotating 3D cubes, pyramids, and spheres in customizable grid layouts.
* **Scene D (Hyper Tunnels):** Draws infinite, accelerating geometric line tunnels.
* **Scene E (Cascading text):** For impactant visuals.
* **Scene F (Long text):** For long messages to display.

## How to Use the Synthesizer

### Step 1: Flash the ESP32
1. Flash the code with PlatformIO
2. Connect pins 25 to the centre pin of the RCA video plug, GND pin to the ground of the plug.
3. For web interface use, connect together the 5v pins (VIN) of the two boards, the GND of the two boards together, connect pin 16 of the rendering board (ESP32-WROOM32) to the pin 4 of the server board (ESP32C3), and the pin 17 of the rendering board to the pin 5 of the server board.
4. Once uploaded, connect your ESP32 to your CRT television. You should see the default startup screen.
5. Connect to the network COMPOSITE_VIDEO_UPLINK that should pop up in your device, go to the page 192.168.4.1 and have fun with the interface! (if you cannot find the page, you probably have to disconnect your data and reload).
6. Alternatively you can connect your puredata patch to your render board via USB, without the need of a server board.

### Step 2: Configure Pure Data
1. Open the provided `.pd` patch.
2. At the bottom of the patch, locate the `[comport]` object. You will need to change the port number to match the USB port your ESP32 is connected to (for example, change it to `[comport 9 115200]` if your board is on COM9).
3. Click the `[open(` message box to establish the connection to the microcontroller. *Note: The ESP32 may briefly reboot when the connection is opened. This is normal.*
4. Turn on the metronome toggle at the top of the patch to begin broadcasting data.

### Step 3: Start VJing
You can now click the radio buttons in the Pure Data patch to switch between scenes. Use the virtual faders to manipulate the visuals live:

* **Size:** Controls the scale of the 3D objects or the thickness of the tunnel lines.
* **Speed:** Controls the rotation, cascade drop rate, or warp speed.
* **Shape:** Morphs the geometry (e.g., changing a cube into a pyramid, or altering the tunnel angle).
* **Multiply:** Increases the density of the scene, such as adding more stars to the starfield or creating a larger grid of 3D shapes.


You should only power through usb ONE board, since the voltage is then passed to the other one via pins connection.

### Adding Physical MIDI Control
If you want to step away from the mouse and use physical hardware faders:
1. Plug your USB MIDI controller into your computer and select it in Pure Data's `Media > MIDI Settings` menu.
2. The patch includes `[ctlin]` objects. Update the numbers inside these objects to match the Continuous Controller (CC) ID numbers of your physical faders.
3. Moving your physical faders will now smoothly drive the on-screen Pure Data sliders, which instantly update the TV visuals.
![PXL_20260320_192842090 RAW-01 MP COVER](https://github.com/user-attachments/assets/70ee1f71-9a6f-436f-9d70-26ab5b8b5843)
![PXL_20260320_201209663 RAW-01 COVER](https://github.com/user-attachments/assets/126401ba-1a51-44a4-9203-f885761e4721)
![PXL_20260320_201201847 RAW-01 MP COVER](https://github.com/user-attachments/assets/82ebea05-21b5-4b6a-9c7f-e41608d28b7a)
![PXL_20260320_201148347 RAW-01 MP COVER](https://github.com/user-attachments/assets/2d14539b-d456-49a7-b038-f2110c93c3f8)
![PXL_20260320_201136670 RAW-01 MP COVER](https://github.com/user-attachments/assets/92d3d6b9-255e-4724-a5a5-992d3dd3b1fa)

## How the Serial Communication Works

To ensure maximum speed and prevent memory crashes on the ESP32, this project bypasses heavy string formatting. Instead, the Pure Data patch translates your slider positions directly into raw ASCII mathematical values. 

The format is:
A: first position is a letter that indicates the scene (A to E)
0: Size
0: Speed
0: Shape
0: Multiplication

For example, selecting Scene A and moving the sliders generates a raw six-byte array (like `65 48 54 51 48 49`). This allows the ESP32 to instantly read the hardware buffer without needing to parse complex text delimiters, ensuring a buttery-smooth framerate on your CRT.

Enjoy making visual art!
