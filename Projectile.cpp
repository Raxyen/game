#include "Projectile.h"

Projectile::Projectile() = default;
Projectile::Projectile(Position p_pos, unsigned int p_damage, float p_speed, bool p_is_enemy, char p_direction)
    : pos(p_pos), prevPos(p_pos), damage(p_damage), speed(p_speed), is_enemy(p_is_enemy), direction(p_direction), is_active(true) {}

Position Projectile::getPosition() { return pos; }
Position Projectile::getPrevPosition() { return prevPos; }
void Projectile::setPosition(Position pos) { this->pos = pos; }

char Projectile::getDirection() { return direction; }

void Projectile::activate() { is_active = true; }
void Projectile::deactivate() { is_active = false; }

bool Projectile::isActive() { return is_active; }
bool Projectile::isEnemy() { return is_enemy; }

unsigned int Projectile::getDamage() { return this->damage; }

void Projectile::move() { // moves projectile
    if (this->is_active) {
        // store previous position to allow segment-based collision checks (fixes tunneling)
        prevPos = pos;
        if (direction == 'w' && pos.getY() > 0) pos.moveY(-speed);
        else if (direction == 'a' && pos.getX() > 0) pos.moveX(-speed);
        else if (direction == 's' && pos.getY() < MAP_HEIGHT - 1) pos.moveY(speed);
        else if (direction == 'd' && pos.getX() < MAP_WIDTH - 1) pos.moveX(speed);
        else deactivate();
    }
}