#pragma once
#include "Font.h"
#include "TriangleTree.h"

class CompositeGraphics
{ 
  public:
  int xres;
  int yres;
  char **frame;
  char **backbuffer;
  char **zbuffer;
  int cursorX, cursorY, cursorBaseX;
  int frontColor, backColor;
  Font<CompositeGraphics> *font;
  
  TriangleTree<CompositeGraphics> *triangleBuffer;
  TriangleTree<CompositeGraphics> *triangleRoot;
  int trinagleBufferSize;
  int triangleCount;

  CompositeGraphics(int w, int h, int initialTrinagleBufferSize = 0)
    :xres(w), 
    yres(h)
  {
    font = 0;
    cursorX = cursorY = cursorBaseX = 0;
    trinagleBufferSize = initialTrinagleBufferSize;
    triangleCount = 0;
    frontColor = 50;
    backColor = -1;
  }

  void setTextColor(int front, int back = -1)
  {
    //-1 = transparent back;
    frontColor = front;
    backColor = back;
  }
  
  void init()
  {
    frame = (char**)malloc(yres * sizeof(char*));
    backbuffer = (char**)malloc(yres * sizeof(char*));
    //not enough memory for z-buffer implementation
    //zbuffer = (char**)malloc(yres * sizeof(char*));
    for(int y = 0; y < yres; y++)
    {
      frame[y] = (char*)malloc(xres);
      backbuffer[y] = (char*)malloc(xres);
      //zbuffer[y] = (char*)malloc(xres);
    }
    triangleBuffer = (TriangleTree<CompositeGraphics>*)malloc(sizeof(TriangleTree<CompositeGraphics>) * trinagleBufferSize);
  }

  void setFont(Font<CompositeGraphics> &font)
  {
    this->font = &font;
  }
  
  void setCursor(int x, int y)
  {
    cursorX = cursorBaseX = x;  
    cursorY = y;  
  }
  
  void print(char *str)
  {
    if(!font) return;
    while(*str)
    {
      if(*str >= 32 && *str < 128)
        font->drawChar(*this, cursorX, cursorY, *str, frontColor, backColor);
      cursorX += font->xres;
      if(cursorX + font->xres > xres || *str == '\n')
      {
        cursorX = cursorBaseX;
        cursorY += font->yres;        
      }
      str++;
    }
  }

void printBig(char *str, int dimension)
{
  if(!font) return;
  while(*str)
  {
    if(*str >= 32 && *str < 128)
    {
      // We pass 'dimension' down to the font drawing function
      font->drawCharBig(*this, cursorX, cursorY, *str, frontColor, backColor, dimension);
    }
    
    // Multiply the cursor jump by the new dimension
    cursorX += (font->xres * dimension);
    
    // Check if the NEXT character will run off the screen, scaled by dimension
    if(cursorX + (font->xres * dimension) > xres || *str == '\n')
    {
      cursorX = cursorBaseX;
      // Multiply the line break drop by the new dimension
      cursorY += (font->yres * dimension);        
    }
    str++;
  }
}

  void print(int number, int base = 10, int minCharacters = 1)
  {
    bool sign = number < 0;
    if(sign) number = -number;
    const char baseChars[] = "0123456789ABCDEF";
    char temp[33];
    temp[32] = 0;
    int i = 31;
    do
    {
      temp[i--] = baseChars[number % base];
      number /= base;
    }while(number > 0);
    if(sign)
      temp[i--] = '-';
    for(;i > 31 - minCharacters; i--)
      temp[i] = ' ';
    print(&temp[i + 1]);
  }
  
  inline void begin(int clear = -1, bool clearZ = true)
  {
    if(clear > -1)
      for(int y = 0; y < yres; y++)
        for(int x = 0; x < xres; x++)
          backbuffer[y][x] = clear;
    triangleCount = 0;
    triangleRoot = 0;
  }

  inline void dotFast(int x, int y, char color)
  {
    backbuffer[y][x] = color;
  }
  
  inline void dot(int x, int y, char color)
  {
    if((unsigned int)x < xres && (unsigned int)y < yres)
      backbuffer[y][x] = color;
  }
  
