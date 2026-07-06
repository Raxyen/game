#pragma once

#include "Position.h"
#include "Bullet.h"
#include "Player.h"

class Enemy {
private:
    Position pos;
    Bullet bullet;
    float speed;
    char texture;
    bool is_hit;
    char direction;
    int hp;
public:
    Enemy(char p_direction, Player& player);

    Position getPosition();
    Bullet& getBullet();
    const Bullet& getBullet() const;
    void setBullet(Bullet bullet);
    float getSpeed();
    int getHP();
    char getTexture();

    bool isHit();
    void setIsHit(bool is_hit);

    void modifyHealth(int health);
    void move(float dx, float dy);
    void shoot(Player& player);
};