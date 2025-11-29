#include <Arduino.h>
#include "TouchManager.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h> 

#define TFT_DC 26
#define TFT_CS 28
#define TFT_ORENTATION 1
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_FT6206 ts = Adafruit_FT6206(); 

TouchManager g_touchManager;

/**
 * @brief Processes a touch at (x, y) and returns the found group ID.
 *
 * @param x The mapped x-coordinate of the touch.
 * @param y The mapped y-coordinate of the touch.
 * @return The ID of the group that was touched, or -1 if no match.
 */
int doTouch(int x, int y) {
  return g_touchManager.findGroupIDAt(x, y);
}

/**
 * @brief Remaps a raw touch point from an FT6206 (240x320 native)
 * to the rotated coordinate system of an Adafruit_GFX display.
 *
 * @param tft       A pointer to your display object (e.g., &tft).
 * @param raw_x     The original p.x from the touch controller.
 * @param raw_y     The original p.y from the touch controller.
 * @param mapped_x  A pointer to store the final, remapped x-coordinate.
 * @param mapped_y  A pointer to store the final, remapped y-coordinate.
 */
TS_Point remapTouchPoint(Adafruit_GFX* tft, TS_Point t) {
  TS_Point p;
  // Get the display's current rotation
  uint8_t rotation = tft->getRotation();
  switch (rotation) {
    case 0:  // Portrait (0,0 in top-left)
      p.x = t.x;
      p.y = t.y;
      break;

    case 1:  // Landscape (0,0 in top-left)
      p.x = t.y;
      p.y = (tft->height() - 1) - t.x;
      break;

    case 2:  // Portrait-Inverted (0,0 in top-left)
      p.x = (tft->width() - 1) - t.x;
      p.y = (tft->height() - 1) - t.y;
      break;

    case 3:  // Landscape-Inverted (0,0 in top-left)
      p.x = (tft->width() - 1) - t.y;
      p.y = t.x;
      break;
  }
  return p;
}

uint16_t radix;
int n; //a global to hold accumulated digits as a number
int attr[('Z' - 'A')]; //attributes are an array of letters
#define LTR(x) (x - 'a')
char c;
boolean quoting;
String text;

TS_Point p;
std::vector<GFXPoint> points;

std::vector<int> series;
std::vector<std::vector<int>> graph;

void printAttrib() {
  for (int i = 0; i<sizeof(attr)/sizeof(attr[0]); i++) {
    Serial1.print((char)(i + 'a'));
    Serial1.print("="); 
    if ('c' == i + 'a') {
      Serial1.print("#"); 
      Serial1.print(attr[i], HEX);
    } else {
      Serial1.print(attr[i]);
    }
    Serial1.print(", "); 
  }
  Serial1.println(".");
}

void printPoints() {
  for (int i = 0; i<points.size(); i++) {
    Serial1.print(i); Serial1.print("=("); 
    Serial1.print(points[i].x); Serial1.print(", "); 
    Serial1.print(points[i].y); Serial1.print(") ");
  }
  Serial1.println(".");
}


void setup() {
  tft.begin();
  g_touchManager.begin(&tft);
  radix = 10;
  n = 0; //current number in radix
  c = 0; //current character
  quoting = false; //are we taking in quoted text?
  text = ""; //The quoted text


  tft.setRotation(1);

  tft.setTextColor(C565_WHITE);
  tft.setTextSize(2);
  tft.print("Ready");

  Wire.setSDA(4); // Use GPIO 4 for SDA
  Wire.setSCL(5); // Use GPIO 5 for SCL
  Wire.begin(); 

  if (! ts.begin(40)) {
    Serial1.println(".");
    while (1);
  }
  Serial1.println("Touch screen up");

  Serial1.begin(115200);
  // Serial1.println("Ready2");
  g_touchManager.addText( 
    0, //X
    0, //Y
    "Ready2", 0, C565_YELLOW, 
    1 + 1, //size
    (0 + TFT_ORENTATION) % 4, //direction
    0  //ID
  );

  // --- Define Groups and Rectangles ---
  
  // As you requested: multiple rectangles in one group.
  Serial1.println("1i 10x 20y 40h 50w #f800C R");
  g_touchManager.addRect(10, 20, 40, 50, C565_RED, true, 1); // Group 1, Rect 1
  Serial1.println("1i 70x 10y 30h 20w #001fC R");
  g_touchManager.addRect(70, 10, 30, 20, C565_BLUE, false, 1); // Group 1, Rect 2

  // Add another group with one, circle
  Serial1.println("2i 100x 35y 25d #07e0C O");
  g_touchManager.addCircle(100, 35, 25, C565_GREEN, false, 2); // (100, 35) center, 25 diameter

  // Add a circle, not in a group
  Serial1.println("0i 100x 35y 25d #fd20C O");
  g_touchManager.addCircle(200, 100, 50, C565_ORANGE, true, 0); 
  
  // Add an overlapping rect for Z-order testing
  // This rect is added LAST, so it will be "on top"
  Serial1.println("99i 30x 40y 50w 50h 30735c R");
  g_touchManager.addRect(30, 40, 50, 50, C565_PURPLE, true, 99); // Group 99, Rect 1

  Serial1.println("3i 220x 20y P 270x 20y P ");
  Serial1.println("220x 70y P 270x 70y P #ffe0C L");
  points.push_back({220, 20});
  points.push_back({270, 20});
  points.push_back({220, 70});
  points.push_back({270, 70});
  g_touchManager.addPolygon(points, C565_YELLOW, 3); 
  points.clear();

  // g_touchManager.drawAll(&tft);

  Serial1.println("\nTesting:");

  // --- Run Tests ---

  // Test 1: Hit the first rectangle of Group 1
  if(1 != doTouch(15, 20)) Serial1.println("Error: 15,20 should be in group 1");

  // Test 2: Hit the second rectangle of Group 1
  if(1 != doTouch(72, 15)) Serial1.println("Error: 72,15 should be in group 1");
 
  // Test 3: Hit the rectangle for Group 2
  if(2 != doTouch(100, 33)) Serial1.println("Error: 100,33 should be in group 2");

  // Test 4: Hit a blank area
  if(-1 != doTouch(200, 200)) Serial1.println("Error: 200,200 should not be in a group -1");

  // Test 5: Z-ORDER TEST. This point is inside both
  // Group 1 (10,10,50,50) and Group 99 (40,40,50,50).
  // Since Group 99 was added last, it should win.
  if(99 != doTouch(45, 45)) Serial1.println("Error: 45,45 should be in group 99"); 

}


