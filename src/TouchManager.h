/*
  TouchManager.h

  A C++11-compliant library for managing touch-sensitive areas 
  and their associated groups, 
*/

#pragma once

#include <vector>     // For dynamic arrays
#include <memory>     // For std::shared_ptr
#include <algorithm>  // For std::find_if
#include <string>     // For std::string
#include <cmath> 
#include <Adafruit_GFX.h> //THE graphics library!

// --- Standard GFX colors (for convenience) ---
#define C565_BLACK        0x0000 ///<   0,   0,   0
#define C565_NAVY         0x000F ///<   0,   0, 123
#define C565_DARKGREEN    0x03E0 ///<   0, 125,   0
#define C565_DARKCYAN     0x03EF ///<   0, 125, 123
#define C565_MAROON       0x7800 ///< 123,   0,   0
#define C565_PURPLE       0x780F ///< 123,   0, 123
#define C565_OLIVE        0x7BE0 ///< 123, 125,   0
#define C565_LIGHTGREY    0xC618 ///< 198, 195, 198
#define C565_DARKGREY     0x7BEF ///< 123, 125, 123
#define C565_BLUE         0x001F ///<   0,   0, 255
#define C565_GREEN        0x07E0 ///<   0, 255,   0
#define C565_CYAN         0x07FF ///<   0, 255, 255
#define C565_RED          0xF800 ///< 255,   0,   0
#define C565_MAGENTA      0xF81F ///< 255,   0, 255
#define C565_YELLOW       0xFFE0 ///< 255, 255,   0
#define C565_WHITE        0xFFFF ///< 255, 255, 255
#define C565_ORANGE       0xFD20 ///< 255, 165,   0
#define C565_GREENYELLOW  0xAFE5 ///< 173, 255,  41
#define C565_PINK         0xFC18 ///< 255, 130, 198

/**
 * @brief A X/Y coordinate struct of int16 for GFX operations.
 */
struct GFXPoint {
  int16_t x;
  int16_t y;
};

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

/**
 * @brief A polygon TouchShape defined by a variable list of points.
 */
class TouchPolygon : public TouchShape {
public:
  // Store a copy of the points
  std::vector<GFXPoint> points;

  TouchPolygon(const std::vector<GFXPoint>& _points,
               uint16_t _color, bool _filled, std::shared_ptr<TouchGroup> _group)
    : TouchShape(_group, _color, _filled), points(_points) {}
    //note Adafruit_GFX can't fill polygons so we won't use the filled option

  /**
   * @brief Draws a polygon outline by connecting all points with lines.
   */
  void draw(Adafruit_GFX* gfx) const override {
    if (points.size() < 2) {
      return; // Need at least 2 points to draw a line
      // TODO figure out a way to return an error
    }

    for (size_t i = 0; i < points.size() - 1; ++i) {
      gfx->drawLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, color);
    }
    // If we have at least 3 points, draw the final line connecting the last point back to the first
    if (points.size() >= 3) {
      gfx->drawLine(points.back().x, points.back().y, points.front().x, points.front().y, color);
      }
    }

  /**
   * @brief Checks if a point is inside the polygon using the
   * Ray Casting (even-odd) algorithm. Works for non-convex shapes.
   */
  bool contains(int px, int py) const override {
    int n = points.size();
    // if there are less than 3 points, it can't contain an area.
    if (n < 3) {
      return false;
    }

    bool isInside = false; //start false, 'cause an odd number of flips gives us a true
    // Loop through all edges (i, j) where j is the previous vertex
    for (int i = 0, j = n - 1; i < n; j = i++) {
      if (((points[i].y > py) != (points[j].y > py)) &&
          (px < (points[j].x - points[i].x) * (py - points[i].y) / (points[j].y - points[i].y) + points[i].x)) {
        isInside = !isInside; // Flip the state
      }
    }
    return isInside;
  }
};

class TouchText : public TouchShape {
public:
  std::string text;
  int x, y;
  int fontIndex;
  uint16_t color;
  uint8_t size;
  uint8_t direction; // Rotation (0-3)

  // We need to look up fonts, so we need a pointer to the font table
  const std::vector<const GFXfont*>* fontTable;

  // Cached bounds for touch detection, 
  // "mutable" so we can update these inside the const draw() function
  mutable int16_t bX, bY;
  mutable uint16_t bW, bH;
  mutable bool boundsCalculated;

