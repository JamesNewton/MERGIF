/*
  TouchManager.h

  A C++11-compliant library for managing touch-sensitive areas 
  and their associated groups, 
*/

#pragma once

#include <vector>     // For dynamic arrays
#include <memory>     // For std::shared_ptr
#include <algorithm>  // For std::find_if

/**
 * @brief Represents a group, which only contains an ID for now.
 * This object is designed to be shared by multiple shapes.
 */
struct TouchGroup {
  int id;

  // Constructor
  explicit TouchGroup(int groupID) : id(groupID) {}
};

/**
 * @brief Represents a single rectangular area.
 * It holds its dimensions and smart pointer to its group.
 */
struct TouchRect {
  int x, y, w, h;
  std::shared_ptr<TouchGroup> group; // Pointer to the shared group

  // Constructor for a rect associated with a group
  TouchRect(int _x, int _y, int _w, int _h, std::shared_ptr<TouchGroup> _group)
    : x(_x), y(_y), w(_w), h(_h), group(_group) {}

  // Constructor for a rect with no group (e.g., a "dead" area)
  TouchRect(int _x, int _y, int _w, int _h)
    : x(_x), y(_y), w(_w), h(_h), group(nullptr) {}

  /**
   * @brief Checks if a point (px, py) is inside this rectangle.
   */
  bool contains(int px, int py) const {
    return (px >= x) && (px < (x + w)) && (py >= y) && (py < (y + h));
  }
};

/**
 * @brief Manages all touch groups and rectangles.
 */
class TouchManager {
private:
  // Dynamic vectors
  std::vector<std::shared_ptr<TouchGroup>> allGroups;
  std::vector<TouchRect> allRects;

  /**
   * @brief Finds a group by its ID.
   * @return A shared_ptr to the group, or nullptr if not found.
   */
  std::shared_ptr<TouchGroup> findGroup(int groupID) {
    auto it = std::find_if(
      allGroups.begin(), allGroups.end(),
      [groupID](const auto& groupPtr) {
        return groupPtr->id == groupID;
      });

    if (it != allGroups.end()) {
      return *it; // Return the existing shared_ptr
    }
    return nullptr;
  }

  /**
   * @brief Gets an existing group or creates a new one if it
   * doesn't exist.
   * @return A shared_ptr to the group.
   */
  std::shared_ptr<TouchGroup> getOrCreateGroup(int groupID) {
    std::shared_ptr<TouchGroup> group = findGroup(groupID);
    if (!group) {
      // Group doesn't exist, so create it and store it
      group = std::make_shared<TouchGroup>(groupID);
      allGroups.push_back(group);
    }
    return group;
  }

public:
  TouchManager() {}

  /**
   * @brief Adds a new shape and associates it with a group ID.
   * If the group ID doesn't already exist, a new group is created.
   */
  void addRectToGroup(int x, int y, int w, int h, int groupID) {
    // Get (or create) the shared group object
    std::shared_ptr<TouchGroup> group = getOrCreateGroup(groupID);

    // Add the new rectangle, passing it the shared_ptr to the group
    allRects.emplace_back(x, y, w, h, group);
  }

  /**
   * @brief Adds a new rectangle that is NOT associated with any group.
   * Touching this area will do nothing.
   */
  void addRect(int x, int y, int w, int h) {
    // Pass nullptr as the group
    allRects.emplace_back(x, y, w, h, nullptr);
  }

  /**
   * @brief Processes a touch at (px, py).
   * Searches the shapes in reverse order (Z-order) to find a match.
   *
   * @param px The x-coordinate of the touch.
   * @param py The y-coordinate of the touch.
   * @return The ID of the group that was touched, or -1 if no
   * match or if the matched rect has no group.
   */
  int findGroupIDAt(int px, int py) {
    // Iterate in reverse order (from last-added to first)
    // This way, more recently added shapes are checked first,
    // simulating a "Z-order" (on-top)
    for (auto it = allRects.rbegin(); it != allRects.rend(); ++it) {
      if (it->contains(px, py)) {
        // Found a matching shape!
        // Now, check if this shape actually belongs to a group.
        if (it->group) {
          // It does! Return the group's ID.
          return it->group->id;
        } else {
          // This shape was a match, but it has no group (it's a
          // "dead" area). We stop searching and return -1.
          return -1;
        }
      }
    }

    // No shapes around this point
    return -1;
  }

  /**
   * @brief Clears all defined groups and shapes.
   */
  void clearAll() {
      allRects.clear();
      allGroups.clear();
  }
};