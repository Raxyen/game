#include <thread>
#include <windows.h>

#include "SoundSystem.h"

using namespace std;

SoundSystem::SoundSystem() = default;

void SoundSystem::playMenuSound() {
    thread([] { Beep(500, 50); }).detach();
}

void SoundSystem::playEnemyHitSound() {
    thread([] { Beep(300, 50); }).detach();
}

void SoundSystem::playerHitSound() {
    thread([] {
        Beep(200, 100);
        Beep(150, 100);
        }).detach();
}

void SoundSystem::playEnemyKilledSound() {
    thread([] {
        Beep(300, 50);
        Beep(300, 50);
        Beep(400, 50);
        }).detach();
}

void SoundSystem::playBulletSound() {
    thread([] { Beep(1000, 30); }).detach();
}

void SoundSystem::playRocketSound() {
    thread([] { Beep(200, 30); }).detach();
}

void SoundSystem::playPowerUpPickedSound() {
    thread([] {
        Beep(400, 50);
        Beep(500, 50);
        Beep(400, 50);
        Beep(500, 50);
        }).detach();
}
