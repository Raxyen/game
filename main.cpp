#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <algorithm>
#include <thread>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <string>
#include <sstream>

#include "Position.h"
#include "Projectile.h"
#include "Player.h"
#include "Enemy.h"
#include "FileManager.h"
#include "PowerUp.h"
#include "SoundSystem.h"
#include "Data.h"
#include "Renderer.h"

using namespace std;

// system functions

void setupConsole() { // wide characters
    if (_setmode(_fileno(stdout), _O_U16TEXT) == -1) {
        wcerr << L"Error setting console mode\n";
    }

    // WŁĄCZANIE OBSŁUGI KODÓW ANSI W KONSOLI WINDOWS:
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }

    // Dodatkowo odpinamy powolną synchronizację strumieni z C
    std::ios_base::sync_with_stdio(false);
    std::wcout.tie(NULL);
}

void setConsoleColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void menu() {
    system("cls");
    wcout << L"╔═══════════════════════╗\n";
    wcout << L"║     * MAIN MENU *     ║\n";
    wcout << L"╠═══╦═══════════════════╣\n";
    wcout << L"║ 1 ║  Play!            ║\n";
    wcout << L"║ 2 ║  Controls         ║\n";
    wcout << L"║ 3 ║  Tutorial         ║\n";
    wcout << L"║ 4 ║  Scorelist        ║\n";
    wcout << L"║ 5 ║  Credits          ║\n";
    wcout << L"║ 6 ║  Settings         ║\n";
    wcout << L"║ 7 ║  Quit game        ║\n";
    wcout << L"╚═══╩═══════════════════╝\n";
}

// general game functions

void resetPlayerStats(Player& player, vector<Enemy>& enemies) {
    for (auto& bullet : player.getProjectiles()) {
        bullet.deactivate();
    }
    for (auto& enemy : enemies) {
        enemy.getBullet().deactivate();
    }
}

string gameOver(Player& player, vector<Enemy>& enemies, vector<PowerUp>& powerups) {
    system("cls");
    setConsoleColor(12);

    wcout << "\n\n\n";
    wcout << L"   ╔═════════════════════════════╗\n";
    wcout << L"   ║                             ║░\n";
    wcout << L"   ║          GAME OVER          ║░\n";
    wcout << L"   ║                             ║░\n";
    wcout << L"   ╚═════════════════════════════╝░\n";
    wcout << L"    ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n\n";

    player.finalInfo();
    enemies.clear();
    player.getProjectiles().clear();
    powerups.clear();
    Sleep(3000);
    resetPlayerStats(player, enemies);
    _getch();
    setConsoleColor(7);
    system("cls");
    string name;
    wcout << "Name: ";
    getline(cin, name);
    return name;
}

// game objects manipulation 

void spawnEnemies(Player& player, vector<Enemy>& enemies, unsigned int count) {
    array<char, 4> dir = { 'w', 'a', 's', 'd' };
    for (size_t i = 0; i < count; i++)
        enemies.emplace_back(Enemy(dir[rand() % 4], player));
}

void moveEnemies(Player& player, vector<Enemy>& enemies) {
	array<array<bool, MAP_WIDTH>, MAP_HEIGHT> occupied = { false };

    for (auto& e : enemies)
        occupied[static_cast<int>(e.getPosition().getY())][static_cast<int>(e.getPosition().getX())] = true;

    // iterate every enemy
    for (auto& e : enemies) {

        int oldX = static_cast<int>(e.getPosition().getX());
        int oldY = static_cast<int>(e.getPosition().getY());

        // determine enemy direction
        int dx = 0, dy = 0;

        if (fabs(player.getPosition().getX() - e.getPosition().getX()) > fabs(player.getPosition().getY() - e.getPosition().getY()))
            dx = (player.getPosition().getX() > e.getPosition().getX()) ? 1 : -1;
        else
            dy = (player.getPosition().getY() > e.getPosition().getY()) ? 1 : -1;

        int newX = oldX + dx;
        int newY = oldY + dy;

        // check if other enemy doesn't occupy the same position
        if (newX >= 0 && newX < MAP_WIDTH && newY >= 0 && newY < MAP_HEIGHT && !occupied[newY][newX]) {
            // updating if position is occupied 
            occupied[oldY][oldX] = false;
            occupied[newY][newX] = true;

            // move enemy
            e.move(dx * e.getSpeed(), dy * e.getSpeed());
        }
    }
}

// game objects collisions

