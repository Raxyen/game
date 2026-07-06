#pragma once
#include <vector>
#include <string>

#include "Position.h"
#include "Bullet.h"

class Player {
private:
    Position pos;
    std::vector<Bullet> bullets;
    std::string name;

    char direction;
    int hp;

    unsigned int kills;
    unsigned int score;
    unsigned int bullets_count;
    unsigned int rockets_count;
    unsigned int bullets_fired;
    unsigned int rockets_fired;

    int cause_of_death;
public:
    Player();

    Position getPosition();
    void setPosition(Position pos);

    std::vector<Bullet>& getBullets();
    const std::vector<Bullet>& getBullets() const;

    char getDirection();
    void setDirection(char direction);

    int getHP();

    unsigned int getScore() const;

    unsigned int getBulletsCount() const;
    unsigned int getRocketsCount() const;

    void setCauseOfDeath(int cause);

    void setName(std::string name);
    std::string getName();

    void modifyHealth(int health);
    void modifyScore(int score);
    void modifyBulletsCount(int bullets_count);
    void modifyRocketsCount(int rockets_count);
    void modifyRocketsFired(int rockets_fired);
    void kill();
    void normalizeHP();
    void move(float dx, float dy);
    void shoot();
    void reset();
    void info();
    void finalInfo();
};