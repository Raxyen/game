#include "Rocket.h"

Rocket::Rocket() = default;

Rocket::Rocket(Position p_pos, unsigned int p_damage, bool p_is_enemy, char p_direction)
    : Projectile(p_pos, p_damage, 0.7f, p_is_enemy, p_direction, 'r') {
}