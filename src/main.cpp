#include <Arduino.h>
#include "TouchManager.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h> 

#define TFT_DC 26
#define TFT_CS 28
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

void setup() {
  tft.begin();
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
  Serial1.println("Ready2");

  // --- Define Groups and Rectangles ---
  
  // As you requested: multiple rectangles in one group.
  Serial1.println("ID 1: I1 Rx10y10h20w50cF00 Px70y10x75y30x70c00F G");
  delay(1);
  g_touchManager.addRect(10, 20, 40, 50, C565_RED, true, 1); // Group 1, Rect 1
  g_touchManager.addRect(70, 10, 30, 20, C565_BLUE, false, 1); // Group 1, Rect 2

  // Add another group with one, circle
  Serial1.println("ID 2: I2 Ox100,y35,d25,c0F0 G");
  g_touchManager.addCircle(100, 35, 25, C565_GREEN, false, 2); // (100, 35) center, 25 diameter

  // Add an overlapping rect for Z-order testing
  // This rect is added LAST, so it will be "on top"
  Serial1.println("ID 99 Overlaps 1");
  g_touchManager.addRect(30, 40, 50, 50, C565_PURPLE, true, 99); // Group 99, Rect 1

  g_touchManager.drawAll(&tft);

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

TS_Point p;

void loop() {
  if (ts.touched()) {
    // Get the touch point
    TS_Point np = remapTouchPoint(&tft, ts.getPoint());
    if (np.x != p.x || np.y != p.y) {
      p = np;
      Serial1.print(doTouch(p.x, p.y));
      Serial1.print("@ X:"); Serial1.print(p.x);
      Serial1.print("Y:"); Serial1.println(p.y);
    }

  }
  delay(10); // this speeds up the simulation
}
