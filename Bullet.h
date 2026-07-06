#pragma once
#include "Projectile.h"

class Bullet : public Projectile {
public:
    Bullet();
    Bullet(Position p_pos, unsigned int p_damage, bool p_is_enemy, char p_direction);
};