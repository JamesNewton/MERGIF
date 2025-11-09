/*
 * Unit tests for the TouchManager class.
 * Run on the target device (Pico W)
 */

#include <Arduino.h>
#include <unity.h>

#include "TouchManager.h"

TouchManager testManager;

// setUp() is called by the test runner before EACH test function
void setUp(void) {
  testManager.clearAll(); // Ensure a clean state for every test

  // Group 1: Green square
  testManager.addRect(10, 10, 50, 50, C565_GREEN, true, 1);
  
  // Group 2: Blue circle
  testManager.addCircle(100, 35, 25, C565_BLUE, false, 2);
  
  // Group 99: Overlapping red circle (tests Z-order)
  testManager.addCircle(40, 40, 20, C565_RED, true, 99);
}

// tearDown() is called by the test runner after EACH test function
void tearDown(void) {
  // Not needed here, as setUp() clears everything.
}

// --- Test Cases ---

void test_touch_group_1_rect(void) {
  // Test 1: Hit the green square (Group 1)
  // This point (20, 20) is inside the rect but NOT the overlapping circle.
  int id = testManager.findGroupIDAt(20, 20);
  TEST_ASSERT_EQUAL(1, id); // Expect ID 1
}

void test_touch_group_2_circle(void) {
  // Test 2: Hit the blue circle (Group 2) at its top/bottom edges
  int id_top = testManager.findGroupIDAt(100, 10); // 35(center) - 25(radius)
  int id_bottom = testManager.findGroupIDAt(100, 60); // 35(center) + 25(radius)
  
  TEST_ASSERT_EQUAL(2, id_top);
  TEST_ASSERT_EQUAL(2, id_bottom);
}

void test_touch_z_order_overlap(void) {
  // Test 3: Z-ORDER TEST. This point (45, 45) is inside BOTH
  // the green square (Group 1) and the red circle (Group 99).
  // Since the red circle was added last, it's "on top" and should win.
  int id = testManager.findGroupIDAt(45, 45);
  TEST_ASSERT_EQUAL(99, id); // Expect ID 99
}

void test_touch_miss_blank_area(void) {
  // Test 4: Hit a blank area
  int id = testManager.findGroupIDAt(200, 200);
  TEST_ASSERT_EQUAL(-1, id); // Expect ID -1 (no match)
}


// --- The Test Runner ---

void setup() {
  // Initialize Serial for the test runner to report results
  Serial.begin(115200);
  delay(2000); // Wait for the serial connection

  UNITY_BEGIN(); // Start the test framework

  // Run all your test functions
  RUN_TEST(test_touch_group_1_rect);
  RUN_TEST(test_touch_group_2_circle);
  RUN_TEST(test_touch_z_order_overlap);
  RUN_TEST(test_touch_miss_blank_area);

  UNITY_END(); // End the test framework
}

void loop() {
  // The test firmware does not need a loop
}