unsigned int checkPlayerProjectileAndEnemyCollision(Player& player, Enemy& enemy) {
    for (Projectile& projectile : player.getProjectiles()) {
        if (!projectile.isActive() || projectile.isEnemy())
            continue;

        // casting float coordinates to ints
        int x1 = static_cast<int>(projectile.getPrevPosition().getX());
        int y1 = static_cast<int>(projectile.getPrevPosition().getY());
        int x2 = static_cast<int>(projectile.getPosition().getX());
        int y2 = static_cast<int>(projectile.getPosition().getY());

        int ex = static_cast<int>(enemy.getPosition().getX());
        int ey = static_cast<int>(enemy.getPosition().getY());

        // rectangle of tiles bullet flew through in this frame/tick
        int minX = (x1 < x2) ? x1 : x2;
        int maxX = (x1 > x2) ? x1 : x2;
        int minY = (y1 < y2) ? y1 : y2;
        int maxY = (y1 > y2) ? y1 : y2;

        // check collision
        if (ex >= minX && ex <= maxX && ey >= minY && ey <= maxY) {
            projectile.deactivate();
            return projectile.getDamage();
        }
    }
    return 0;
}

bool checkEnemyBulletAndPlayerCollision(Player& player, Enemy& enemy) { // checks if both the coordinates of enemy's bullet and player are equal (checks collision)
    return (enemy.getBullet().isActive() && enemy.getBullet().isEnemy() && static_cast<int>(player.getPosition().getX()) == static_cast<int>(enemy.getBullet().getPosition().getX())) && static_cast<int>(player.getPosition().getY()) == static_cast<int>(enemy.getBullet().getPosition().getY());
}

bool checkPlayerAndEnemyCollision(Player& player, Enemy& enemy) { // checks if both the coordinates of enemy and player are equal (checks collision)
    return (static_cast<int>(enemy.getPosition().getX()) == static_cast<int>(player.getPosition().getX()) && static_cast<int>(enemy.getPosition().getY()) == static_cast<int>(player.getPosition().getY()));
}

bool checkPlayerAndPowerUpCollision(Player& player, PowerUp& power_up) { // checks if both the coordinates of powerup and player are equal (checks collision)
    return (static_cast<int>(power_up.getPosition().getX()) == static_cast<int>(player.getPosition().getX()) && static_cast<int>(power_up.getPosition().getY()) == static_cast<int>(player.getPosition().getY()));
}

// checks whether the player distance from enemy is least or equal than 14 
// and if the player and enemy are in the same line to make a shot possible

bool playerNearEnemy(Player& player, Enemy& enemy) {
    if (static_cast<int>(player.getPosition().getX()) == static_cast<int>(enemy.getPosition().getX()) && fabs(player.getPosition().getY() - enemy.getPosition().getY()) <= 14) {
        return true; // same column
    }
    else if (static_cast<int>(player.getPosition().getY()) == static_cast<int>(enemy.getPosition().getY()) && fabs(player.getPosition().getX() - enemy.getPosition().getX()) <= 14) {
        return true; // same row
    }
    return false;
}

// drawing map
void drawUpperBorder() {
    Renderer::drawChar(0, 0, 7, L'╔'); // left upper corner of the frame
	for (int i = 1; i <= MAP_WIDTH; i++) {
		Renderer::drawChar(i, 0, 7, L'═');
	}
	Renderer::drawChar(MAP_WIDTH + 1, 0, 7, L'╗'); // right upper corner of the frame
}

void drawLowerBorder() {
    Renderer::drawChar(0, MAP_HEIGHT + 1, 7, L'╚'); // left lower corner of the frame
    for (int i = 1; i <= MAP_WIDTH; i++) {
        Renderer::drawChar(i, MAP_HEIGHT + 1, 7, L'═');
    }
    Renderer::drawChar(MAP_WIDTH + 1, MAP_HEIGHT + 1, 7, L'╝'); // right lower corner of the frame
}

void drawLeftBorder() {
    for (int i = 1; i <= MAP_HEIGHT; i++) {
        Renderer::drawChar(0, i, 7, L'║');
    }
}

void drawRightBorder() {
    for (int i = 1; i <= MAP_HEIGHT; i++) {
        Renderer::drawChar(MAP_WIDTH + 1, i, 7, L'║');
    }
}

void drawPlayer(Player& player) {
    // draw player
    int x = static_cast<int>(player.getPosition().getX()) + 1;
    int y = static_cast<int>(player.getPosition().getY()) + 1;

    Renderer::drawChar(x, y, 2, player.getTexture());
}

