#pragma once

#include <glm/vec2.hpp>
#include <map>

namespace wf
{
namespace touch
{
using point_t = glm::dvec2;

struct finger_t
{
    point_t origin;
    point_t current;

    /** Get movement vector */
    point_t delta() const;
};

/**
 * Movement direction.
 */
enum move_direction_t
{
    MOVE_DIRECTION_LEFT  = (1 << 0),
    MOVE_DIRECTION_RIGHT = (1 << 1),
    MOVE_DIRECTION_UP    = (1 << 2),
    MOVE_DIRECTION_DOWN  = (1 << 3),
};

/**
 * Contains all fingers.
 */
struct gesture_state_t
{
  public:
    // finger_id -> finger_t
    std::map<int, finger_t> fingers;

    /**
     * Find the center of the fingers.
     */
    finger_t get_center() const;
};

/**
 * Represents the current state of a gesture action.
 */
struct gesture_action_progress_t
{
    /** Whether the gesture action was broken */
    bool cancelled = false;


};

/**
 * Represents a part of the gesture.
 */
class gesture_action_t
{
  public:
    /**
     * Set the move tolerance.
     * This is the maximum amount the fingers may move in unwanted directions.
     */
    void set_move_tolerance(double tolerance);

    /** @return The move tolerance. */
    double get_move_tolerance() const;

    /**
     * Set the threshold for the action (in pixels or radians).
     * This is the amount of movement in the desired direction(s) needed for
     * the action to be completed.
     */
    void set_threshold(double threshold);

    /** @return The threshold */
    double get_threshold() const;

    /**
     * Set the duration of the action in milliseconds.
     * This is the minimal time needed for this action to be happening to
     * consider it complete.
     */
    void set_duration(double duration);

    /** @return The duration of the gesture action. */
    double get_duration() const;

    /**
     * Update the action's state according to the new gesture state.
     * @return True if the action is completed, false otherwise.
     */
    virtual bool update_state(const gesture_state_t& state) = 0;

    /**
     * Reset the action's state.
     */
    virtual void reset_state() {}

    virtual ~gesture_action_t() {}

  protected:
    gesture_action_t() {}

  private:
    double tolerance;
    double threshold;
    double duration;
};



}
}
