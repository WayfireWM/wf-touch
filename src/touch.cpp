#include <wayfire/touch/touch.hpp>
#include "math.hpp"

point_t wf::touch::finger_t::delta() const
{
    return this->current - this->origin;
}
