#pragma once

#include "Projectile.h"
#include "Position.h"

class Rocket : public Projectile {
public:
    Rocket(Position p_pos, unsigned int p_damage, bool p_is_enemy, char p_direction);
};