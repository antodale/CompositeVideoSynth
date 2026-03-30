//code by bitluni (send me a high five if you like the code)
#include "esp_pm.h"
#include "CompositeGraphics.h"
#include "Image.h"
#include "CompositeOutput.h"

#include "luni.h"
#include "font6x8.h"

#define RXD2 16 // Connect the C3's TX pin to this pin!
#define TXD2 17 // (We won't actually send anything back to the C3)



//PAL MAX, half: 324x268 full: 648x536
//NTSC MAX, half: 324x224 full: 648x448
const int XRES = 320;
const int YRES = 225;
String displayText = "INIT";

//Graphics using the defined resolution for the backbuffer
CompositeGraphics graphics(XRES, YRES);
CompositeOutput composite(CompositeOutput::NTSC, XRES * 2, YRES * 2);

Image<CompositeGraphics> luni0(luni::xres, luni::yres, luni::pixels);
Font<CompositeGraphics> font(6, 8, font6x8::pixels);

#include <soc/rtc.h>
char currentScene = 'C';

// --- NOW ACCEPTING 0 to 127 ---
int paramBg = 0;     
int paramSize = 64;   
int paramSpeed = 64;  
int paramShape = 64;  
int paramMulti = 64;  

int t = millis();
int j = 0;
int scene = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Composite Video Started!");
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  

  esp_pm_lock_handle_t powerManagementLock;
  esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "compositeCorePerformanceLock", &powerManagementLock);
  esp_pm_lock_acquire(powerManagementLock);

  composite.init();
  graphics.init();
  graphics.setFont(font);

  xTaskCreatePinnedToCore(compositeCore, "compositeCoreTask", 1024, NULL, 1, NULL, 0);
}

void compositeCore(void *data) {
  while (true) {
    composite.sendFrameHalfResolution(&graphics.frame);
  }
}

// --- THE NEW FIXED-LENGTH PARSER ---
void handleSerial() {
  if (Serial2.available() > 0) {
    String input = Serial2.readStringUntil('\n');
    input.trim();
    int len = input.length();

    // The shortest possible valid packet (Mode + 5 padded sliders) is 16 chars.
    if (len >= 16) {
      // 1. Grab the Mode
      currentScene = input.charAt(0);

      // 2. The sliders are ALWAYS the last 15 characters. Find where they start.
      int s1Start = len - 15;

      // 3. If there is a gap between Mode (index 1) and Sliders, that gap is the text!
      if (s1Start > 1) {
        displayText = input.substring(1, s1Start);
      }

      // 4. Extract the exact 3-digit blocks
      paramBg    = input.substring(s1Start, s1Start + 3).toInt();
      paramSize  = input.substring(s1Start + 3, s1Start + 6).toInt();
      paramSpeed = input.substring(s1Start + 6, s1Start + 9).toInt();
      paramShape = input.substring(s1Start + 9, s1Start + 12).toInt();
      paramMulti = input.substring(s1Start + 12, s1Start + 15).toInt();

      // Debugging print
      Serial.print("message recieved: "); Serial.println(input);
      Serial.print("TEXT: "); Serial.println(displayText);
      Serial.printf("SLIDERS: Bg:%d, Size:%d, Spd:%d, Shp:%d, Mlt:%d\n", 
                     paramBg, paramSize, paramSpeed, paramShape, paramMulti);
    }
  }
}

