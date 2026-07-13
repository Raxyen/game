#include <windows.h>
#include <mmsystem.h> // Dodajemy nagłówek multimedialny

// Informujemy Visual Studio, żeby podlinkowało systemową bibliotekę dźwięku WinMM
#pragma comment(lib, "winmm.lib")

#include "SoundSystem.h"

SoundSystem::SoundSystem() = default;

void SoundSystem::playMenuSound() {
    // SND_ALIAS używa dźwięków systemowych, SND_ASYNC odtwarza w tle bez blokowania gry i BEZ WĄTKÓW!
    PlaySoundW(L"SystemAsterisk", NULL, SND_ALIAS | SND_ASYNC);
}

void SoundSystem::playEnemyHitSound() {
    PlaySoundW(L"SystemDefault", NULL, SND_ALIAS | SND_ASYNC);
}

void SoundSystem::playerHitSound() {
    PlaySoundW(L"SystemHand", NULL, SND_ALIAS | SND_ASYNC);
}

void SoundSystem::playEnemyKilledSound() {
    PlaySoundW(L"SystemQuestion", NULL, SND_ALIAS | SND_ASYNC);
}

void SoundSystem::playBulletSound() {
    // Prosty, krótki sygnał dźwiękowy z głośniczka (odpowiednik Beep, ale nie blokuje tak wątku głównego)
    MessageBeep(MB_OK);
}

void SoundSystem::playRocketSound() {
    MessageBeep(MB_ICONWARNING);
}

void SoundSystem::playPowerUpPickedSound() {
    PlaySoundW(L"SystemExclamation", NULL, SND_ALIAS | SND_ASYNC);
}