  TouchText(int _x, int _y, std::string _text, int _fontIdx, 
            uint16_t _color, uint8_t _size, uint8_t _dir,
            std::shared_ptr<TouchGroup> _group,
            const std::vector<const GFXfont*>* _fonts)
    : TouchShape(_group, _color, false), // Filled doesn't apply to text
      x(_x), y(_y), text(_text), fontIndex(_fontIdx), 
      color(_color), size(_size), direction(_dir),
      fontTable(_fonts), boundsCalculated(false) {}

  void draw(Adafruit_GFX* gfx) const override {
    // 1. Save previous state
    uint8_t oldRot = gfx->getRotation();
    // We don't strictly need to save cursor/color as they are volatile anyway

    // 2. Set Font (Bounds Check the index!)
    if (fontTable && fontIndex >= 0 && fontIndex < (int)fontTable->size()) {
      gfx->setFont((*fontTable)[fontIndex]);
    } else {
      gfx->setFont(NULL); // Fallback to default system font
    }

    // 3. Apply User Settings
    gfx->setRotation(direction);
    gfx->setCursor(x, y);
    gfx->setTextColor(color);
    gfx->setTextSize(size);

    // 4. Calculate Bounds (If not done yet)
    // We do this here because we need the GFX context to measure text
    if (!boundsCalculated) {
      gfx->getTextBounds(text.c_str(), x, y, &bX, &bY, &bW, &bH);
      boundsCalculated = true;
    }

    // 5. Print
    gfx->print(text.c_str());

    // 6. Restore Rotation (Crucial!)
    gfx->setRotation(oldRot);
  }

  bool contains(int px, int py) const override {
    // If we haven't drawn yet, we don't know the size, so we can't be touched.
    if (!boundsCalculated) return false;

    // Standard rectangle check using the calculated bounds
    return (px >= bX) && (px < (bX + bW)) && (py >= bY) && (py < (bY + bH));
  }
};

// ----------------------------------------------------
//  MAIN TOUCH MANAGER CLASS
// ----------------------------------------------------

class TouchManager {
private:
  std::vector<std::shared_ptr<TouchGroup>> allGroups;
  std::vector<std::shared_ptr<TouchShape>> allShapes;
  Adafruit_GFX* m_gfx; // Pointer to the registered display

  std::vector<const GFXfont*> fontTable;

  std::shared_ptr<TouchGroup> getOrCreateGroup(int groupID) {
    if (!groupID) return nullptr;
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
  TouchManager() : m_gfx(nullptr) {}

  /**
   * @brief Binds the manager to a display for auto-drawing.
   * @param gfx A pointer to the Adafruit_GFX display object.
   */
  void begin(Adafruit_GFX* gfx) {
    m_gfx = gfx;
  }

  /**
   * @brief Adds a new rectangle associated with a group ID.
   * 
   * @param x left edge
   * @param y upper edge
   * @param w width
   * @param h height
   * @param color rgb
   * @param filled boolean
   * @param ID group ID or 0 aka nullptr
   */
  void addRect(int x, int y, int w, int h, uint16_t color, bool filled, int groupID) {
    auto group = getOrCreateGroup(groupID);
    auto newShape = std::make_shared<TouchRect>(x, y, w, h, color, filled, group);
    // Add it to the list
    allShapes.push_back(newShape);
    // Draw it if the display is registered
    if (m_gfx) {
      newShape->draw(m_gfx);
    }
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
    auto newShape = std::make_shared<TouchCircle>(x, y, d, color, filled, group);
    allShapes.push_back(newShape);
    if (m_gfx) {
      newShape->draw(m_gfx);
    }
  }

  /**
   * @brief Adds a new polygon associated with a group ID.
   * @param points A std::vector of GFXPoint structs.
   */
  void addPolygon(const std::vector<GFXPoint>& points, uint16_t color, int groupID) {
    auto group = getOrCreateGroup(groupID);
    auto newShape = std::make_shared<TouchPolygon>(points, color, false, group);
    allShapes.push_back(newShape);
    if (m_gfx) {
      newShape->draw(m_gfx);
    }
  }

  // Returns the index of the font to be used later
  int addFont(const GFXfont* font) {
    fontTable.push_back(font);
    return fontTable.size() - 1;
  }

  void addText(int x, int y, std::string text, int fontIndex, 
               uint16_t color, uint8_t size, uint8_t direction, int groupID) {
    auto group = getOrCreateGroup(groupID);
    // Pass the pointer to our fontTable so the object can look it up later
    auto newShape = std::make_shared<TouchText>(
      x, y, text, fontIndex, color, size, direction, group, &fontTable
    );
    allShapes.push_back(newShape);
    if (m_gfx) {
      newShape->draw(m_gfx);
    }
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