void draw() {
  // Translate 0-127 into a clean binary switch for Black vs White BG
  int bgColor = (paramBg > 63) ? 54 : 0;
  int fgColor = (paramBg > 63) ? 0 : 54;

  graphics.begin(bgColor);

  switch (currentScene) {
    case 'A':
      if (paramMulti < 64) {
        graphics.fillRect(0, 0, XRES, YRES, 54);
      } else {
        graphics.fillRect(0, 0, XRES, YRES, 20);
      }
      break;

    // --- SCENE B: CASCADING TEXT ---
    case 'E':
      {
        // Map high-res data back to safe limits
        int numLines = map(paramMulti, 0, 127, 1, 10);                 
        float cascadeSpeed = map(paramSpeed, 0, 127, 1, 10) * 0.05;  
        int rowHeight = (paramSize > 64) ? 16 : 8;
        int totalRows = YRES / rowHeight;

        static float virtualRow = 0;
        virtualRow += cascadeSpeed;
        int headRow = (int)virtualRow % totalRows;

        for (int i = 0; i < numLines; i++) {
          int drawRow = headRow - i;
          if (drawRow < 0) drawRow += totalRows;

          int yPos = drawRow * rowHeight;
          int colorIntensity = (bgColor == 54 ? 1 : 0 * 5) + (54 - (i * (54 / numLines)));
          if (colorIntensity > 54) colorIntensity = 54;
          if (colorIntensity < 0) colorIntensity = 0;

          graphics.setTextColor(colorIntensity);
          graphics.setCursor(10, yPos);

          if (paramSize > 64) {
            int scaleFactor = map(paramSize, 64, 127, 5, 9);
            graphics.printBig((char *)displayText.c_str(), scaleFactor);
          } else {
            graphics.print((char *)displayText.c_str());
          }
        }
        break;
      }

    // --- SCENE C: 3D WIREFRAMES ---
    case 'C':
      {
        // Map paramMulti perfectly to the 0-9 array index limits to prevent crashes
        int mIdx = map(paramMulti, 0, 127, 0, 9);
        if (mIdx == 0) break;

        static float angle = 0;
        float speed = map(paramSpeed, 0, 127, 0, 9) * 0.005;
        angle += speed;
        int actualSize = 20 + map(paramSize, 0, 127, 0, 90);

        int gridCols[] = { 0, 1, 2, 2, 2, 2, 3, 3, 3, 3 };
        int gridRows[] = { 0, 1, 1, 1, 2, 2, 2, 2, 2, 3 };

        int cols = gridCols[mIdx];
        int rows = gridRows[mIdx];
        int baseItems = cols * rows;
        int remainder = mIdx - baseItems;

        int cellWidth = XRES / cols;
        int cellHeight = YRES / rows;

        int safeShape = map(paramShape, 0, 127, 0, 2);

        for (int i = 0; i < baseItems; i++) {
          int col = i % cols;
          int row = i / cols;
          int cx = (col * cellWidth) + (cellWidth / 2);
          int cy = (row * cellHeight) + (cellHeight / 2);

          switch (safeShape) {
            case 0: graphics.drawWireframeCube(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
            case 1: graphics.drawWireframePyramid(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
            case 2: graphics.drawWireFrameSphere(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
          }
        }

        for (int i = 0; i < remainder; i++) {
          int cx = XRES / 2;
          int cy = YRES / 2;
          if (remainder == 2) {
            int spreadDistance = (actualSize / 2) + 30;
            cx += (i == 0) ? -spreadDistance : spreadDistance;
          }
          switch (safeShape) {
            case 0: graphics.drawWireframeCube(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
            case 1: graphics.drawWireframePyramid(angle, angle * 0.8, angle * 1.2, cx, cy, actualSize, fgColor); break;
          }
        }
        break;
      }

    // --- SCENE D: TUNNEL LINES ---
    case 'D':
      {
        int numPairs = 2 + map(paramSize, 0, 127, 0, 9);                
        float speed = map(paramSpeed, 0, 127, 1, 10) * 0.005;      
        
        // Use the raw 127 data for massive, buttery smooth angle rotation!
        float baseAngle = (paramShape / 127.0) * PI;  
        
        int lineThickness = map(paramMulti, 0, 127, 1, 9);              

        static float tunnelTime = 0;
        tunnelTime += speed;

        float nx = cos(baseAngle);
        float ny = sin(baseAngle);
        float dx = -sin(baseAngle);
        float dy = cos(baseAngle);

        int L = 400;  
        int cx = XRES / 2;
        int cy = YRES / 2;

        for (int i = 0; i < numPairs; i++) {
          float phase = (float)i / numPairs;
          float current_t = fmod(tunnelTime + phase, 1.0f);
          float d = 1.0f * pow(350.0f, current_t);

          float px1 = cx + (d * nx); float py1 = cy + (d * ny);
          float px2 = cx - (d * nx); float py2 = cy - (d * ny);

          int halfThick = lineThickness / 2;
          int evenOffset = (lineThickness % 2 == 0) ? 1 : 0;  

          for (int w = -halfThick; w <= halfThick - evenOffset; w++) {
            int offsetX = (int)(w * nx);
            int offsetY = (int)(w * ny);

            graphics.line((int)(px1 - L * dx) + offsetX, (int)(py1 - L * dy) + offsetY,
                          (int)(px1 + L * dx) + offsetX, (int)(py1 + L * dy) + offsetY, fgColor);
            graphics.line((int)(px2 - L * dx) + offsetX, (int)(py2 - L * dy) + offsetY,
                          (int)(px2 + L * dx) + offsetX, (int)(py2 + L * dy) + offsetY, fgColor);
          }
        }
        break;
      }

    // --- SCENE E: WARP SPEED STARFIELD ---
    case 'B':
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

        int numStars = map(paramMulti, 0, 127, 10, MAX_STARS);
        float speed = map(paramSpeed, 0, 127, 1, 10) * 0.5;
        float maxSize = map(paramSize, 0, 127, 1, 10);

        static float tunnelRot = 0;
        tunnelRot += (paramShape - 63.5) * 0.001; 
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

          graphics.fillRect(drawX, drawY, s, s, fgColor);
        }
        break;
      }

    // --- SCENE F: WEB TEXT SCENE ---
    // --- SCENE F: WEB TEXT SCENE (NOW MULTI-LINE CAPABLE) ---
    case 'F':
      {
        int cursorX = 10;
        int cursorY = 10;
        
        // Calculate the physical height of the text based on the slider
        int scaleFactor = (paramSize > 64) ? map(paramSize, 64, 127, 5, 9) : 1;
        int rowSpacing = 9 * scaleFactor; // Base font is 8px high + 1px padding

        graphics.setCursor(cursorX, cursorY);
        
        String tempLine = "";
        
        // Scan the text character by character
        for (int i = 0; i < displayText.length(); i++) {
          char c = displayText.charAt(i);
          
          if (c == '|') { 
            // We hit a line break! Print what we have so far...
            if (paramSize > 64) {
              graphics.printBig((char *)tempLine.c_str(), scaleFactor);
            } else {
              graphics.print((char *)tempLine.c_str());
            }
            
            // ...then physically move the cursor down...
            cursorY += rowSpacing; 
            graphics.setCursor(cursorX, cursorY);
            
            // ...and clear the temp line to start fresh on the new row!
            tempLine = ""; 
          } else {
            // Keep building the line
            tempLine += c;
          }
        }
        
        // Print the final remaining chunk of text
        if (tempLine.length() > 0) {
          if (paramSize > 64) {
            graphics.printBig((char *)tempLine.c_str(), scaleFactor);
          } else {
            graphics.print((char *)tempLine.c_str());
          }
        }
        break;
      }

    default:
      graphics.setCursor(10, 10);
      graphics.setTextColor(fgColor, bgColor);
      graphics.print((char *)"AWAITING UPLINK");
      break;
  }

  graphics.end();
}

void loop() {
  handleSerial();
  draw();
}