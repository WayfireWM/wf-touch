#include <wayfire/touch/touch.hpp>
#include <glm/glm.hpp>

#include <iostream>
#define _ << " " <<
#define debug(x) #x << " = " << (x)

using namespace wf::touch;
/* -------------------------- Touch action ---------------------------------- */
wf::touch::touch_action_t::touch_action_t(int cnt_fingers, bool touch_down)
{
    this->cnt_fingers = cnt_fingers;
    this->type = touch_down ? EVENT_TYPE_TOUCH_DOWN : EVENT_TYPE_TOUCH_UP;

    this->target.x = -1e9;
    this->target.y = -1e9;
    this->target.width = 2e9;
    this->target.height = 2e9;
}

void wf::touch::touch_action_t::set_target(const touch_target_t& target)
{
    this->target = target;
}

static double find_max_delta(const gesture_state_t& state)
{
    double max_length = 0;
    for (auto& f : state.fingers)
    {
        max_length = std::max(max_length, glm::length(f.second.delta()));
    }

    return max_length;
}

bool wf::touch::touch_action_t::exceeds_tolerance(const gesture_state_t& state)
{
    return find_max_delta(state) > this->get_move_tolerance();
}

void wf::touch::touch_action_t::reset(uint32_t time)
{
    gesture_action_t::reset(time);
    this->released_fingers = 0;
}

action_status_t wf::touch::touch_action_t::update_state(
    const gesture_state_t& state, const gesture_event_t& event)
{
    // Allow motion events because of tolerance
    if (this->type != event.type && event.type != EVENT_TYPE_MOTION)
    {
        return ACTION_STATUS_CANCELLED;
    }

    for (auto& f : state.fingers)
    {
        point_t relevant_point = (this->type == EVENT_TYPE_TOUCH_UP ?
            f.second.current : f.second.origin);
        if (!this->target.contains(relevant_point))
        {
            return ACTION_STATUS_CANCELLED;
        }
    }

    if (event.type == EVENT_TYPE_MOTION)
    {
        return calculate_next_status(state, event, true);
    }

    if (this->type == EVENT_TYPE_TOUCH_DOWN)
    {
        if (this->cnt_fingers < (int)state.fingers.size())
        {
            return ACTION_STATUS_CANCELLED;
        }

        return calculate_next_status(state, event,
            this->cnt_fingers > (int)state.fingers.size());
    } else
    {
        ++this->released_fingers;
        return calculate_next_status(state, event,
            this->released_fingers < this->cnt_fingers);
    }
}

/*- -------------------------- Hold action ---------------------------------- */
wf::touch::hold_action_t::hold_action_t(int32_t threshold)
{
    this->threshold = threshold;
}

action_status_t wf::touch::hold_action_t::update_state(const gesture_state_t& state,
    const gesture_event_t& event)
{
    bool action_done = ((event.time - this->start_time) >= this->threshold);
    if (!action_done && event.type != EVENT_TYPE_MOTION)
    {
        return ACTION_STATUS_CANCELLED;
    }

    if (action_done)
    {
        return ACTION_STATUS_ALREADY_COMPLETED;
    }

    return calculate_next_status(state, event, true);
}

bool wf::touch::hold_action_t::exceeds_tolerance(const gesture_state_t& state)
{
    return find_max_delta(state) > this->get_move_tolerance();
}

/*- -------------------------- Drag action ---------------------------------- */
wf::touch::drag_action_t::drag_action_t(uint32_t direction, double threshold)
{
    this->direction = direction;
    this->threshold = threshold;
}

action_status_t wf::touch::drag_action_t::update_state(const gesture_state_t& state,
    const gesture_event_t& event)
{
    if (event.type != EVENT_TYPE_MOTION)
    {
        return ACTION_STATUS_CANCELLED;
    }

    const double dragged = state.get_center().get_drag_distance(this->direction);
    return calculate_next_status(state, event, dragged < this->threshold);
}

bool wf::touch::drag_action_t::exceeds_tolerance(const gesture_state_t& state)
{
    for (auto& f : state.fingers)
    {
        if (f.second.get_incorrect_drag_distance(this->direction) >
            this->get_move_tolerance())
        {
            return true;
        }
    }

    return false;
}

/*- -------------------------- Pinch action ---------------------------------- */
wf::touch::pinch_action_t::pinch_action_t(double threshold)
{
    this->threshold = threshold;
}

action_status_t wf::touch::pinch_action_t::update_state(const gesture_state_t& state,
    const gesture_event_t& event)
{
    if (event.type != EVENT_TYPE_MOTION)
    {
        return ACTION_STATUS_CANCELLED;
    }

    bool running = true;
    const double current_scale = state.get_pinch_scale();
    if (((this->threshold < 1.0) && (current_scale <= threshold)) ||
        ((this->threshold > 1.0) && (current_scale >= threshold)))
    {
        running = false;
    }

    return calculate_next_status(state, event, running);
}

bool wf::touch::pinch_action_t::exceeds_tolerance(const gesture_state_t& state)
{
    return glm::length(state.get_center().delta()) > this->get_move_tolerance();
}

/*- -------------------------- Rotate action ---------------------------------- */
wf::touch::rotate_action_t::rotate_action_t(double threshold)
{
    this->threshold = threshold;
}

action_status_t wf::touch::rotate_action_t::update_state(const gesture_state_t& state,
    const gesture_event_t& event)
{
    if (event.type != EVENT_TYPE_MOTION)
    {
        return ACTION_STATUS_CANCELLED;
    }

    bool running = true;
    const double current_scale = state.get_rotation_angle();
    if (((this->threshold < 0.0) && (current_scale <= threshold)) ||
        ((this->threshold > 0.0) && (current_scale >= threshold)))
    {
        running = false;
    }

    return calculate_next_status(state, event, running);
}

bool wf::touch::rotate_action_t::exceeds_tolerance(const gesture_state_t& state)
{
    return glm::length(state.get_center().delta()) > this->get_move_tolerance();
}
