#pragma once
#include "Position.h"

class Projectile {
protected:
    Position pos;
    Position prevPos;
    unsigned int damage;
    float speed;
    bool is_enemy;
    bool is_active;
    char direction;
    char type;
public:
    Projectile();
    Projectile(Position p_pos, unsigned int p_damage, float p_speed, bool p_is_enemy, char p_direction, char type);

    Position getPosition();
    Position getPrevPosition();
    void setPosition(Position pos);

    char getDirection();

    void activate();
    void deactivate();

    bool isActive();
    bool isEnemy();

    unsigned int getDamage();

    char getType();

    void move();
};