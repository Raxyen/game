#include "Enemy.h"

Enemy::Enemy(char p_direction, Player& player) :
    is_hit(false), direction(p_direction), hp(100), speed(0.2f), texture(L'⬢') {
    unsigned int x = rand() % (MAP_WIDTH - 2) + 1;
    unsigned int y = rand() % (MAP_HEIGHT - 2) + 1;
    do {
        x = rand() % (MAP_WIDTH - 2) + 1;
        y = rand() % (MAP_HEIGHT - 2) + 1;
    } while (x == static_cast<int>(player.getPosition().getX()) && y == static_cast<int>(player.getPosition().getY()));
    pos = Position(static_cast<float>(x), static_cast<float>(y));
    bullet = Bullet(getPosition(), 20, true, ' ');
    bullet.deactivate();
}

Position Enemy::getPosition() {
    return pos;
}

Bullet& Enemy::getBullet() {
    return bullet;
}

const Bullet& Enemy::getBullet() const {
    return bullet;
}

void Enemy::setBullet(Bullet bullet) {
    this->bullet = bullet;
}

float Enemy::getSpeed() {
    return speed;
}

int Enemy::getHP() {
    return hp;
}

wchar_t Enemy::getTexture() {
    return texture;
}

bool Enemy::isHit() {
    return is_hit;
}

void Enemy::setIsHit(bool is_hit) {
    this->is_hit = is_hit;
}

void Enemy::modifyHealth(int health) {
    this->hp += health;
}

void Enemy::move(float dx, float dy) {
    pos.moveX(dx);
    pos.moveY(dy);
}

void Enemy::shoot(Player& player) {
    float dx = player.getPosition().getX() - getPosition().getX();
    float dy = player.getPosition().getY() - getPosition().getY();

    char temp_dir;
    if (fabs(dx) > fabs(dy))
        temp_dir = (dx > 0) ? 'd' : 'a'; // horizontally
    else
        temp_dir = (dy > 0) ? 's' : 'w'; // vertically

    bullet = Bullet(getPosition(), 20, true, temp_dir);
    bullet.activate();
}