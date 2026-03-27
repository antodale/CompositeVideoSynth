//code by bitluni (send me a high five if you like the code)
#include "esp_pm.h"
#include "CompositeGraphics.h"
#include "Image.h"
#include "CompositeOutput.h"

#include "luni.h"
#include "font6x8.h"

#define RXD2 16 // Connect the C3's TX pin to this pin!
#define TXD2 17 // (We won't actually send anything back to the C3, but Serial2 needs it defined)

#define BOOT_BUTTON 0  // The built-in BOOT button on the ESP32

// Button tracking variables
bool lastButtonState = HIGH;
unsigned long lastPressTime = 0;
// The master scene controller

//PAL MAX, half: 324x268 full: 648x536
//NTSC MAX, half: 324x224 full: 648x448
const int XRES = 320;
const int YRES = 225;
String displayText = "INIT";
//Graphics using the defined resolution for the backbuffer
CompositeGraphics graphics(XRES, YRES);
//Composite output using the desired mode (PAL/NTSC) and twice the resolution.
//It will center the displayed image automatically
CompositeOutput composite(CompositeOutput::NTSC, XRES * 2, YRES * 2);

//image and font from the included headers created by the converter. Each iamge uses its own namespace.
Image<CompositeGraphics> luni0(luni::xres, luni::yres, luni::pixels);

//font is based on ASCII starting from char 32 (space), width end height of the monospace characters.
//All characters are staored in an image vertically. Value 0 is background.
Font<CompositeGraphics> font(6, 8, font6x8::pixels);

#include <soc/rtc.h>
char currentScene = 'C';
int paramBg = 0;     // Index 1: 0 = Black BG, 1 = White BG
int paramSize = 6;   // Index 2: 0-9 (Size multiplier)
int paramSpeed = 3;  // Index 3: 0-9 (Speed multiplier)
int paramShape = 1;  // Index 4: 0 = Cube, 1 = Pyramid
int paramMulti = 1;  // Index 5: 1-9 (Grid Multiplier)
// Global variables for animation and serial
int t = millis();
int j = 0;
int scene = 0;
String customSerialText = "WAITING FOR SERIAL...";


void setup() {
  // 1. Initialize Serial Communication at 115200 baud rate
  Serial.begin(115200);
  Serial.println("ESP32 Composite Video Started!");
  Serial.println("Type 'scene 0' or 'scene 1' to change scenes, or type a custom message.");
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  // button state
  pinMode(BOOT_BUTTON, INPUT_PULLUP);

  //highest clockspeed needed
  esp_pm_lock_handle_t powerManagementLock;
  esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "compositeCorePerformanceLock", &powerManagementLock);
  esp_pm_lock_acquire(powerManagementLock);

  //initializing DMA buffers and I2S
  composite.init();
  //initializing graphics double buffer
  graphics.init();
  //select font
  graphics.setFont(font);

  //running composite output pinned to first core
  xTaskCreatePinnedToCore(compositeCore, "compositeCoreTask", 1024, NULL, 1, NULL, 0);

}

void compositeCore(void *data) {
  while (true) {
    //just send the graphics frontbuffer whithout any interruption
    composite.sendFrameHalfResolution(&graphics.frame);
  }
}

void handleSerial_debug() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    currentScene = 'B';
    displayText = input;
    paramBg = 0;
    paramSize = 0;
    paramSpeed = 0;
    paramShape = 1;
    paramMulti = 1;
  }
}

