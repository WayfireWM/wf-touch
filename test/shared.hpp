#define _USE_MATH_DEFINES
#include <cmath>

#include <wayfire/touch/touch.hpp>
using namespace wf::touch;

static finger_t finger_in_dir(double x, double y)
{
    return finger_t {
        .origin = {0, 0},
        .current = {x, y}
    };
}

const uint32_t lu = MOVE_DIRECTION_LEFT | MOVE_DIRECTION_UP;
const uint32_t ld = MOVE_DIRECTION_LEFT | MOVE_DIRECTION_DOWN;
const uint32_t rd = MOVE_DIRECTION_RIGHT | MOVE_DIRECTION_DOWN;
const uint32_t ru = MOVE_DIRECTION_RIGHT | MOVE_DIRECTION_UP;

static finger_t finger_2p(double x, double y, double a, double b)
{
    return finger_t {
        .origin = {x, y},
        .current = {a, b}
    };
}
