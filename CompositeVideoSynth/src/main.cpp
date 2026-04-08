#include <Arduino.h>

//code by bitluni (send me a high five if you like the code)
#include <esp_pm.h>             // System file -> Angle brackets
#include <CompositeGraphics.h>  // In the 'lib' folder -> Angle brackets
#include <CompositeOutput.h>    // In the 'lib' folder -> Angle brackets
#include "font6x8.h"            // Local file in 'src' -> Quotes

#define RXD2 16 // Connect the C3's TX pin to this pin!
#define TXD2 17 // (We won't actually send anything back to the C3)

//PAL MAX, half: 324x268 full: 648x536
//NTSC MAX, half: 324x224 full: 648x448
const int XRES = 320;
const int YRES = 225;
String displayText = "REBOOT_\nTHE_RISE\n_OF_THE_\nROBOTS";

//Graphics using the defined resolution for the backbuffer
CompositeGraphics graphics(XRES, YRES);
CompositeOutput composite(CompositeOutput::NTSC, XRES * 2, YRES * 2);
Font<CompositeGraphics> font(6, 8, font6x8::pixels);

#include <soc/rtc.h>
char currentScene = 'F';

// --- NOW ACCEPTING 0 to 127 ---
int paramBg = 0;     
int paramSize = 79;   
int paramSpeed = 0;  
int paramShape = 111;  
int paramMulti = 0;  

int t = millis();
int j = 0;
int scene = 0;

void compositeCore(void *data);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Composite Video Started!");
  Serial2.begin(460800, SERIAL_8N1, RXD2, TXD2);
  
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
      currentScene = input.charAt(0);
      int s1Start = len - 15;

      if (s1Start > 1) {
        displayText = input.substring(1, s1Start);
      }

      paramBg    = input.substring(s1Start, s1Start + 3).toInt();
      paramSize  = input.substring(s1Start + 3, s1Start + 6).toInt();
      paramSpeed = input.substring(s1Start + 6, s1Start + 9).toInt();
      paramShape = input.substring(s1Start + 9, s1Start + 12).toInt();
      paramMulti = input.substring(s1Start + 12, s1Start + 15).toInt();

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

