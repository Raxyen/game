#include "PowerUp.h"

PowerUp::PowerUp(Player& player, int p_type) : type(p_type) {
    switch (type) {
        case 0:
            texture = L'♥';
            break;
        case 1:
            texture = L'⌻';
            break;
    }

    unsigned int x, y;
    do {
        x = rand() % (MAP_WIDTH - 2) + 1;
        y = rand() % (MAP_HEIGHT - 2) + 1;
    } while (x == static_cast<int>(player.getPosition().getX()) && y == static_cast<int>(player.getPosition().getY()));
    pos = Position(static_cast<float>(x), static_cast<float>(y));
}

Position PowerUp::getPosition() { return pos; }
wchar_t PowerUp::getTexture() { return texture; }
int PowerUp::getType() { return type; }