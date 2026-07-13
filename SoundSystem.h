#pragma once

class SoundSystem {
public:
    SoundSystem();

    static void playMenuSound();
    static void playEnemyHitSound();
    static void playerHitSound();
    static void playEnemyKilledSound();
    static void playBulletSound();
    static void playRocketSound();
    static void playPowerUpPickedSound();
};