void handleSerial() {
  if (Serial2.available() > 0) {
    String input = Serial2.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {
      // 1. Grab the Scene (the very first character)
      currentScene = input.charAt(0);

      int len = input.length();

      // --- SCENE B: Text + 4 trailing numbers ---
      // Example: "BHELLO WORLD1234"
      if (currentScene == 'B' && len > 5) {
        // Everything from index 1 up to (length - 4)
        displayText = input.substring(1, len - 5);

        // The last 4 characters are your parameters
        // .charAt() returns the ASCII char, so we subtract '0' to get the actual math integer
        paramSize = input.charAt(len - 4) - '0';
        paramSpeed = input.charAt(len - 3) - '0';
        paramShape = input.charAt(len - 2) - '0';
        paramMulti = input.charAt(len - 1) - '0';
      }

      // --- OTHER SCENES: Just numbers after the letter ---
      // Example: "A12345"
      else if (len >= 6) {
        paramBg = input.charAt(1) - '0';
        paramSize = input.charAt(2) - '0';
        paramSpeed = input.charAt(3) - '0';
        paramShape = input.charAt(4) - '0';
        paramMulti = input.charAt(5) - '0';
      }

      Serial.print("Scene: ");
      Serial.println(currentScene);
      Serial.print("Text: ");
      Serial.println(displayText);
    }
  }
}
void draw() {
  // 1. Determine background and foreground colors based on paramBg
  // (Assuming 54 is white and 0 is black in Bitluni's palette)
  int bgColor = (paramBg == 1) ? 54 : 0;
  int fgColor = (paramBg == 1) ? 0 : 54;

  // Clear the screen with the chosen background color
  graphics.begin(bgColor);
  // Serial.println(currentScene);
  switch (currentScene) {
    case 'A':
      if (paramMulti == 0) {
        // A0000: Whole screen white
        graphics.fillRect(0, 0, XRES, YRES, 54);
      } else if (paramMulti == 1) {
        // A0001: Whole screen gray
        graphics.fillRect(0, 0, XRES, YRES, 20);
      }
      break;


    // --- SCENE B: CASCADING TEXT ---
    case 'B':
      {
        // 1. Map the Parameters
        int numLines = paramMulti + 1;                 // Number of lines visible at once (1 to 10)
        float cascadeSpeed = (paramSpeed + 1) * 0.05;  // How fast the cascade drops

        // Determine row height based on size parameter
        int rowHeight = (paramSize > 4) ? 16 : 8;
        int totalRows = YRES / rowHeight;

        // 2. Calculate the Cascade Position
        static float virtualRow = 0;
        virtualRow += cascadeSpeed;
        int headRow = (int)virtualRow % totalRows;

        // 3. Draw the Visible Lines
        for (int i = 0; i < numLines; i++) {
          int drawRow = headRow - i;
          if (drawRow < 0) {
            drawRow += totalRows;
          }

          int yPos = drawRow * rowHeight;

          // 4. Color Math
          // Bitluni's grayscale color maxes out at 54 (not 255)
          int colorIntensity = (paramBg * 5) + (54 - (i * (54 / numLines)));
          if (colorIntensity > 54) colorIntensity = 54;
          if (colorIntensity < 0) colorIntensity = 0;

          // 5. Library-Specific Drawing
          // Set the color and cursor position FIRST
          graphics.setTextColor(colorIntensity);
          graphics.setCursor(10, yPos);

          // Print the text (casting the Arduino String to the required char*)
          if (paramSize > 2) {
            // Once the fader passes 4, we trigger the custom big font
            // and use the fader's exact value (5 through 9) as the multiplier!
            graphics.printBig((char *)displayText.c_str(), paramSize);
          } else {
            // If the fader is 0 to 4, stick to the standard, unscaled hardware font
            graphics.print((char *)displayText.c_str());
          }
        }
        break;
      }
    // --- SCENE C: 3D WIREFRAMES ---
    case 'C':
      {
        if (paramMulti == 0) {
          break;
        }
        static float angle = 0;
        float speed = paramSpeed * 0.005;
        angle += speed;
        int actualSize = 20 + (paramSize * 10);

        // 1. LOOKUP ARRAYS FOR BASE GRID
        // Index matches paramMulti (0 to 9).
        // M=1(1x1), M=2(2x1), M=3(2x1), M=4(2x2), M=5(2x2), M=6(2x2), M=7(3x2), M=8(3x2), M=9(3x3)
        int gridCols[] = { 0, 1, 2, 2, 2, 2, 3, 3, 3, 3 };
        int gridRows[] = { 0, 1, 1, 1, 2, 2, 2, 2, 2, 3 };

        int cols = gridCols[paramMulti];
        int rows = gridRows[paramMulti];

        // Calculate how many go in the grid, and how many overlap in the center
        int baseItems = cols * rows;
        int remainder = paramMulti - baseItems;

        int cellWidth = XRES / cols;
        int cellHeight = YRES / rows;

        // 2. DRAW THE BASE GRID (The normal layout)
        for (int i = 0; i < baseItems; i++) {
          int col = i % cols;
          int row = i / cols;

          int cx = (col * cellWidth) + (cellWidth / 2);
          int cy = (row * cellHeight) + (cellHeight / 2);

          switch (paramShape) {
            case 0: graphics.drawWireframeCube(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
            case 1: graphics.drawWireframePyramid(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
            case 2: graphics.drawWireFrameSphere(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
          }
        }

        // 3. DRAW THE OVERLAPPING REMAINDER (In the center)
        for (int i = 0; i < remainder; i++) {
          int cx = XRES / 2;
          int cy = YRES / 2;

          // If there are exactly 2 remainder items, spread them out!
          if (remainder == 2) {
            // Calculate the offset: Half the shape's size PLUS 20 extra pixels of padding
            int spreadDistance = (actualSize / 2) + 30;

            // Apply the negative offset to the first one, and positive to the second
            cx += (i == 0) ? -spreadDistance : spreadDistance;
          }

          switch (paramShape) {
            case 0: graphics.drawWireframeCube(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
            case 1: graphics.drawWireframePyramid(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
          }
        }
        break;
      }
    // --- SCENE D: TUNNEL LINES ---
    case 'D':
      {
        // 1. Map the Pure Data parameters
        int numPairs = 2 + paramSize;                // Density: 2 to 11 pairs of lines
        float speed = (paramSpeed + 1) * 0.005;      // Animation speed
        float baseAngle = (paramShape / 10.0) * PI;  // Angle: 0 to 180 degrees
        int lineThickness = paramMulti;              // Width of the lines: 1 to 9 pixels

        // 2. Advance a continuous time counter
        static float tunnelTime = 0;
        tunnelTime += speed;

        // Calculate the normal vector (perpendicular) and direction vector (the line itself)
        float nx = cos(baseAngle);
        float ny = sin(baseAngle);
        float dx = -sin(baseAngle);
        float dy = cos(baseAngle);

        int L = 400;  // Large length to ensure the lines always reach off-screen
        int cx = XRES / 2;
        int cy = YRES / 2;

        // 3. Draw the tunnel
        for (int i = 0; i < numPairs; i++) {
          // Phase offsets each pair so they are evenly spaced in the tunnel
          float phase = (float)i / numPairs;

          // current_t loops continuously from 0.0 to 1.0 over time
          float current_t = fmod(tunnelTime + phase, 1.0f);

          // Exponential distance creates 3D perspective acceleration
          float d = 1.0f * pow(350.0f, current_t);

          // Calculate the base center points for this specific pair of lines
          float px1 = cx + (d * nx);
          float py1 = cy + (d * ny);
          float px2 = cx - (d * nx);
          float py2 = cy - (d * ny);

          // --- THE THICKNESS LOOP ---
          // Draw the line multiple times, nudged slightly along the normal vector (nx, ny)
          int halfThick = lineThickness / 2;
          int evenOffset = (lineThickness % 2 == 0) ? 1 : 0;  // Adjusts centering for even numbers

          for (int w = -halfThick; w <= halfThick - evenOffset; w++) {

            // Calculate the tiny 1-pixel nudges
            int offsetX = (int)(w * nx);
            int offsetY = (int)(w * ny);

            // Draw Thick Line 1 (Positive side of the center)
            graphics.line((int)(px1 - L * dx) + offsetX, (int)(py1 - L * dy) + offsetY,
                          (int)(px1 + L * dx) + offsetX, (int)(py1 + L * dy) + offsetY, fgColor);

            // Draw Thick Line 2 (Negative side of the center)
            graphics.line((int)(px2 - L * dx) + offsetX, (int)(py2 - L * dy) + offsetY,
                          (int)(px2 + L * dx) + offsetX, (int)(py2 + L * dy) + offsetY, fgColor);
          }
        }
        break;
      }
    // --- SCENE E: WARP SPEED STARFIELD ---
    // --- SCENE E: WARP SPEED STARFIELD ---
    case 'E':
      {
        const int MAX_STARS = 100;
        static float starsX[MAX_STARS];
        static float starsY[MAX_STARS];
        static float starsZ[MAX_STARS];
        static bool starsInit = false;

        if (!starsInit) {
          for (int i = 0; i < MAX_STARS; i++) {
            starsX[i] = random(-500, 500);
            starsY[i] = random(-500, 500);
            starsZ[i] = random(10, 200);
          }
          starsInit = true;
        }

        int numStars = map(paramMulti, 1, 9, 10, MAX_STARS);
        float speed = (paramSpeed + 1) * 0.5;
        float maxSize = paramSize + 1.0;

        static float tunnelRot = 0;
        tunnelRot += (paramShape - 4.5) * 0.01;
        float cosR = cos(tunnelRot);
        float sinR = sin(tunnelRot);

        int cx = XRES / 2;
        int cy = YRES / 2;

        for (int i = 0; i < numStars; i++) {
          starsZ[i] -= speed;

          if (starsZ[i] <= 1.0) {
            starsX[i] = random(-500, 500);
            starsY[i] = random(-500, 500);
            starsZ[i] = 200.0;
          }

          float rx = starsX[i] * cosR - starsY[i] * sinR;
          float ry = starsX[i] * sinR + starsY[i] * cosR;

          float projX = (rx / starsZ[i]) * 100.0 + cx;
          float projY = (ry / starsZ[i]) * 100.0 + cy;

          if (projX < 0 || projX >= XRES || projY < 0 || projY >= YRES) {
            starsX[i] = random(-500, 500);
            starsY[i] = random(-500, 500);
            starsZ[i] = 200.0;
            continue;
          }

          float currentSize = maxSize * (1.0 - (starsZ[i] / 200.0));
          if (currentSize < 1.0) currentSize = 1.0;
          int s = (int)currentSize;

          int drawX = (int)projX - (s / 2);
          int drawY = (int)projY - (s / 2);

          // Look how much cleaner this is using the native library!
          // fgColor should be whatever your standard drawing color variable is
          graphics.fillRect(drawX, drawY, s, s, fgColor);
        }
        break;
      }

    case 'F':
      {
        // THE NEW WEB TEXT SCENE
        // We use the same paramSize logic from your MIDI fader to scale it!
        graphics.setCursor(10, 10);
        if (paramSize > 4) {
          graphics.printBig((char *)displayText.c_str(), paramSize);
        } else {
          graphics.print((char *)displayText.c_str());
        }
        break;
      }


    default:
      graphics.setCursor(10, 10);
      graphics.setTextColor(fgColor, bgColor);
      graphics.print((char *)"UNKNOWN COMMAND");
      break;
  }

  graphics.end();
}
void loop() {
  // Check for serial input every loop
  // handleSerial_debug();
  // server.handleClient();
  handleSerial();


  // button press control code
  // bool currentButtonState = digitalRead(BOOT_BUTTON);

  // if (currentButtonState == LOW && lastButtonState == HIGH && (millis() - lastPressTime > 250)) {
  //   currentScene++;  // Move to the next letter (A becomes B, B becomes C...)
  //   if (currentScene > 'F') {
  //     currentScene = 'A';
  //   }
  //   lastPressTime = millis();  // Reset the debounce timer
  // }
  // lastButtonState = currentButtonState;  // Remember the state for the next frame

  // Draw the graphics
  draw();
}