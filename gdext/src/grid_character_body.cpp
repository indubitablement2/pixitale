#include <cmath>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "godot_cpp/variant/utility_functions.hpp"
#include "grid_character_body.h"
#include "grid.h"
#include "cell.hpp"

bool is_blocking(int x, int y) {
    if (x < 0 || x >= Grid::width || y < 0 || y >= Grid::height) {
        return false;
    }

    auto cell = *(Grid::cells + x + y * Grid::width);
    auto mat_idx = Cell::material_idx(cell);
    
    if (mat_idx == 0) {
        // Empty cell.
        return false;
    }

    auto mat_ptr = Grid::cell_materials + mat_idx;
    if (mat_ptr->cell_collision == Grid::CELL_COLLISION_SOLID) {
        return true;
    }

    return false;
}

bool block_or_step_left(
    int &left,
    int &right,
    int &top,
    int &bot,
    Vector2 &new_position,
    Vector2 &velocity,
    Vector2 &step_offset,
    int max_steps_height
) {
    bool blocked = false;

    int floor = (bot - top) + 1;
    for (int y = top; y <= bot; y++) {
        floor--;

        if (is_blocking(left, y)) {
            if (floor <= max_steps_height) {
                // Try to step up.
                for (int y_step = 1; y_step <= floor; y_step++) {
                    for (int x = left; x < right; x++) {
                        if (is_blocking(x, top - y_step)) {
                            blocked = true;
                            break;
                        }
                    }
                    if (blocked) {
                        break;
                    }
                }

                if (!blocked) {
                    top -= floor;
                    bot -= floor;
                    float pre_step_y = new_position.y;
                    new_position.y = std::floor(new_position.y - float(floor)) - 0.02f;

                    step_offset.y -= new_position.y - pre_step_y;

                    velocity.y = std::min(-0.0f, velocity.y);
                }
            } else {
                blocked = true;
            }

            break;
        }
    }

    return blocked;
}

bool block_or_step_right(
    int &left,
    int &right,
    int &top,
    int &bot,
    Vector2 &new_position,
    Vector2 &velocity,
    Vector2 &step_offset,
    int max_steps_height
) {
    bool blocked = false;

    int floor = (bot - top) + 1;
    for (int y = top; y <= bot; y++) {
        floor--;

        if (is_blocking(right, y)) {
            if (floor <= max_steps_height) {
                // Try to step up.
                for (int y_step = 1; y_step <= floor; y_step++) {
                    for (int x = left + 1; x <= right; x++) {
                        if (is_blocking(x, top - y_step)) {
                            blocked = true;
                            break;
                        }
                    }
                    if (blocked) {
                        break;
                    }
                }

                if (!blocked) {
                    top -= floor;
                    bot -= floor;
                    float pre_step_y = new_position.y;
                    new_position.y = std::floor(new_position.y - float(floor)) - 0.02f;

                    step_offset.y -= new_position.y - pre_step_y;

                    velocity.y = std::min(-0.0f, velocity.y);
                }
            } else {
                blocked = true;
            }

            break;
        }
    }

    return blocked;
}

void GridCharacterBody::_bind_methods() {
	ClassDB::bind_method(
        D_METHOD("set_size", "value"),
        &GridCharacterBody::set_size
    );
    ClassDB::bind_method(
        D_METHOD("get_size"),
        &GridCharacterBody::get_size
    );
	ADD_PROPERTY(
        PropertyInfo(Variant::VECTOR2,
        "size"),
        "set_size",
        "get_size"
    );

    ClassDB::bind_method(
        D_METHOD("set_velocity", "value"),
        &GridCharacterBody::set_velocity
    );
    ClassDB::bind_method(
        D_METHOD("get_velocity"),
        &GridCharacterBody::get_velocity
    );
	ADD_PROPERTY(
        PropertyInfo(Variant::VECTOR2,
        "velocity"),
        "set_velocity",
        "get_velocity"
    );

    ClassDB::bind_method(
        D_METHOD("set_max_steps_height", "value"),
        &GridCharacterBody::set_max_steps_height
    );
    ClassDB::bind_method(
        D_METHOD("get_max_steps_height"),
        &GridCharacterBody::get_max_steps_height
    );
    ADD_PROPERTY(
        PropertyInfo(Variant::INT,
        "max_steps_height"),
        "set_max_steps_height",
        "get_max_steps_height"
    );

    ClassDB::bind_method(
        D_METHOD("get_collision_rect"),
        &GridCharacterBody::get_collision_rect
    );

    ClassDB::bind_method(
        D_METHOD("move"),
        &GridCharacterBody::move
    );
}

