#include <wayfire/touch/touch.hpp>

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
      default:
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

wf::touch::gesture_action_t& wf::touch::gesture_action_t::set_duration(uint32_t duration)
{
    this->duration = duration;
    return *this;
}

std::optional<uint32_t> wf::touch::gesture_action_t::get_duration() const
{
    return this->duration;
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
    std::unique_ptr<timer_interface_t> timer;

    void start_gesture(uint32_t time)
    {
        status = ACTION_STATUS_RUNNING;
        finger_state.fingers.clear();
        current_action = 0;
        actions[0]->reset(time);
        start_timer();
    }

    void start_timer()
    {
        if (auto dur = actions[current_action]->get_duration())
        {
            timer->set_timeout(*dur, [=] ()
            {
                update_state(gesture_event_t{.type = EVENT_TYPE_TIMEOUT});
            });
        }
    }

    void update_state(const gesture_event_t& event)
    {
        if (status != ACTION_STATUS_RUNNING)
        {
            // nothing to do
            return;
        }

        auto& idx = current_action;

        auto old_finger_state = finger_state;
        finger_state.update(event);

        auto next_action = [&] () -> bool
        {
            timer->reset();
            ++idx;
            if (idx < actions.size())
            {
                actions[idx]->reset(event.time);
                finger_state.reset_origin();
                start_timer();
                return true;
            }

            return false;
        };

        action_status_t pending_status = actions[idx]->update_state(finger_state, event);
        switch (pending_status)
        {
          case ACTION_STATUS_RUNNING:
            return; // nothing more to do

          case ACTION_STATUS_CANCELLED:
            this->status = ACTION_STATUS_CANCELLED;
            timer->reset();
            cancelled();
            return;

          case ACTION_STATUS_COMPLETED:
            bool has_next = next_action();
            if (!has_next)
            {
                this->status = ACTION_STATUS_COMPLETED;
                completed();
                return;
            }
        }
    }
};

void wf::touch::gesture_t::set_timer(std::unique_ptr<timer_interface_t> timer)
{
    priv->timer = std::move(timer);
}

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
    assert(priv->timer);
    priv->update_state(event);
}

wf::touch::action_status_t wf::touch::gesture_t::get_status() const
{
    return priv->status;
}

void wf::touch::gesture_t::reset(uint32_t time)
{
    assert(priv->timer);
    if (priv->status == ACTION_STATUS_RUNNING)
    {
        return;
    }

    priv->start_gesture(time);
}

wf::touch::gesture_builder_t::gesture_builder_t() {}

wf::touch::gesture_builder_t& wf::touch::gesture_builder_t::on_completed(gesture_callback_t callback)
{
    this->_on_completed = callback;
    return *this;
}

wf::touch::gesture_builder_t& wf::touch::gesture_builder_t::on_cancelled(gesture_callback_t callback)
{
    this->_on_cancelled = callback;
    return *this;
}

wf::touch::gesture_t wf::touch::gesture_builder_t::build()
{
    return gesture_t(std::move(actions), _on_completed, _on_cancelled);
}