  inline void dotAdd(int x, int y, char color)
  {
    if((unsigned int)x < xres && (unsigned int)y < yres)
      backbuffer[y][x] = min(54, color + backbuffer[y][x]);
  }
  
  inline char get(int x, int y)
  {
    if((unsigned int)x < xres && (unsigned int)y < yres)
      return backbuffer[y][x];
    return 0;
  }
    
  inline void xLine(int x0, int x1, int y, char color)
  {
    if(x0 > x1)
    {
      int xb = x0;
      x0 = x1;
      x1 = xb;
    }
    if(x0 < 0) x0 = 0;
    if(x1 > xres) x1 = xres;
    for(int x = x0; x < x1; x++)
      dotFast(x, y, color);
  }

  void enqueueTriangle(short *v0, short *v1, short *v2, char color)
  {
    if(triangleCount >= trinagleBufferSize) return;
    TriangleTree<CompositeGraphics> &t = triangleBuffer[triangleCount++];
    t.set(v0, v1, v2, color);
    if(triangleRoot)
      triangleRoot->add(&triangleRoot, t);
    else
      triangleRoot = &t;
  }

  void triangle(short *v0, short *v1, short *v2, char color)
  {
    short *v[3] = {v0, v1, v2};
    if(v[1][1] < v[0][1])
    {
      short *vb = v[0]; v[0] = v[1]; v[1] = vb;
    }
    if(v[2][1] < v[1][1])
    {
      short *vb = v[1]; v[1] = v[2]; v[2] = vb;
    }
    if(v[1][1] < v[0][1])
    {
      short *vb = v[0]; v[0] = v[1]; v[1] = vb;
    }
    int y = v[0][1];
    int xac = v[0][0] << 16;
    int xab = v[0][0] << 16;
    int xbc = v[1][0] << 16;
    int xaci = 0;
    int xabi = 0;
    int xbci = 0;
    if(v[1][1] != v[0][1])
      xabi = ((v[1][0] - v[0][0]) << 16) / (v[1][1] - v[0][1]);
    if(v[2][1] != v[0][1])
      xaci = ((v[2][0] - v[0][0]) << 16) / (v[2][1] - v[0][1]);
    if(v[2][1] != v[1][1])
      xbci = ((v[2][0] - v[1][0]) << 16) / (v[2][1] - v[1][1]);

    for(; y < v[1][1] && y < yres; y++)
    {
      if(y >= 0)
        xLine(xab >> 16, xac >> 16, y, color);
      xab += xabi;
      xac += xaci;
    }
    for(; y < v[2][1] && y < yres; y++)
    {
      if(y >= 0)
        xLine(xbc >> 16, xac >> 16, y, color);
      xbc += xbci;
      xac += xaci;
    }
  }

void drawWireframePyramid(float angleX, float angleY, float angleZ, int cx, int cy, int size, char color) {
    // 4 Vertices: 3 for the equilateral triangular base, 1 for the apex
    // We use 0.866 (which is approx sqrt(3)/2) and 0.5 to plot points 120 degrees apart
    float pyr[4][3] = {
      {  0.0f,  -1.0f,   1.0f }, // Base front
      {  0.866f,-1.0f,  -0.5f }, // Base back right
      { -0.866f,-1.0f,  -0.5f }, // Base back left
      {  0.0f,   1.0f,   0.0f }  // Apex
    };

    int proj[4][2]; 

    // Loop only 4 times now
    for (int i = 0; i < 4; i++) {
      float x = pyr[i][0], y = pyr[i][1], z = pyr[i][2];
      float y_rotX = y * cos(angleX) - z * sin(angleX);
      float z_rotX = y * sin(angleX) + z * cos(angleX);
      float x_rotY = x * cos(angleY) + z_rotX * sin(angleY);
      float z_rotY = -x * sin(angleY) + z_rotX * cos(angleY);
      float x_rotZ = x_rotY * cos(angleZ) - y_rotX * sin(angleZ);
      float y_rotZ = x_rotY * sin(angleZ) + y_rotX * cos(angleZ);
      float z_perspective = z_rotY + 2.5;

      proj[i][0] = cx + (int)((x_rotZ / z_perspective) * size);
      proj[i][1] = cy + (int)((y_rotZ / z_perspective) * size);
    }

    // Draw the 3 lines of the triangular base
    line(proj[0][0], proj[0][1], proj[1][0], proj[1][1], color);
    line(proj[1][0], proj[1][1], proj[2][0], proj[2][1], color);
    line(proj[2][0], proj[2][1], proj[0][0], proj[0][1], color);

    // Draw the 3 lines connecting the base to the apex
    line(proj[0][0], proj[0][1], proj[3][0], proj[3][1], color);
    line(proj[1][0], proj[1][1], proj[3][0], proj[3][1], color);
    line(proj[2][0], proj[2][1], proj[3][0], proj[3][1], color);
  }
  
void drawWireframeCube(float angleX, float angleY, float angleZ, int cx, int cy, int size, char color) {
    // 1. Define the 8 vertices of a cube in 3D space (from -1 to 1)
    float cube[8][3] = {
      {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1}, // Front face
      {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}  // Back face
    };

    int proj[8][2]; // Array to store our final 2D screen coordinates

    // 2. Rotate and Project each vertex
    for (int i = 0; i < 8; i++) {
      float x = cube[i][0];
      float y = cube[i][1];
      float z = cube[i][2];

      // Rotate around X-axis
      float y_rotX = y * cos(angleX) - z * sin(angleX);
      float z_rotX = y * sin(angleX) + z * cos(angleX);
      
      // Rotate around Y-axis
      float x_rotY = x * cos(angleY) + z_rotX * sin(angleY);
      float z_rotY = -x * sin(angleY) + z_rotX * cos(angleY);
      
      // Rotate around Z-axis
      float x_rotZ = x_rotY * cos(angleZ) - y_rotX * sin(angleZ);
      float y_rotZ = x_rotY * sin(angleZ) + y_rotX * cos(angleZ);

      // Perspective Projection (push it away from the camera by adding 2.5 to Z)
      float distance = 2.5; 
      float z_perspective = z_rotY + distance;

      // Calculate final 2D coordinates and center them on screen
      proj[i][0] = cx + (int)((x_rotZ / z_perspective) * size);
      proj[i][1] = cy + (int)((y_rotZ / z_perspective) * size);
    }

    // 3. Draw the 12 connecting lines using the internal line() function
    // Front Face
    line(proj[0][0], proj[0][1], proj[1][0], proj[1][1], color);
    line(proj[1][0], proj[1][1], proj[2][0], proj[2][1], color);
    line(proj[2][0], proj[2][1], proj[3][0], proj[3][1], color);
    line(proj[3][0], proj[3][1], proj[0][0], proj[0][1], color);

    // Back Face
    line(proj[4][0], proj[4][1], proj[5][0], proj[5][1], color);
    line(proj[5][0], proj[5][1], proj[6][0], proj[6][1], color);
    line(proj[6][0], proj[6][1], proj[7][0], proj[7][1], color);
    line(proj[7][0], proj[7][1], proj[4][0], proj[4][1], color);

    // Connecting Edges
    line(proj[0][0], proj[0][1], proj[4][0], proj[4][1], color);
    line(proj[1][0], proj[1][1], proj[5][0], proj[5][1], color);
    line(proj[2][0], proj[2][1], proj[6][0], proj[6][1], color);
    line(proj[3][0], proj[3][1], proj[7][0], proj[7][1], color);
  }

