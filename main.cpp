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

#include "Position.h"
#include "Projectile.h"
#include "Bullet.h"
#include "Rocket.h"
#include "Player.h"
#include "Enemy.h"
#include "FileManager.h"
#include "PowerUp.h"


using namespace std;

// sound

void playMenuSound() { thread([] { Beep(500, 50); }).detach(); }

void playEnemyHitSound() { thread([] { Beep(300, 50); }).detach(); }

void playerHitSound() {
    thread([] {
        Beep(200, 100);
        Beep(150, 100);
        }).detach();
}

void playEnemyKilledSound() {
    thread([] {
        Beep(300, 50);
        Beep(300, 50);
        Beep(400, 50);
        }).detach();
}

void playBulletSound() { thread([] { Beep(1000, 30); }).detach(); }

void playRocketSound() {
    thread([] {
        for (DWORD i = 300; i > 60; i -= 10)
            Beep(i, 20); }).detach();
}

void playPowerUpPickedSound() {
    thread([] {
        Beep(400, 50);
        Beep(500, 50);
        Beep(400, 50);
        Beep(500, 50);
        }).detach();
}

struct Data {
    wstring message1 = L"\nEnemy eliminated!";
    wstring message2 = L"\nPowerup collected!";
    wstring message3 = L"\nYou are hurt. Hide.";
    wstring message4 = L"\nYou're out of bullets!";
    wstring message5 = L"\nYou're out of missiles!";

    bool display_no_bullets;
    bool display_no_rockets;
    bool display_enemy_killed;
    bool powerup_and_player_collision;
    bool powerup_spawn_condition;
};

// system functions

void setupConsole() { // wide characters
    _setmode(_fileno(stdout), _O_U16TEXT);
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
    for (auto& bullet : player.getProjectiles())
        bullet.deactivate();
    for (auto& enemy : enemies)
        enemy.getBullet().deactivate();
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

void initMap(array<array<char, MAP_HEIGHT>, MAP_WIDTH> map) { // initializes map fields
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            map[i][j] = ' ';
        }
    }
}

// game objects manipulation 

void spawnEnemies(Player& player, vector<Enemy>& enemies, unsigned int count) {
    array<char, 4> dir = { 'w', 'a', 's', 'd' };
    for (size_t i = 0; i < count; i++)
        enemies.emplace_back(Enemy(dir[rand() % 4], player));
}