// --- SCENE E: CASCADING TEXT ---
    case 'E':
      {
        // 1. DYNAMIC SIZE (Slider 2: paramSize)
        int scaleFactor = map(paramSize, 0, 127, 1, 9);
        if (scaleFactor < 1) scaleFactor = 1;
        int rowSpacing = 9 * scaleFactor; // Pixel height per line
        
        // 2. CALCULATE TOTAL BLOCK HEIGHT
        int textLines = 1;
        for (int i = 0; i < displayText.length(); i++) {
          if (displayText.charAt(i) == '|' || displayText.charAt(i) == '\n') textLines++;
        }
        int textBlockHeight = textLines * rowSpacing;

        // 3. CASCADE SETUP (Slider 3: paramSpeed, Slider 5: paramMulti)
        int numLines = map(paramMulti, 0, 127, 1, 15); // Number of trailing ghosts              
        float cascadeSpeed = 0.05 + ((paramSpeed / 127.0) * 1.45); 
        
        // Space out the ghosts based on the height of the entire paragraph
        int cascadeGap = textBlockHeight; 
        if (cascadeGap < rowSpacing) cascadeGap = rowSpacing; // Safety fallback

        int totalRows = YRES / cascadeGap;
        if (totalRows < 1) totalRows = 1; // Safety for massive text

        static float virtualRow = 0;
        virtualRow += cascadeSpeed;
        int headRow = (int)virtualRow % totalRows;

        // 4. PULSING FREQUENCY LOGIC (Slider 4: paramShape)
        static int flashTimer = 0;
        bool doFlash = false;
        
        if (paramShape >= 10) {
          // Map slider to a cycle length (e.g., from 180 frames down to 10 frames)
          int cycleFrames = map(paramShape, 10, 127, 180, 10); 
          flashTimer++;
          if (flashTimer >= cycleFrames) {
            doFlash = true;
            if (flashTimer >= cycleFrames + 7) { // Hold the flash for exactly 2 frames
              flashTimer = 0;
            }
          }
        } else {
          flashTimer = 0; // Reset timer if slider is pulled below 10
        }

        // Determine the active background color for this specific frame
        int actualBg = doFlash ? fgColor : bgColor;
        
        // Override the global background for the flash
        if (doFlash) {
          graphics.fillRect(0, 0, XRES, YRES, actualBg);
        }

        // 5. DRAW THE TRAILS
        for (int i = 0; i < numLines; i++) {
          int drawRow = headRow - i;
          if (drawRow < 0) drawRow += totalRows;

          int yPosBase = drawRow * cascadeGap;
          
          // 6. CALCULATE TEXT FADE (Respecting the current background contrast)
          int colorIntensity;
          if (actualBg == 0) { 
            // Black background: Text starts White (54) and fades to Black (0)
            colorIntensity = 54 - (i * (54 / numLines));
          } else { 
            // White background: Text starts Black (0) and fades to White (54)
            colorIntensity = i * (54 / numLines);
          }
          
          // Clamp values to prevent graphical glitches
          if (colorIntensity > 54) colorIntensity = 54;
          if (colorIntensity < 0) colorIntensity = 0;

          graphics.setTextColor(colorIntensity);

          // 7. RENDER THE MULTI-LINE BLOCK AT THIS Y-POSITION
          int cursorY = yPosBase;
          String tempLine = "";

          for (int c_idx = 0; c_idx < displayText.length(); c_idx++) {
            char c = displayText.charAt(c_idx);
            
            if (c == '|' || c == '\n') { 
              graphics.setCursor(10, cursorY);
              if (scaleFactor > 1) {
                graphics.printBig((char *)tempLine.c_str(), scaleFactor);
              } else {
                graphics.print((char *)tempLine.c_str());
              }
              cursorY += rowSpacing; 
              tempLine = ""; 
            } else {
              tempLine += c;
            }
          }
          
          // Print the final chunk
          if (tempLine.length() > 0) {
            graphics.setCursor(10, cursorY);
            if (scaleFactor > 1) {
              graphics.printBig((char *)tempLine.c_str(), scaleFactor);
            } else {
              graphics.print((char *)tempLine.c_str());
            }
          }
        }
        break;
      }
    // --- SCENE C: 3D WIREFRAMES ---
    case 'C':
      {
        int mIdx = map(paramMulti, 0, 127, 0, 9);
        if (mIdx == 0) break;

        static float angle = 0;
        float speed = (paramSpeed / 127.0) * 0.1; // Smooth float rotation
        angle += speed;
        int actualSize = 20 + paramSize; // Smooth size scaling

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
        int numPairs = 2 + map(paramSize, 0, 127, 0, 12);                
        float speed = 0.001 + ((paramSpeed / 127.0) * 0.049);      
        float baseAngle = (paramShape / 127.0) * PI;  
        int lineThickness = map(paramMulti, 0, 127, 1, 15);              

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

    // --- SCENE B: WARP SPEED STARFIELD ---
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
        float speed = 0.1 + ((paramSpeed / 127.0) * 14.9);
        float maxSize = 1.0 + ((paramSize / 127.0) * 20.0);

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

    // --- SCENE F: WEB TEXT SCENE (UPGRADED) ---
    case 'F':
      {
        int cursorX = 10;
        
        // 1. DYNAMIC SIZE: Always scales from 1x to 9x smoothly across the 0-127 range
        int scaleFactor = map(paramSize, 0, 127, 1, 9);
        if (scaleFactor < 1) scaleFactor = 1; 
        int rowSpacing = 9 * scaleFactor; // Calculate physical pixel height per line
        
        // 2. DYNAMIC SHADE: Map paramShape to a color intensity (0-54)
        int textShade = map(paramShape, 0, 127, 0, 54);
        if (bgColor == 54) textShade = 54 - textShade; // Invert to fade to black if background is white
        graphics.setTextColor(textShade);

        // 3. CALCULATE TEXT BLOCK HEIGHT (to figure out when it scrolls off screen)
        int numLines = 1;
        for (int i = 0; i < displayText.length(); i++) {
          if (displayText.charAt(i) == '|' || displayText.charAt(i) == '\n') numLines++;
        }
        int totalTextHeight = numLines * rowSpacing;

        // 4. SMOOTH SCROLLING & MARQUEE LOOP
        static float textScrollY = 10;
        
        if (paramSpeed > 10) {
          // Calculate buttery float speed (up to 3 pixels per frame)
          float scrollSpeed = ((paramSpeed - 10) / 117.0) * 3.0; 
          textScrollY -= scrollSpeed; // Move UP
          
          // If the absolute bottom of the text block passes the top of the screen (0)
          if (textScrollY + totalTextHeight < 0) {
            textScrollY = YRES; // Instantly wrap around to the bottom of the CRT
          }
        } else {
          // If speed is 10 or less, snap the text back to its starting anchor
          textScrollY = 10;
        }

        // 5. RENDER THE TEXT
        float cursorY = textScrollY;
        String tempLine = "";
        
        for (int i = 0; i < displayText.length(); i++) {
          char c = displayText.charAt(i);
          
          // Handle both pipe characters AND standard newlines just to be safe
          if (c == '|' || c == '\n') { 
            graphics.setCursor(cursorX, (int)cursorY);
            if (scaleFactor > 1) {
              graphics.printBig((char *)tempLine.c_str(), scaleFactor);
            } else {
              graphics.print((char *)tempLine.c_str());
            }
            cursorY += rowSpacing; 
            tempLine = ""; 
          } else {
            tempLine += c;
          }
        }
        
        // Print the final remaining chunk of text
        if (tempLine.length() > 0) {
          graphics.setCursor(cursorX, (int)cursorY);
          if (scaleFactor > 1) {
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