void drawPlayerProjectiles(Player& player) {
    // draw player's projectiles (bullets and rockets) 
    for (auto& projectile : player.getProjectiles()) {
        int x = static_cast<int>(projectile.getPosition().getX()) + 1;
        int y = static_cast<int>(projectile.getPosition().getY()) + 1;

        if (projectile.isActive()) {          

            if (projectile.getType() == 'b') {
                Renderer::drawChar(x, y, 8, L'·');
            }
            else if (projectile.getType() == 'r') {
                if (projectile.getDirection() == 'w') {
                    Renderer::drawChar(x, y, 8, L'▲'); // rocket with up direction
                }
                else if (projectile.getDirection() == 'a') {
                    Renderer::drawChar(x, y, 8, L'◄'); // rocket with left direction
                }
                else if (projectile.getDirection() == 's') {
                    Renderer::drawChar(x, y, 8, L'▼'); // rocket with down direction
                }
                else if (projectile.getDirection() == 'd') {
                    Renderer::drawChar(x, y, 8, L'►'); // rocket with right direction  
                }
            }
        }
    }
}

void drawEnemies(vector<Enemy>& enemies) {
    // draw enemies
    for (auto& enemy : enemies) {
        int x = static_cast<int>(enemy.getPosition().getX()) + 1;
        int y = static_cast<int>(enemy.getPosition().getY()) + 1;
        int color = enemy.isHit() ? 13 : (enemy.getHP() > 30 ? 4 : 6);

        Renderer::drawChar(x, y, color, enemy.getTexture());
    }
}

void drawEnemyProjectiles(vector<Enemy>& enemies) {
    // draw enemy bullets for each enemy
    for (auto& enemy : enemies) {
        if (enemy.getBullet().isActive()) { // draw enemy's bullet
            int x = static_cast<int>(enemy.getBullet().getPosition().getX()) + 1;
            int y = static_cast<int>(enemy.getBullet().getPosition().getY()) + 1;
            
            Renderer::drawChar(x, y, 12, L'·');
        }
    }
}

void drawPowerUps(vector<PowerUp>& powerups) {
    // draw powerups
    for (auto& powerup : powerups) {
        int x = static_cast<int>(powerup.getPosition().getX()) + 1;
        int y = static_cast<int>(powerup.getPosition().getY()) + 1;
        
        Renderer::drawChar(x, y, 9, powerup.getTexture());
    }
}

void drawMessage(bool& condition, int color, const std::wstring& message, unsigned int& counter) {
    if (condition) {
		counter = 1;         // Start frame counting for message display
        condition = false;   // Disable the global flag so the counting happens only once
    }

    if (counter > 0 && counter < 10) {
        // Message blinking: draw message only in odd frames (1, 3, 5, 7, 9)
        // In even frames we do nothing – Renderer::clear() will take care of "disappearing" the text!
        if (counter % 2 != 0) {
            Renderer::drawString(34, 12, color, message);
        }
        counter++;
    }
    else {
		counter = 0; // message display finished, reset counter
    }
}

void drawMap(array<array<char, MAP_HEIGHT>, MAP_WIDTH>& map, Player& player, vector<Enemy>& enemies, vector<PowerUp>& powerups) {
    Renderer::clear();

    drawUpperBorder();
    drawLowerBorder();
    drawLeftBorder();
    drawRightBorder();

    // priotity:
    // first display dynamic entities: the player, then their projectiles, then enemies projectiles, then enemies
    // then display static entities like powerups

    drawPlayer(player);
    drawPlayerProjectiles(player);
    drawEnemyProjectiles(enemies);
    drawEnemies(enemies);
    drawPowerUps(powerups);

    Renderer::drawString(34, 0, 6, L"═══════════ PLAYER STATUS ═══════════");
    std::wstringstream ssScore;
    ssScore << L"Score:          " << std::setw(6) << std::setfill(L'0') << player.getScore();
    Renderer::drawString(34, 1, 7, ssScore.str());
    Renderer::drawString(34, 2, 11, L"Enemy kills:");
    Renderer::drawString(50, 2, 8, std::to_wstring(player.getKills()));
    Renderer::drawString(34, 4, 11, L"Health: ");
    Renderer::drawString(34, 6, 6, L"═══════════ AMMUNITION ══════════════");
    Renderer::drawString(34, 7, 11, L"Bullets:");
    Renderer::drawString(50, 7, 8, std::to_wstring(player.getBulletsCount()));
    Renderer::drawString(34, 8, 11, L"Missiles:");
    Renderer::drawString(50, 8, 8, std::to_wstring(player.getRocketsCount()));
    Renderer::drawString(34, 10, 6, L"═══════════ INFO ════════════════");


    // draw hp bar
    Renderer::drawChar(50, 4, 8, L'[');
    for (int i = 0; i < 10; i++) {
        if (i <= static_cast<int>(player.getHP() / 10))
            Renderer::drawChar(51 + i, 4, 12, L'█');
        else {
            Renderer::drawChar(51 + i, 4, 12, L'░');
        }
    }
    Renderer::drawChar(61, 4, 8, L']');

    static unsigned int killed_counter = 0;
    static unsigned int bullets_counter = 0;
    static unsigned int rockets_counter = 0;
	static unsigned int powerup_counter = 0;
	static unsigned int hurt_counter = 0;

    bool is_hurt = player.getHP() <= 30;

    // display messages
    drawMessage(Data::display_enemy_killed, 3, Data::message1, killed_counter);
    drawMessage(Data::powerup_and_player_collision, 3, Data::message2, powerup_counter);
    drawMessage(is_hurt, 12, Data::message3, hurt_counter);
    drawMessage(Data::display_no_bullets, 3, Data::message4, bullets_counter);
    drawMessage(Data::display_no_rockets, 3, Data::message5, rockets_counter);

    Renderer::render();
}

