#pragma once

#include <string>

class Data {
public:
    static inline bool display_no_bullets;
    static inline bool display_no_rockets;
    static inline bool display_enemy_killed;
    static inline bool powerup_and_player_collision;
    static inline bool powerup_spawn_condition;
    static inline std::wstring message1 = L"\nEnemy eliminated!";
    static inline std::wstring message2 = L"\nPowerup collected!";
    static inline std::wstring message3 = L"\nYou are hurt. Hide.";
    static inline std::wstring message4 = L"\nYou're out of bullets!";
    static inline std::wstring message5 = L"\nYou're out of missiles!";
};