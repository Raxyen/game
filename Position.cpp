#include "Position.h"

// constuctors
Position::Position() : x(0), y(0) {} // default
Position::Position(float p_x, float p_y) : x(p_x), y(p_y) {}

// getters
float Position::getX() { return x; }
float Position::getY() { return y; }

// methods
void Position::moveX(float dx) { this->x += dx; }
void Position::moveY(float dy) { this->y += dy; }

// operators
Position& Position::operator=(const Position& other) {
    if (this != &other) {
        this->x = other.x;
        this->y = other.y;
    }
    return *this;
}