int main() {
    srand(time(NULL));

    array<array<char, MAP_HEIGHT>, MAP_WIDTH> map;
    Player player;
    vector<Enemy> enemies;
    vector<PowerUp> powerups;
    FileManager file_manager;

    // scoring
    vector <pair<wstring, unsigned int>> scorelist;
    unsigned short score_number = 1;

    char key;
    bool sound_on = true;

    setupConsole();
    char menu_option;
    while (true) {
        menu();
        menu_option = _getch();
        switch (menu_option) {
        case '1':
            // game
            if (sound_on)
                SoundSystem::playMenuSound();

            for (auto& row : map)
                row.fill(' ');

            player.reset();

            Data::display_no_bullets = false;
            Data::display_no_rockets = false;
            Data::display_enemy_killed = false;
            Data::powerup_and_player_collision = false;
            Data::powerup_spawn_condition = false;

            spawnEnemies(player, enemies, 4);

            // game loop
            while (true) {
                if (_kbhit()) { // reading pressed keys
                    key = _getch();
                    if ((key == 'w' || key == 'W') && player.getPosition().getY() > 0) { // up
                        player.move(0, -1);
                        player.setDirection('w');
                    }
                    if ((key == 'a' || key == 'A') && player.getPosition().getX() > 0) { // left
                        player.move(-1, 0);
                        player.setDirection('a');
                    }
                    if ((key == 's' || key == 'S') && player.getPosition().getY() < MAP_HEIGHT - 1) { // down
                        player.move(0, 1);
                        player.setDirection('s');
                    }
                    if ((key == 'd' || key == 'D') && player.getPosition().getX() < MAP_WIDTH - 1) { // right
                        player.move(1, 0);
                        player.setDirection('d');
                    }
                    if ((key == 'f' || key == 'F')) {
                        if (player.getBulletsCount() == 0) {
                            Data::display_no_bullets = true;
                        }
                        else {
                            player.shoot('b'); // shoot a bullet
                            if (sound_on)
                                SoundSystem::playBulletSound();
                        }
                    }
                    if ((key == 'r' || key == 'R')) {
                        if (player.getRocketsCount() == 0) {
                            Data::display_no_rockets = true;
                        }
                        else {
                            player.shoot('r'); // shoot a rocket
                            if (sound_on)
                                SoundSystem::playRocketSound();
                        }
                    }
                    // test only
                    if (key == 't' || key == 'T')
                        spawnEnemies(player, enemies, 4);
                }

                // move projectiles
                for (auto& projectile : player.getProjectiles()) {
                    projectile.move();
                }

                // remove inactive projectiles
                player.getProjectiles().erase(
                    remove_if(player.getProjectiles().begin(), player.getProjectiles().end(),
                        [](Projectile& p) { return !p.isActive(); }),
                    player.getProjectiles().end());

                // iterate enemies vector
                for (auto& enemy : enemies) {
                    unsigned dmg = checkPlayerProjectileAndEnemyCollision(player, enemy);
                    if (dmg) {
                        if (sound_on)
                            SoundSystem::playEnemyHitSound();
                        enemy.setIsHit(true);
                        enemy.modifyHealth(-static_cast<int>(dmg));
                        player.modifyScore(10);
                        if (enemy.getHP() <= 0)
                            Data::powerup_spawn_condition = true;
                    }
                    if (enemy.getHP() <= 0) {
                        if (sound_on)
                            SoundSystem::playEnemyKilledSound();
                        enemy.getBullet().deactivate();
                        player.modifyScore(50);
                        player.kill();
                        Data::display_enemy_killed = true;
                        Data::powerup_spawn_condition = true;
                        if (Data::powerup_spawn_condition) {
                            int temp_rand = rand() % 4;
                            if (temp_rand == 0 || temp_rand == 2)
                                powerups.emplace_back(PowerUp(player, 0));
                            else if (temp_rand == 1)
                                powerups.emplace_back(PowerUp(player, 1));
                            Data::powerup_spawn_condition = false;
                        }
                    }
                    enemy.getBullet().move();
                    if (playerNearEnemy(player, enemy) && !enemy.getBullet().isActive())
                        enemy.shoot(player);
                    if (checkEnemyBulletAndPlayerCollision(player, enemy)) {
                        player.modifyHealth(-20);
                        if (sound_on)
                            SoundSystem::playerHitSound();
                        enemy.getBullet().deactivate();
                    }
                    if (checkPlayerAndEnemyCollision(player, enemy)) {
                        player.setCauseOfDeath(1);
                        player.modifyHealth(-100);
                    }
                    enemy.setIsHit(false);
                }
                enemies.erase(
                    remove_if(enemies.begin(), enemies.end(),
                        [](Enemy& e) { return e.getHP() <= 0; }),
                    enemies.end());

                // iterate powerups vector
                for (auto it = powerups.begin(); it != powerups.end();) {
                    if (checkPlayerAndPowerUpCollision(player, *it)) {
                        if (sound_on)
                            SoundSystem::playPowerUpPickedSound();
                        if (it->getType() == 0) {
                            player.modifyHealth(20);
                        }
                        else if (it->getType() == 1) {
                            player.modifyBulletsCount(rand() % 10 + 11);
                            player.modifyRocketsCount(rand() % 4 + 2);
                        }
                        Data::powerup_and_player_collision = true;
                        it = powerups.erase(it);
                    }
                    else
                        ++it;
                }

                // check if player has not been killed
                if (player.getHP() <= 0) {
                    player.setName(gameOver(player, enemies, powerups));
                    file_manager.saveScoreToFile(player);
                    break;
                }

                // move entities
                moveEnemies(player, enemies);

                // draw map and set some game data false
                drawMap(map, player, enemies, powerups);

                Data::powerup_and_player_collision = false;
                Data::display_no_bullets = false;
                Data::display_no_rockets = false;
                Data::display_enemy_killed = false;

                player.normalizeHP();

                // framerate and game speed
                Sleep(40);
            }
            break;
        case '2':
            // display controls

            SoundSystem::playMenuSound();
            while (true) {
                system("cls");
                wcout << L"╔════════════════════════╗\n";
                wcout << L"║      * CONTROLS *      ║\n";
                wcout << L"╠═══╦════════════════════╣\n";
                wcout << L"║ W ║  Up                ║\n";
                wcout << L"║ A ║  Left              ║\n";
                wcout << L"║ S ║  Down              ║\n";
                wcout << L"║ D ║  Right             ║\n";
                wcout << L"║ F ║  Fire a bullet     ║\n";
                wcout << L"║ R ║  Fire a missile    ║\n";
                wcout << L"╚═══╩════════════════════╝\n";
                setConsoleColor(10);
                wcout << L"\n   Press any key to proceed to the menu.";
                setConsoleColor(7);
                _getch();
                break;
            }
            break;
        case '3':
            // display game rules 

            if (sound_on)
                SoundSystem::playMenuSound();
            system("cls");
            wcout << L"\n   Goal: Survive as long as you can with infinite\n";
            wcout << L"   enemies coming towards you\n\n";
            wcout << L"   Game symbols:\n";
            setConsoleColor(2);
            wcout << L"   ⎔";
            setConsoleColor(7);
            wcout << L" - player\n";
            setConsoleColor(4);
            wcout << L"   ⬢";
            setConsoleColor(7);
            wcout << L" - enemy\n";
            setConsoleColor(8);
            wcout << L"   ·";
            setConsoleColor(7);
            wcout << L" - player's bullet\n";
            setConsoleColor(12);
            wcout << L"   ·";
            setConsoleColor(7);
            wcout << L" - enemy's bullet\n";
            setConsoleColor(8);
            wcout << L"   ◄, ►, ▲, ▼ ";
            setConsoleColor(7);
            wcout << L" - player's missile\n";
            setConsoleColor(9);
            wcout << L"   ♥";
            setConsoleColor(7);
            wcout << L" - powerup adding 20 HP (two bars)\n";
            setConsoleColor(9);
            wcout << L"   ⌻";
            setConsoleColor(7);
            wcout << L" - powerup adding 10 to 20 bullets and\n";
            wcout << L"   1 to 4 missiles\n\n";
            wcout << L"   When an enemy is hit by player's bullet it gets 30 HP of damage,\n";
            wcout << L"   when hit by a missile it's 100 HP (elimination with one hit).\n";
            wcout << L"   There's a 50% chance of getting health powerup adding\n";
            wcout << L"   20 HP, 25% chance of getting ammo adding powerup and\n";
            wcout << L"   25% chance of getting no powerup.\n";
            setConsoleColor(10);
            wcout << L"\n   Press any key to proceed to the menu.";
            setConsoleColor(7);
            _getch();
            system("cls");
            break;
        case '4':
            // display scorelist read from file
            if (sound_on)
                SoundSystem::playMenuSound();

            scorelist = file_manager.readScoreListFromFile();

            system("cls");
            wcout << L"   ╔═════════════════════════════════════╗\n";
            wcout << L"   ║            * SCORELIST *            ║\n";
            wcout << L"   ╠═════╦══════════════════════╦════════╣ \n";
            for (const auto& line : scorelist) {
                wcout << L"   ║" << fixed << setw(4) << score_number << L" ║ " << setw(20) << line.first << L" ║ " << setw(6) << line.second << L" ║" << endl;
                score_number++;
            }
            wcout << L"   ╚═════╩══════════════════════╩════════╝ \n";
            
            _getch();
            break;
        case '5':
            // display information about the author
            if (sound_on)
                SoundSystem::playMenuSound();

            system("cls");
            wcout << "\n\n\n";
            wcout << L"   ╔════════════════════════════════════════════════════╗ \n";
            wcout << L"   ║   Game fully made by Ignacy \"Raxyen\" Chmielewski   ║░ \n";
            wcout << L"   ║";
            setConsoleColor(10);
            wcout << L"                www.youtube.com/raxyen              ";
            setConsoleColor(7);
            wcout << L"║░ \n";
            wcout << L"   ╚════════════════════════════════════════════════════╝░ \n";
            wcout << L"    ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ \n";
            setConsoleColor(10);
            wcout << L"\n   Press any key to proceed to the menu.";
            setConsoleColor(7);
            _getch();
            system("cls");
            break;
        case '6':
            // options
            if (sound_on)
                SoundSystem::playMenuSound();
            system("cls");

            while (true) {
                system("cls");
                wcout << L"╔═════════════════════════╗\n";
                wcout << L"║      *  OPTIONS  *      ║\n";
                wcout << L"╠═══╦═══════════════╦═════╣\n";
                wcout << L"║ 1 ║  Sound        ║ ";
                sound_on ? wcout << "ON " : wcout << "OFF";
                wcout << L" ║\n";
                wcout << L"╚═══╩═══════════════╩═════╝\n";
                setConsoleColor(10);
                wcout << L"\n   Press ESC to proceed to the menu.";
                setConsoleColor(7);

                int key = _getch();
                if (key == '1') {
                    sound_on = !sound_on;
                }
                else if (key == 27) { // ESC
                    break;
                }
            }
            break;
        case '7':
            SoundSystem::playMenuSound();
            // display quit screen

            while (true)
            {
                system("cls");
                setConsoleColor(14);
                wcout << "\n\n\n";
                wcout << L"   ╔═════════════════════════════════════╗\n";
                wcout << L"   ║    Are you sure you want to quit?   ║░\n";
                wcout << L"   ║             y/n (yes/no)            ║░\n";
                wcout << L"   ╚═════════════════════════════════════╝░\n";
                wcout << L"    ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n";
                char quit;
                quit = _getch();
                if (quit == 'y' || quit == 'Y') { // exits the application if provided character is 'y'
                    return 0;
                }
                else if (quit == 'n' || quit == 'N') { // returns to main menu loop if provided character is 'n'   
                    setConsoleColor(7);
                    break;            
                }
            }
            break;
        }
    }
}