void GridCharacterBody::set_size(Vector2 value) {
    size = value.abs();

    if (Engine::get_singleton()->is_editor_hint()) {
        queue_redraw();
    }
}

Vector2 GridCharacterBody::get_size() const {
    return size;
}

void GridCharacterBody::set_velocity(Vector2 value) {
    velocity = value;
}

Vector2 GridCharacterBody::get_velocity() const {
    return velocity;
}

void GridCharacterBody::set_max_steps_height(int value) {
    max_steps_height = value;
}

int GridCharacterBody::get_max_steps_height() const {
    return max_steps_height;
}

Rect2 GridCharacterBody::get_collision_rect() const {
    auto position = get_position();
    return Rect2(
        position.x - size.x * 0.5f,
        position.y - size.y * 0.5f,
        size.x,
        size.y
    );
}

bool GridCharacterBody::move() {
    if (Grid::cells == nullptr) {
        return false;
    }

    auto previous_position = get_position() - step_offset;
    auto wish_position = previous_position + velocity;

    step_offset *= 0.8f;

    if (!collision) {
        set_position(wish_position + step_offset);
        return false;
    }

    auto new_position = previous_position;

    int top = previous_position.y - size.y * 0.5f;
    int bot = previous_position.y + size.y * 0.5f;
    int left = previous_position.x - size.x * 0.5f;
    int right = previous_position.x + size.x * 0.5f;

    bool hit_left_wall = false;
    bool hit_right_wall = false;

    int horizontal_steps = 0;

    // Move left
    if (velocity.x < -0.01f) {
        // Move left until we hit a wall.
        while (new_position.x > wish_position.x) {
            if (block_or_step_left(
                left,
                right,
                top,
                bot,
                new_position,
                velocity,
                step_offset,
                max_steps_height
            )) {
                hit_left_wall = true;

                if (horizontal_steps > 0) {
                    new_position.x = std::floor(new_position.x + 1.0f) + 0.02f;
                }
                
                velocity.x = -0.0f;
                break;
            } else {
                left -= 1;
                right -= 1;
                new_position.x -= 1.0f;
                
                horizontal_steps++;
            }
        }
        
        if (!hit_left_wall) {
            if (block_or_step_left(
                left,
                right,
                top,
                bot,
                new_position,
                velocity,
                step_offset,
                max_steps_height
            )) {
                hit_left_wall = true;
                new_position.x = std::floor(new_position.x + 1.0f) + 0.02f;
                velocity.x = -0.0f;
            }
        }

        new_position.x = std::max(new_position.x, wish_position.x);
    } else if (velocity.x > 0.01f) {
        // Move right until we hit a wall.
        while (new_position.x < wish_position.x) {
            if (block_or_step_right(
                left,
                right,
                top,
                bot,
                new_position,
                velocity,
                step_offset,
                max_steps_height
            )) {
                hit_right_wall = true;

                if (horizontal_steps > 0) {
                    // TODO: is this right?
                    new_position.x = std::floor(new_position.x) - 0.02f;
                }
                
                velocity.x = 0.0f;
                break;
            } else {
                left += 1;
                right += 1;
                new_position.x += 1.0f;
                
                horizontal_steps++;
            }
        }
        
        if (!hit_right_wall) {
            if (block_or_step_right(
                left,
                right,
                top,
                bot,
                new_position,
                velocity,
                step_offset,
                max_steps_height
            )) {
                hit_right_wall = true;
                new_position.x = std::floor(new_position.x) - 0.02f;
                velocity.x = 0.0f;
            }
        }

        new_position.x = std::min(new_position.x, wish_position.x);
    }

    new_position.y += velocity.y;

    set_position(new_position + step_offset);

    return hit_left_wall;
}

void GridCharacterBody::_draw() {
    if (Engine::get_singleton()->is_editor_hint()) {
        draw_rect(
            Rect2(size * -0.5f, size),
            Color(1.0f, 0.0f, 0.0f, 0.5f),
            false
        );
    }
}