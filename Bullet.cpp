#include "Bullet.h"

Bullet::Bullet() = default;

Bullet::Bullet(Position p_pos, unsigned int p_damage, bool p_is_enemy, char p_direction)
    : Projectile(p_pos, p_damage, 1.0f, p_is_enemy, p_direction, 'b') {
}