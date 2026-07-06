#pragma once

constexpr int MAP_HEIGHT = 15;
constexpr int MAP_WIDTH = 30;

class Position {
private:
    float x, y;
public:
    Position();
    Position(float p_x, float p_y);

    float getX();
    float getY();
    void moveX(float dx);
    void moveY(float dy);

    Position& operator=(const Position& other);
};