void moveEnemies(Player& player, vector<Enemy>& enemies) {
    bool occupied[MAP_HEIGHT][MAP_WIDTH] = { false };

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
void drawMap(array<array<char, MAP_HEIGHT>, MAP_WIDTH>& map, Player& player, vector<Enemy>& enemies, vector<PowerUp>& powerups, Data& data) {
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

    setConsoleColor(7);
    wcout << L"╔"; // left upper corner of the frame
    for (int i = 0; i < MAP_WIDTH; i++) {
        wcout << L"═";
    }
    wcout << L"╗" << endl; // right upper corner of the frame

    for (int i = 0; i < MAP_HEIGHT; i++) {
        wcout << L"║"; // left side of the frame
        for (int j = 0; j < MAP_WIDTH; j++) {
            bool drawn = false; // flag to check if the element is drawn on x y

            // draw player
            if (!drawn && i == static_cast<int>(player.getPosition().getY()) && j == static_cast<int>(player.getPosition().getX())) {
                setConsoleColor(2); // set green color for player
                wcout << player.getTexture(); // draw player 
                setConsoleColor(7); // reset to white color
                drawn = true;
            }

            // draw player's projectiles (bullets and rockets) 
            if (!drawn) { 
                for (auto& projectile : player.getProjectiles()) {
                    if (projectile.isActive() && i == static_cast<int>(projectile.getPosition().getY()) && j == static_cast<int>(projectile.getPosition().getX())) {
                        setConsoleColor(8); // set gray color for projectile
                        if (projectile.getType() == 'b') {
                            wcout << L"·";
                        }
                        else if (projectile.getType() == 'r') {
                            if (projectile.getDirection() == 'w') wcout << L"▲"; // rocket with up direction
                            else if (projectile.getDirection() == 'a') wcout << L"◄"; // rocket with left direction
                            else if (projectile.getDirection() == 's') wcout << L"▼"; // rocket with down direction
                            else if (projectile.getDirection() == 'd') wcout << L"►"; // rocket with right direction  
                        }
                        setConsoleColor(7); // reset to white color
                        drawn = true;
                        break;
                    }
                }
            }

            // draw enemies
            if (!drawn) {
                for (auto& enemy : enemies) {
                    if (i == static_cast<int>(enemy.getPosition().getY()) && j == static_cast<int>(enemy.getPosition().getX())) {
                        setConsoleColor(enemy.isHit() ? 13 : (enemy.getHP() > 30 ? 4 : 6)); // set red color for enemy if enemy's hp > 30 else set yellow
                        wcout << enemy.getTexture();
                        setConsoleColor(7); // reset to white color
                        drawn = true;
                        break;
                    }
                }
            }

            // draw enemy bullets for each enemy
            if (!drawn) {
                for (auto& enemy : enemies) {
                    if (enemy.getBullet().isActive() && i == static_cast<int>(enemy.getBullet().getPosition().getY()) && j == static_cast<int>(enemy.getBullet().getPosition().getX())) { // draw enemy's bullet
                        setConsoleColor(12); // set light red color for rocket
                        if (enemy.getBullet().getDirection() == 'a' || enemy.getBullet().getDirection() == 'd') {
                            wcout << L"·"; // horizontal bullet
                        }
                        else if (enemy.getBullet().getDirection() == 'w' || enemy.getBullet().getDirection() == 's') {
                            wcout << L"·"; // vertical bullet
                        }
                        setConsoleColor(7); // reset to white color
                        drawn = true;
                    }
                }
            }

            // draw powerups
            if (!drawn) {
                for (auto& powerup : powerups) {
                    if (!drawn && i == static_cast<int>(powerup.getPosition().getY()) && j == static_cast<int>(powerup.getPosition().getX())) {
                        setConsoleColor(9);  // set blue color for powerup
                        wcout << powerup.getTexture();
                        setConsoleColor(7); // reset to white color
                        drawn = true;
                    }
                }
            }
            if (!drawn)                
                wcout << map[j][i];
        }
        wcout << L"║" << endl; // parts of right side of the frame
    }

    wcout << L"╚"; // left lower corner of the frame
    for (int i = 0; i < MAP_WIDTH; i++) {
        wcout << L"═"; // lower side of the frame
    }
    wcout << L"╝" << endl; // right lower corner of the frame

    // display player stats
    setConsoleColor(12);
    player.info();

    // draw player HP {
    for (int i = 0; i < player.getHP() / 10; i++) 
        wcout << L"█";
    for (int i = 0; i < 10 - player.getHP() / 10; i++)
        wcout << L"░";
    setConsoleColor(8);

    // display messages

    static unsigned int enemy_killed_message_counter {0};
    if (data.display_enemy_killed) enemy_killed_message_counter++;
    else if (enemy_killed_message_counter < 10 && enemy_killed_message_counter > 0) {
        if (enemy_killed_message_counter % 2) {
            setConsoleColor(4);
            wcout << data.message1;
            setConsoleColor(7);
            enemy_killed_message_counter++;
        }
        else enemy_killed_message_counter++;
    }
    else enemy_killed_message_counter = 0;

    static unsigned int power_up_collected_message_counter {0};
    if (data.powerup_and_player_collision) power_up_collected_message_counter++;
    else if (power_up_collected_message_counter < 10 && power_up_collected_message_counter > 0) {
        if (power_up_collected_message_counter % 2) {
            setConsoleColor(3);
            wcout << data.message2;
            setConsoleColor(7);
            power_up_collected_message_counter++;
        }
        else power_up_collected_message_counter++;
    }
    else power_up_collected_message_counter = 0;

    static unsigned int player_low_hp_message_counter = 0;
    if (player.getHP() <= 20) {
        if (player_low_hp_message_counter % 2) {
            setConsoleColor(12);
            wcout << data.message3;
            setConsoleColor(7);
            player_low_hp_message_counter++;
        }
        else player_low_hp_message_counter++;
    }
    else player_low_hp_message_counter = 0;

    static unsigned int no_bullets_message_counter = 0;
    if (data.display_no_bullets) no_bullets_message_counter++;
    else if (no_bullets_message_counter < 20 && no_bullets_message_counter > 0) {
        setConsoleColor(3);
        wcout << data.message4;
        setConsoleColor(7);
        no_bullets_message_counter++;
    }
    else {
        no_bullets_message_counter = 0;
        data.display_no_bullets = false;
    }

    static unsigned int no_rocket_message_counter = 0;
    if (data.display_no_rockets) no_rocket_message_counter++;
    else if (no_rocket_message_counter < 20 && no_rocket_message_counter > 0) {
        setConsoleColor(3);
        wcout << data.message5;
        setConsoleColor(7);
        no_rocket_message_counter++;
    }
    else {
        no_rocket_message_counter = 0;
        data.display_no_rockets = false;
    }
}

int main() {
    srand(time(NULL));

    array<array<char, MAP_HEIGHT>, MAP_WIDTH> map;
    Player player;
    vector<Enemy> enemies;
    vector<PowerUp> powerups;
    Data data;
    FileManager file_manager;

    // temp
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
            // map initialization
            if (sound_on)
                thread(playMenuSound).detach();

            for (auto& row : map)
                row.fill(' ');

            player.reset();

            data.display_no_bullets = false;
            data.display_no_rockets = false;
            data.display_enemy_killed = false;
            data.powerup_and_player_collision = false;
            data.powerup_spawn_condition = false;

            spawnEnemies(player, enemies, 4);
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
                            data.display_no_bullets = true;
                        }
                        else {
                            player.shoot('b'); // shoot a bullet
                            if (sound_on)
                                thread(playBulletSound).detach();
                        }
                    }
                    if ((key == 'r' || key == 'R')) {
                        if (player.getRocketsCount() == 0) {
                            data.display_no_rockets = true;
                        }
                        else {
                            player.shoot('r'); // shoot a rocket
                            if (sound_on)
                                thread(playRocketSound).detach();
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
                            thread(playEnemyHitSound).detach();
                        enemy.setIsHit(true);
                        enemy.modifyHealth(dmg);
                        player.modifyScore(10);
                        if (enemy.getHP() <= 0)
                            data.powerup_spawn_condition = true;
                    }
                    if (enemy.getHP() <= 0) {
                        if (sound_on)
                            thread(playEnemyKilledSound).detach();
                        enemy.getBullet().deactivate();
                        player.modifyScore(50);
                        player.kill();
                        data.display_enemy_killed = true;
                        data.powerup_spawn_condition = true;
                        if (data.powerup_spawn_condition) {
                            int temp_rand = rand() % 4;
                            if (temp_rand == 0 || temp_rand == 2)
                                powerups.emplace_back(PowerUp(player, 0));
                            else if (temp_rand == 1)
                                powerups.emplace_back(PowerUp(player, 1));
                            data.powerup_spawn_condition = false;
                        }
                    }
                    enemy.getBullet().move();
                    if (playerNearEnemy(player, enemy) && !enemy.getBullet().isActive())
                        enemy.shoot(player);
                    if (checkEnemyBulletAndPlayerCollision(player, enemy)) {
                        player.modifyHealth(-20);
                        if (sound_on)
                            thread(playerHitSound).detach();
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
                            thread(playPowerUpPickedSound).detach();
                        if (it->getType() == 0) {
                            player.modifyHealth(20);
                        }
                        else if (it->getType() == 1) {
                            player.modifyBulletsCount(rand() % 10 + 11);
                            player.modifyRocketsCount(rand() % 4 + 2);
                        }
                        data.powerup_and_player_collision = true;
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
                drawMap(map, player, enemies, powerups, data);

                data.powerup_and_player_collision = false;
                data.display_no_bullets = false;
                data.display_no_rockets = false;
                data.display_enemy_killed = false;

                player.normalizeHP();

                // framerate and game speed
                Sleep(40);
            }
            break;
        case '2':
            // display controls
            thread(playMenuSound).detach();
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
                thread(playMenuSound).detach();
            system("cls");
            wcout << L"\n   Goal: Survive as long as you can with infinite\n";
            wcout << L"   enemies coming towards you\n\n";
            wcout << L"   Game symbols:\n";
            setConsoleColor(2);
            wcout << L"   @";
            setConsoleColor(7);
            wcout << L" - player\n";
            setConsoleColor(4);
            wcout << L"   X";
            setConsoleColor(7);
            wcout << L" - enemy\n";
            setConsoleColor(8);
            wcout << L"   ─ or │";
            setConsoleColor(7);
            wcout << L" - player's bullet\n";
            setConsoleColor(12);
            wcout << L"   ─ or │";
            setConsoleColor(7);
            wcout << L" - enemy's bullet\n";
            setConsoleColor(8);
            wcout << L"   ←, →, ↑, ↓ ";
            setConsoleColor(7);
            wcout << L" - player's missile\n";
            setConsoleColor(9);
            wcout << L"   $";
            setConsoleColor(7);
            wcout << L" - powerup adding 20 HP (two bars)\n";
            setConsoleColor(9);
            wcout << L"   *";
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
                thread(playMenuSound).detach();
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
                thread(playMenuSound).detach();
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
                thread(playMenuSound).detach();
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
            thread(playMenuSound).detach();
            // display quit screen
            while (true)
            {
                system("cls");
                setConsoleColor(9);
                wcout << "\n\n\n";
                wcout << L"   ╔═════════════════════════════════════╗\n";
                wcout << L"   ║    Are you sure you want to quit?   ║░\n";
                wcout << L"   ║             y/n (yes/no)            ║░\n";
                wcout << L"   ╚═════════════════════════════════════╝░\n";
                wcout << L"    ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n";
                char quit;
                quit = _getch();
                if (quit == 'y' || quit == 'Y') return 0; // exits the application if provided character is 'y'
                else if (quit == 'n' || quit == 'N') {
                    setConsoleColor(7);
                    break; // returns to main menu loop if provided character is 'n'               
                }
            }
            break;
        }
    }
}