void loop() {
  if (ts.touched()) {
    // Get the touch point
    TS_Point np = remapTouchPoint(&tft, ts.getPoint());
    if (np.x != p.x || np.y != p.y) {
      p = np;
      // printAttrib();
      // Serial1.print(n); Serial1.print(" ");
      Serial1.print(doTouch(p.x, p.y));
      Serial1.print("@ X:"); Serial1.print(p.x);
      Serial1.print("Y:"); Serial1.println(p.y);
    }

  }
  if (Serial1.available()) {
    if (Serial1.peek() == 34) { //about to get a quote
      if (c == 34) { //getting a double quote
        if (quoting) {// just two quotes
          text = ""; //empty string
        } else { // three quotes
          text += '"'; //this is an escaped quote
          quoting = true; //keep quoting
        }
      } else { //single quote
        if (quoting) { //end the quoted string
          quoting = false;
        } else {
          quoting = true;
          // Serial1.println(">");
          text = ""; //start a new string
        }
      }
      Serial1.print(c = Serial1.read()); //quote managed
      return;
    }
    c = Serial1.read();
    Serial1.print(c);
    if (quoting) {
      text += c;
      return;
    }
    if (isdigit(c) || (radix > 10 && c >= 'a' && c <= 'f')) {
      //Note: don't use isHexadecimalDigit(c) so that 'C' (or whatever) can pop us out
      n *= radix;
      if (radix > 10 && c >= 'a' && c <= 'f') { //a-f are numbers now
        n += (int)(c - 'a' + 10);
      } else {
        n += (int)(c - '0');
      }
      // Serial1.print("\n");Serial1.print(radix); Serial1.print(" "); Serial1.println(n); 
      return;
    }

    switch (c) {

      case 'Z': //Zero out the display and objects
        tft.fillScreen(C565_BLACK);
        g_touchManager.clearAll();
        for (int i = 0; i<sizeof(attr)/sizeof(attr[0]); i++) { 
          attr[i] = 0; 
        }
        points.clear(); n = 0; radix = 10;
        delay(100);
        break;

      case 'R': //Rectangle
        g_touchManager.addRect(
          attr[LTR('x')], attr[LTR('y')], attr[LTR('w')], attr[LTR('h')], 
          attr[LTR('c')], true, attr[LTR('i')] 
        );
        n = 0; radix = 10;
        break;
      
      case 'O': //Circle
        g_touchManager.addCircle(
          attr[LTR('x')], attr[LTR('y')], attr[LTR('d')], 
          attr[LTR('c')], true, attr[LTR('i')] 
        );
        n = 0; radix = 10;
        break;

      case 'P': //Point
        points.push_back(
          (GFXPoint){(int16_t)attr[LTR('x')], (int16_t)attr[LTR('y')]}
        );
        break;

      case 'L': //Line
        g_touchManager.addPolygon(points, attr[LTR('c')], attr[LTR('i')]);
        points.clear(); n = 0; radix = 10;
        break;

      case '#': //Hex set radix to 16
        radix = (0 == n ? 16 : n);
        n = 0;
        break;

      case 'C': //Color (also 'c' if not in hex)
        attr[LTR('c')] = n;
        // Serial1.print(n); Serial1.print(" "); Serial1.println(n, HEX);
        n = 0; radix = 10;
        break; 

      case 'T': //Text
        Serial1.println(text);
        g_touchManager.addText(
          attr[LTR('x')], attr[LTR('y')], text.c_str(), attr[LTR('f')], 
          attr[LTR('c')], 
          attr[LTR('s')] + 1, //size default is 1
          (attr[LTR('d')] + TFT_ORENTATION) % 4, //orentation; relate to display orientation
          attr[LTR('i')]  
        );
        break;

      case ',': //series
        series.push_back(n);
        n = 0; radix = 10;
        break;

      case 'G': { //Graph
        series.push_back(n);
        n = 0; radix = 10;
        graph.push_back(series);
        int y_count = attr[LTR('w')];
        while (graph.size() > y_count) {
          graph.erase(graph.begin());
        }
        Serial1.println("");
        for (const auto& aseries : graph) {
          for (const auto& element : aseries) {
            Serial1.print(element);
            Serial1.print(",\t");
          }
          Serial1.println("");
        }
        series.clear();
        break;
      }

      case '?':
        printAttrib();
        printPoints();
        break;

      default:
        break;
    }

    if ('a' <= c && c <= 'z') {
      attr[LTR(c)] = n;
      // Serial1.print("\nslot "); Serial1.print(LTR(c));
      // Serial1.print((char)(LTR(c) + 'a'));
      // Serial1.print("="); Serial1.println(attr[LTR(c)]);
      radix = 10; //back to decimal
      n = 0;
      return;
    }
  }
  delay(100); // this speeds up the simulation
}
