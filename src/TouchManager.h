/*
  TouchManager.h

  A C++11-compliant library for managing touch-sensitive areas 
  and their associated groups, 
*/

#pragma once

#include <vector>     // For dynamic arrays
#include <memory>     // For std::shared_ptr
#include <algorithm>  // For std::find_if
#include <cmath> 
#include <Adafruit_GFX.h> //THE graphics library!

/**
 * @brief Groups just attach an ID to a list of objects
 */
struct TouchGroup {
  int id;

  // Constructor
  explicit TouchGroup(int groupID) : id(groupID) {}
};


// ----------------------------------------------------
//  BASE CLASS FOR ALL SHAPES
// ----------------------------------------------------

/**
 * @brief Base class contract for any drawable, touchable shape.
 */
class TouchShape {
public:
  // pointer to the group for shape or nullptr if not in a group.
  std::shared_ptr<TouchGroup> group;
  
  // GFX properties
  uint16_t color;
  bool filled;

  // Constructor
  TouchShape(std::shared_ptr<TouchGroup> g, uint16_t c, bool f)
    : group(g), color(c), filled(f) {}
  
  // Destructor (base class best practice)
  virtual ~TouchShape() {}

  /**
   * @brief Pure virtual function checks if point inside this shape.
   * @param px The point's x-coordinate.
   * @param py The point's y-coordinate.
   * @return true if the point is inside, false otherwise.
   */
  virtual bool contains(int px, int py) const = 0;

  /**
   * @brief Pure virtual function draw the shape.
   * @param gfx Point to Adafruit_GFX.
   */
  virtual void draw(Adafruit_GFX* gfx) const = 0;
};


// ----------------------------------------------------
//  SHAPES
// ----------------------------------------------------

/**
 * @brief Rect shape.
 */
class TouchRect : public TouchShape {
public:
  int x, y, w, h;

  TouchRect(int _x, int _y, int _w, int _h, 
            uint16_t _color, bool _filled, std::shared_ptr<TouchGroup> _group)
    : TouchShape(_group, _color, _filled), x(_x), y(_y), w(_w), h(_h) {}

  bool contains(int px, int py) const override {
    return (px >= x) && (px < (x + w)) && (py >= y) && (py < (y + h));
  }

  void draw(Adafruit_GFX* gfx) const override {
    if (filled) {
      gfx->fillRect(x, y, w, h, color);
    } else {
      gfx->drawRect(x, y, w, h, color);
    }
  }
};

/**
 * @brief 'O' circle shape.
 */
class TouchCircle : public TouchShape {
public:
  int x, y, d; // Center (x, y) and diameter (d)

  TouchCircle(int _x, int _y, int _d, 
              uint16_t _color, bool _filled, std::shared_ptr<TouchGroup> _group)
    : TouchShape(_group, _color, _filled), x(_x), y(_y), d(_d) {}

  bool contains(int px, int py) const override {
    // Use distance formula: (px-x)^2 + (py-y)^2 <= r^2
    // Use int32_t to avoid overflow when squaring
    int32_t dx = px - x;
    int32_t dy = py - y;
    int32_t r = d / 2;
    return (dx * dx + dy * dy) <= (int32_t)(r * r);
  }

  void draw(Adafruit_GFX* gfx) const override {
    if (filled) {
      gfx->fillCircle(x, y, d / 2, color);
    } else {
      gfx->drawCircle(x, y, d / 2, color);
    }
  }
};


// ----------------------------------------------------
//  MAIN TOUCH MANAGER CLASS
// ----------------------------------------------------

class TouchManager {
private:
  std::vector<std::shared_ptr<TouchGroup>> allGroups;
  std::vector<std::shared_ptr<TouchShape>> allShapes;

  std::shared_ptr<TouchGroup> getOrCreateGroup(int groupID) {
    auto it = std::find_if(allGroups.begin(), allGroups.end(),
                           [groupID](const auto& groupPtr) {
                             return groupPtr->id == groupID;
                           });

    if (it != allGroups.end()) {
      return *it; // Return existing group
    }
    
    // Create new group
    auto newGroup = std::make_shared<TouchGroup>(groupID);
    allGroups.push_back(newGroup);
    return newGroup;
  }

public:
  TouchManager() {}

  /**
   * @brief Adds a new rectangle associated with a group ID.
   * 
   * @param x left edge
   * @param y upper edge
   * @param w width
   * @param h height
   * @param color rgb
   * @param filled boolean
   * @param ID
   */
  void addRect(int x, int y, int w, int h, uint16_t color, bool filled, int groupID) {
    auto group = getOrCreateGroup(groupID);
    allShapes.push_back(
      std::make_shared<TouchRect>(x, y, w, h, color, filled, group)
    );
  }

  /**
   * @brief Adds a new rectangle NOT associated with any group.
   * 
   * @param x left edge
   * @param y upper edge
   * @param w width
   * @param h height
   * @param color rgb
   * @param filled boolean
   */
  void addRect(int x, int y, int w, int h, uint16_t color, bool filled) {
    allShapes.push_back(
      std::make_shared<TouchRect>(x, y, w, h, color, filled, nullptr)
    );
  }

  /**
   * @brief Adds a new circle associated with a group ID.
   * 
   * @param x left edge
   * @param y upper edge
   * @param d diameter
   * @param color rgb
   * @param filled boolean
   * @param ID
   */
  void addCircle(int x, int y, int d, uint16_t color, bool filled, int groupID) {
    auto group = getOrCreateGroup(groupID);
    allShapes.push_back(
      std::make_shared<TouchCircle>(x, y, d, color, filled, group)
    );
  }

  /**
   * @brief Adds a new circle NOT associated with any group.
   * 
   * @param x left edge
   * @param y upper edge
   * @param d diameter
   * @param color rgb
   * @param filled boolean
   */
  void addCircle(int x, int y, int d, uint16_t color, bool filled) {
    allShapes.push_back(
      std::make_shared<TouchCircle>(x, y, d, color, filled, nullptr)
    );
  }

  /**
   * @brief Processes a touch at (px, py).
   * Searches all shapes in reverse order (Z-order) to find a match.
   *
   * @return The ID of the group that was touched, or -1 if no match.
   */
  int findGroupIDAt(int px, int py) {
    // Iterate in reverse order (Z-order: last-added is checked first)
    for (auto it = allShapes.rbegin(); it != allShapes.rend(); ++it) {
      if ((*it)->contains(px, py)) {
        // Found a matching shape!
        if ((*it)->group) {
          return (*it)->group->id; // Return its group ID
        } else {
          return -1; // Matched a "dead" shape (no group)
        }
      }
    }
    return -1; // No shape contained this point
  }

  /**
   * @brief Draws all shapes to the screen.
   * Iterates in forward order, so first-added is "on the bottom".
   * @param gfx A pointer to the Adafruit_GFX display object.
   */
  void drawAll(Adafruit_GFX* gfx) {
    for (const auto& shape : allShapes) {
      shape->draw(gfx);
    }
  }

  /**
   * @brief Clears all defined groups and shapes.
   */
  void clearAll() {
    allShapes.clear();
    allGroups.clear();
  }
};