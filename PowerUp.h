#pragma once

#include <cstdlib>

#include "Position.h"
#include "Player.h"

class PowerUp {
private:
    Position pos;
    int type;
    char texture;
public:
    PowerUp(Player& player, int p_type);

    Position getPosition();
    char getTexture();
    int getType();
};