#include <Arduino.h>
#include "TouchManager.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_DC 26
#define TFT_CS 28
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

TouchManager g_touchManager;

void simulateTouch(int x, int y) {
  Serial1.print("Touch: ");
  Serial1.print(x);
  Serial1.print(", ");
  Serial1.print(y);

  int foundID = g_touchManager.findGroupIDAt(x, y);

  if (foundID != -1) {
    Serial1.print(" ID: ");
    Serial1.println(foundID);
  } else {
    Serial1.println(" ignored.");
  }
}

void setup() {
  tft.begin();
  tft.setRotation(1);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Ready");

  Serial1.begin(115200);
  Serial1.println("Ready2");

  // --- Define Groups and Rectangles ---
  
  // As you requested: multiple rectangles in one group.
  Serial1.println("ID 1: I1 Rx10y10h20w50cF00 Px70y10x75y30x70c00F G");
  delay(1);
  g_touchManager.addRect(10, 10, 20, 50, ILI9341_RED, true, 1); // Group 1, Rect 1
  g_touchManager.addRect(70, 10, 5, 20, ILI9341_BLUE, false, 1); // Group 1, Rect 2

  // Add another group with one, circle
  Serial1.println("ID 2: I2 Ox100,y35,d25,c0F0 G");
  g_touchManager.addCircle(100, 35, 25, ILI9341_GREEN, false, 2); // (100, 35) center, 25 diameter

  // Add an overlapping rect for Z-order testing
  // This rect is added LAST, so it will be "on top"
  Serial1.println("ID 99 Overlaps 1");
  g_touchManager.addRect(40, 40, 50, 50, ILI9341_PURPLE, 99); // Group 99, Rect 1

  g_touchManager.drawAll(&tft);
  
  Serial1.println("\nTesting:");

  // --- Run Tests ---

  // Test 1: Hit the first rectangle of Group 1
  simulateTouch(20, 20); // Expected: 1

  // Test 2: Hit the second rectangle of Group 1
  simulateTouch(80, 20); // Expected: 1

  // Test 3: Hit the rectangle for Group 2
  simulateTouch(50, 90); // Expected: 2

  // Test 4: Hit a blank area
  simulateTouch(200, 200); // Expected: -1 (No match)

  // Test 5: Z-ORDER TEST. This point is inside both
  // Group 1 (10,10,50,50) and Group 99 (40,40,50,50).
  // Since Group 99 was added last, it should win.
  simulateTouch(45, 45); // Expected: 99

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100); // this speeds up the simulation
}
