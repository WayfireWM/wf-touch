#include <wayfire/touch/touch.hpp>

#include <iostream>
#define _ << " " <<
#define debug(x) #x << " = " << (x)

using namespace wf::touch;

point_t wf::touch::finger_t::delta() const
{
    return this->current - this->origin;
}

finger_t wf::touch::gesture_state_t::get_center() const
{
    finger_t center;
    center.origin = {0, 0};
    center.current = {0, 0};

    for (auto& f : this->fingers)
    {
        center.origin += f.second.origin;
        center.current += f.second.current;
    }

    center.origin /= this->fingers.size();
    center.current /= this->fingers.size();
    return center;
}

void wf::touch::gesture_state_t::update(const gesture_event_t& event)
{
    switch (event.type)
    {
      case EVENT_TYPE_TOUCH_DOWN:
        fingers[event.finger].origin = event.pos;
        // fallthrough
      case EVENT_TYPE_MOTION:
        fingers[event.finger].current = event.pos;
        break;
      case EVENT_TYPE_TOUCH_UP:
        fingers.erase(event.finger);
        break;
    }
}

void wf::touch::gesture_state_t::reset_origin()
{
    for (auto& f : fingers)
    {
        f.second.origin = f.second.current;
    }
}

void wf::touch::gesture_action_t::set_move_tolerance(double tolerance)
{
    this->tolerance = tolerance;
}

double wf::touch::gesture_action_t::get_move_tolerance() const
{
    return this->tolerance;
}

void wf::touch::gesture_action_t::set_duration(uint32_t duration)
{
    this->duration = duration;
}

uint32_t wf::touch::gesture_action_t::get_duration() const
{
    return this->duration;
}

action_status_t wf::touch::gesture_action_t::calculate_next_status(
    const gesture_state_t& state, const gesture_event_t& last_event, bool running)
{
    uint32_t elapsed = last_event.time - this->start_time;
    if ((elapsed > this->get_duration()) || exceeds_tolerance(state))
    {
        return ACTION_STATUS_CANCELLED;
    }

    return running ? ACTION_STATUS_RUNNING : ACTION_STATUS_COMPLETED;
}

bool wf::touch::gesture_action_t::exceeds_tolerance(const gesture_state_t&)
{
    return false;
}

void wf::touch::gesture_action_t::reset(uint32_t time)
{
    this->start_time = time;
}

bool wf::touch::touch_target_t::contains(const point_t& pt) const
{
    return x <= pt.x && pt.x < x + width &&
        y <= pt.y && pt.y < y + height;
}

class wf::touch::gesture_t::impl
{
  public:
    gesture_callback_t completed;
    gesture_callback_t cancelled;

    std::vector<std::unique_ptr<gesture_action_t>> actions;
    size_t current_action = 0;
    action_status_t status = ACTION_STATUS_CANCELLED;

    gesture_state_t finger_state;
};

wf::touch::gesture_t::gesture_t(std::vector<std::unique_ptr<gesture_action_t>> actions,
        gesture_callback_t completed, gesture_callback_t cancelled)
{
    assert(!actions.empty());
    this->priv = std::make_unique<impl>();
    priv->actions = std::move(actions);
    priv->completed = completed;
    priv->cancelled = cancelled;
}

wf::touch::gesture_t::~gesture_t() = default;

double wf::touch::gesture_t::get_progress() const
{
    if (priv->status == ACTION_STATUS_CANCELLED)
    {
        return 0.0;
    }

    return 1.0 * priv->current_action / priv->actions.size();
}

void wf::touch::gesture_t::update_state(const gesture_event_t& event)
{
    if (priv->status != ACTION_STATUS_RUNNING)
    {
        // nothing to do
        return;
    }

    auto& actions = priv->actions;
    auto& idx = priv->current_action;

    auto old_finger_state = priv->finger_state;
    priv->finger_state.update(event);

    auto next_action = [&] ()
    {
        ++idx;
        if (idx < actions.size())
        {
            actions[idx]->reset(event.time);
            priv->finger_state.reset_origin();
        }
    };

    /** Go through all ALREADY_COMPLETED gestures */
    action_status_t status;
    while (idx < actions.size())
    {
        status = actions[idx]->update_state(priv->finger_state, event);
        if (status == ACTION_STATUS_ALREADY_COMPLETED)
        {
            /* Make sure that the previous finger state is marked as origin,
             * because the last update is not consumed by the last action */
            priv->finger_state = old_finger_state;
            next_action();
            priv->finger_state.update(event);
        } else
        {
            break;
        }
    }

    switch (status)
    {
      case ACTION_STATUS_RUNNING:
        return; // nothing more to do
      case ACTION_STATUS_CANCELLED:
        priv->status = ACTION_STATUS_CANCELLED;
        break;
      case ACTION_STATUS_ALREADY_COMPLETED:
        // fallthrough
      case ACTION_STATUS_COMPLETED:
        if (idx < actions.size())
        {
            next_action();
        }

        if (idx == actions.size())
        {
            priv->status = ACTION_STATUS_COMPLETED;
        }
        break;
    }

    if (priv->status == ACTION_STATUS_CANCELLED)
    {
        priv->cancelled();
    }

    if (priv->status == ACTION_STATUS_COMPLETED)
    {
        priv->completed();
    }
}

void wf::touch::gesture_t::reset(uint32_t time)
{
    priv->status = ACTION_STATUS_RUNNING;
    priv->finger_state.fingers.clear();
    priv->current_action = 0;
    priv->actions[0]->reset(time);
}