  void drawWireFrameSphere(float angleX, float angleY, float angleZ, int cx, int cy, int size, char color) {
    // 12 Vertices of an Icosahedron (Low-poly sphere)
    // We use the Golden Ratio to mathematically calculate perfectly spaced points
    const float a = 0.525731f; 
    const float b = 0.850651f; 

    float verts[12][3] = {
      {-a,  b,  0}, { a,  b,  0}, {-a, -b,  0}, { a, -b,  0},
      { 0, -a,  b}, { 0,  a,  b}, { 0, -a, -b}, { 0,  a, -b},
      { b,  0, -a}, { b,  0,  a}, {-b,  0, -a}, {-b,  0,  a}
    };

    // 30 Edges connecting the vertices
    int edges[30][2] = {
      {0,1}, {0,5}, {0,7}, {0,10}, {0,11},
      {1,5}, {1,7}, {1,8},  {1,9},
      {2,3}, {2,4}, {2,6},  {2,10}, {2,11},
      {3,4}, {3,6}, {3,8},  {3,9},
      {4,5}, {4,9}, {4,11},
      {5,9}, {5,11},
      {6,7}, {6,8}, {6,10},
      {7,8}, {7,10},
      {8,9}, {10,11}
    };

    int proj[12][2]; 

    // 1. Rotate and Project the 12 vertices
    for (int i = 0; i < 12; i++) {
      float x = verts[i][0], y = verts[i][1], z = verts[i][2];
      
      // 3D Rotations
      float y_rotX = y * cos(angleX) - z * sin(angleX);
      float z_rotX = y * sin(angleX) + z * cos(angleX);
      float x_rotY = x * cos(angleY) + z_rotX * sin(angleY);
      float z_rotY = -x * sin(angleY) + z_rotX * cos(angleY);
      float x_rotZ = x_rotY * cos(angleZ) - y_rotX * sin(angleZ);
      float y_rotZ = x_rotY * sin(angleZ) + y_rotX * cos(angleZ);
      
      // Perspective
      float z_perspective = z_rotY + 2.5;

      proj[i][0] = cx + (int)((x_rotZ / z_perspective) * size);
      proj[i][1] = cy + (int)((y_rotZ / z_perspective) * size);
    }

    // 2. Draw the 30 connecting lines
    for (int i = 0; i < 30; i++) {
      int p1 = edges[i][0];
      int p2 = edges[i][1];
      line(proj[p1][0], proj[p1][1], proj[p2][0], proj[p2][1], color);
    }
  }

