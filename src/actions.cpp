#include <wayfire/touch/touch.hpp>
#include <glm/glm.hpp>

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

wf::touch::touch_action_t& wf::touch::touch_action_t::set_target(const touch_target_t& target)
{
    this->target = target;
    return *this;
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
    return find_max_delta(state) > this->move_tolerance;
}

void wf::touch::touch_action_t::reset(uint32_t time)
{
    gesture_action_t::reset(time);
    this->cnt_touch_events = 0;
}

action_status_t wf::touch::touch_action_t::update_state(
    const gesture_state_t& state, const gesture_event_t& event)
{
    if (exceeds_tolerance(state))
    {
        return ACTION_STATUS_CANCELLED;
    }

    switch (event.type)
    {
      case EVENT_TYPE_MOTION:
        return ACTION_STATUS_RUNNING;
      case EVENT_TYPE_TIMEOUT:
        return ACTION_STATUS_CANCELLED;

      case EVENT_TYPE_TOUCH_UP: // fallthrough
      case EVENT_TYPE_TOUCH_DOWN:
        if (this->type != event.type)
        {
            // down when we want up or vice versa
            return ACTION_STATUS_CANCELLED;
        }

        for (auto& f : state.fingers)
        {
            point_t relevant_point = (this->type == EVENT_TYPE_TOUCH_UP ? f.second.current : f.second.origin);
            if (!this->target.contains(relevant_point))
            {
                return ACTION_STATUS_CANCELLED;
            }
        }

        this->cnt_touch_events++;
        if (this->cnt_touch_events == this->cnt_fingers)
        {
            return ACTION_STATUS_COMPLETED;
        } else
        {
            return ACTION_STATUS_RUNNING;
        }
    }

    return ACTION_STATUS_RUNNING;
}

/*- -------------------------- Hold action ---------------------------------- */
wf::touch::hold_action_t::hold_action_t(int32_t threshold)
{
    set_duration(threshold);
}

action_status_t wf::touch::hold_action_t::update_state(const gesture_state_t& state,
    const gesture_event_t& event)
{
    switch (event.type)
    {
      case EVENT_TYPE_MOTION:
        if (exceeds_tolerance(state))
        {
            return ACTION_STATUS_CANCELLED;
        } else
        {
            return ACTION_STATUS_RUNNING;
        }
      case EVENT_TYPE_TIMEOUT:
        return ACTION_STATUS_COMPLETED;
      default:
        return ACTION_STATUS_CANCELLED;
    }
}

bool wf::touch::hold_action_t::exceeds_tolerance(const gesture_state_t& state)
{
    return find_max_delta(state) > this->move_tolerance;
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

    if (exceeds_tolerance(state))
    {
        return ACTION_STATUS_CANCELLED;
    }

    const double dragged = state.get_center().get_drag_distance(this->direction);
    if (dragged >= this->threshold)
    {
        return ACTION_STATUS_COMPLETED;
    } else
    {
        return ACTION_STATUS_RUNNING;
    }
}

bool wf::touch::drag_action_t::exceeds_tolerance(const gesture_state_t& state)
{
    for (auto& f : state.fingers)
    {
        if (f.second.get_incorrect_drag_distance(this->direction) > move_tolerance)
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

    if (exceeds_tolerance(state))
    {
        return ACTION_STATUS_CANCELLED;
    }

    const double current_scale = state.get_pinch_scale();
    if (((this->threshold < 1.0) && (current_scale <= threshold)) ||
        ((this->threshold > 1.0) && (current_scale >= threshold)))
    {
        return ACTION_STATUS_COMPLETED;
    }

    return ACTION_STATUS_RUNNING;
}

bool wf::touch::pinch_action_t::exceeds_tolerance(const gesture_state_t& state)
{
    return glm::length(state.get_center().delta()) > this->move_tolerance;
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

    if (exceeds_tolerance(state))
    {
        return ACTION_STATUS_CANCELLED;
    }

    const double current_scale = state.get_rotation_angle();
    if (((this->threshold < 0.0) && (current_scale <= threshold)) ||
        ((this->threshold > 0.0) && (current_scale >= threshold)))
    {
        return ACTION_STATUS_COMPLETED;
    }

    return ACTION_STATUS_RUNNING;
}

bool wf::touch::rotate_action_t::exceeds_tolerance(const gesture_state_t& state)
{
    return glm::length(state.get_center().delta()) > this->move_tolerance;
}
