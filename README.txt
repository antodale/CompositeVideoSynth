# Retro Video Synthesizer: ESP32 & Pure Data

Welcome to the Retro Video Synthesizer project. This tool allows you to generate live, dynamic composite video art directly from an ESP32 microcontroller, controlled in real-time by a custom Pure Data interface. 

Whether you are performing live visuals, circuit bending, or just exploring retro hardware graphics, this project provides a lightweight, highly responsive bridge between your computer and a classic CRT television and a user friendly interface for generating your visuals and syncing them to your music.

## Project Overview

This system is divided into two main components:
1. **The Hardware (ESP32):** Runs a custom C++ firmware based on Bitluni's Composite Video library. It acts as the graphics engine, rendering 3D wireframes, starfields, tunnels, and cascading text directly to a composite video signal.
2. **The Software (Pure Data):** Acts as the control surface. It takes your physical mouse movements (or physical MIDI controller faders) and translates them into a highly optimized stream of raw ASCII bytes sent over USB to command the ESP32 in real-time.

## Hardware Requirements

To get this project running, you will need:
* **An ESP32 Development Board:** The standard ESP32 WROOM-32 (30-pin or 38-pin) is highly recommended. 
* **A Composite Video Cable (RCA):** To connect the ESP32 to your screen, you need a chip version that still has the DAC (NODE MCU, WROOM 32, ...).
* **A Display:** A vintage CRT television or any monitor with an analog composite video input.
* **A MIDI Controller (Optional but recommended):** For tactile, hands-on control of the visual parameters.

*Note: You will need to wire the specific composite video output pin (pin 21) of your ESP32 to the center pin of the RCA cable, and connect the grounds together. Please refer to the Bitluni Composite Video documentation for the safest resistor/DAC wiring schematic for your specific board.*

## Software Dependencies

Before opening the files, ensure you have the following installed:
* **Arduino IDE:** Configured for the ESP32 board manager (use the esp32 by espressif, version 2.0.0).
* **Pure Data:** The standard Vanilla version works perfectly.

## The Scenes

The ESP32 is programmed with several distinct visual modes, known as Scenes. Each scene reacts differently to the sliders in Pure Data.

* **Scene A (Solid Colors):** Generates full-screen solid blocks of color or grayscale.
* **Scene B (Cascading Text):** A digital rain effect where custom text cascades down the screen.
* **Scene C (3D Wireframes):** Renders rotating 3D cubes, pyramids, and spheres in customizable grid layouts.
* **Scene D (Hyper Tunnels):** Draws infinite, accelerating geometric line tunnels.
* **Scene E (Starfield):** A classic warp-speed 3D starfield simulation.

## How to Use the Synthesizer

### Step 1: Flash the ESP32
1. Open the provided `.ino` file in the Arduino IDE.
2. Ensure your board settings match your specific ESP32 model.
3. Upload the code to the board. 
4. Once uploaded, connect your ESP32 to your CRT television. You should see the default startup screen.

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

### Adding Physical MIDI Control
If you want to step away from the mouse and use physical hardware faders:
1. Plug your USB MIDI controller into your computer and select it in Pure Data's `Media > MIDI Settings` menu.
2. The patch includes `[ctlin]` objects. Update the numbers inside these objects to match the Continuous Controller (CC) ID numbers of your physical faders.
3. Moving your physical faders will now smoothly drive the on-screen Pure Data sliders, which instantly update the TV visuals.

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