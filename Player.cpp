#include <iostream>

#include "Player.h"

using namespace std;

Player::Player() { reset(); }

Position Player::getPosition() { return pos; }
void Player::setPosition(Position pos) { this->pos = pos; }

std::vector<Bullet>& Player::getBullets() { return bullets; }
const vector<Bullet>& Player::getBullets() const { return bullets; }

char Player::getDirection() { return direction; }
void Player::setDirection(char direction) { this->direction = direction; }

int Player::getHP() { return hp; }

unsigned int Player::getScore() const { return score; }

unsigned int Player::getBulletsCount() const { return bullets_count; }
unsigned int Player::getRocketsCount() const { return rockets_count; }

void Player::setCauseOfDeath(int cause) { this->cause_of_death = cause; }

void Player::setName(string name) { this->name = name; }
string Player::getName() { return name; }

void Player::modifyHealth(int health) { this->hp += health; }
void Player::modifyScore(int score) { this->score += score; }

void Player::modifyBulletsCount(int bullets_count) { this->bullets_count += bullets_count; }
void Player::modifyRocketsCount(int rockets_count) { this->rockets_count += rockets_count; }

void Player::modifyRocketsFired(int rockets_fired) { this->rockets_fired += rockets_fired; }

void Player::kill() { kills++; }

void Player::normalizeHP() { if (hp > 100) hp = 100; }

void Player::move(float dx, float dy) {
    pos.moveX(dx);
    pos.moveY(dy);
}

void Player::shoot() {
    Bullet bullet(getPosition(), 30, false, getDirection());
    bullets.emplace_back(bullet);
    bullets_count--;
    bullets_fired++;
}

void Player::reset() {
    direction = 'w';
    kills = 0;
    score = 0;
    hp = 100;
    bullets_count = 30;
    rockets_count = 2;
    bullets_fired = 0;
    rockets_fired = 0;
    cause_of_death = 0;
    setPosition(Position(10.0f, 5.0f));
}

void Player::info() {
    wcout << "Enemy kills: " << kills << endl;
    wcout << "Bullets fired: " << bullets_fired << endl;
    wcout << "Bullets: " << bullets_count << endl;
    wcout << "Missiles fired: " << rockets_fired << endl;
    wcout << "Missiles: " << rockets_count << endl;
    wcout << "Score: " << score << endl;
    wcout << "Health: ";
}

void Player::finalInfo() {
    wcout << "  * Score: " << score << "\n  * Enemy kills: " << kills << L"\n  * Killed by: ";
    cause_of_death == 1 ? wcout << L"Enemy crashing you\n\n" : wcout << L"Enemy bullet\n\n";
    wcout << L"   Press any key to proceed to the menu.\n";
}