  void line(int x1, int y1, int x2, int y2, char color)
  {
    int x, y, xe, ye;
    int dx = x2 - x1;
    int dy = y2 - y1;
    int dx1 = labs(dx);
    int dy1 = labs(dy);
    int px = 2 * dy1 - dx1;
    int py = 2 * dx1 - dy1;
    if(dy1 <= dx1)
    {
      if(dx >= 0)
      {
        x = x1;
        y = y1;
        xe = x2;
      }
      else
      {
        x = x2;
        y = y2;
        xe = x1;
      }
      dot(x, y, color);
      for(int i = 0; x < xe; i++)
      {
        x = x + 1;
        if(px < 0)
        {
          px = px + 2 * dy1;
        }
        else
        {
          if((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
          {
            y = y + 1;
          }
          else
          {
            y = y - 1;
          }
          px = px + 2 *(dy1 - dx1);
        }
        dot(x, y, color);
      }
    }
    else
    {
      if(dy >= 0)
      {
        x = x1;
        y = y1;
        ye = y2;
      }
      else
      {
        x = x2;
        y = y2;
        ye = y1;
      }
      dot(x, y, color);
      for(int i = 0; y < ye; i++)
      {
        y = y + 1;
        if(py <= 0)
        {
          py = py + 2 * dx1;
        }
        else
        {
          if((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
          {
            x = x + 1;
          }
          else
          {
            x = x - 1;
          }
          py = py + 2 * (dx1 - dy1);
        }
        dot(x, y, color);
      }
    }
  }
  
  inline void flush()
  {
    if(triangleRoot)
      triangleRoot->draw(*this);
  }

  inline void end()
  {
    char **b = backbuffer;
    backbuffer = frame;
    frame = b;    
  }

  void fillRect(int x, int y, int w, int h, int color)
  {
    if(x < 0)
    {
      w += x;
      x = 0;
    }
    if(y < 0)
    {
      h += y;
      y = 0;
    }
    if(x + w > xres)
      w = xres - x;
    if(y + h > yres)
      h = yres - y;
    for(int j = y; j < y + h; j++)
      for(int i = x; i < x + w; i++)
        dotFast(i, j, color);
  }

  void rect(int x, int y, int w, int h, int color)
  {
    fillRect(x, y, w, 1, color);
    fillRect(x, y, 1, h, color);
    fillRect(x, y + h - 1, w, 1, color);
    fillRect(x + w - 1, y, 1, h, color);
  }
};

