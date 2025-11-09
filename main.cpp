#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <conio.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <ctime>
#include <cmath>

const int MAP_HEIGHT = 15;
const int MAP_WIDTH = 30;

using namespace std;

struct Position {
    float x, y;
};

struct Player {
    Position pos = { 0,0 };
    char facing = ' ';
    int hp = 100;
    unsigned int kills = 0;
    unsigned int score = 0;
    unsigned int bullets = 0;
    unsigned int rockets = 0;
    unsigned int bullets_fired = 0;
    unsigned int rockets_fired = 0;
    int cause_of_death = 0;
};

struct Bullet {
    Position pos = { 0,0 };
    bool is_enemy = false;
    bool isActive = false;
    char direction = ' ';
};

struct Enemy {
    Position pos = { 0,0 };
    Bullet bullet;
    bool isHit = false;
    char direction = ' ';
    int hp = 100;
};

struct Rocket {
    Position pos = { 0,0 };
    bool isActive = false;
    char direction = ' ';
};

struct PowerUp {
    Position pos = { 0,0 };
    int type = 0;
    char texture = ' ';
};

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

void setupConsole() // allows wide characters 
{
    SetConsoleOutputCP(CP_UTF8);
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
    wcout << L"║ 4 ║  Scorelist (wip)  ║\n";
    wcout << L"║ 5 ║  Credits          ║\n";
    wcout << L"║ 6 ║  Quit game        ║\n";
    wcout << L"╚═══╩═══════════════════╝\n";
}

// general game functions

void resetPlayerStats(Player& player, vector<Bullet>& bullets, vector<Enemy>& enemies) {
    player.hp = 100;
    player.kills = 0;
    for (auto i : bullets) i.isActive = false;
    Enemy enemy;
    for (auto enemy : enemies)
        enemy.bullet.isActive = false;
    player.pos.x = 10.0f; // starting x position of the player
    player.pos.y = 5.0f; // starting y position of the player
    player.bullets = 50;
    player.rockets = 10;
    player.score = 0;
}

void gameOver(Player& player, vector<Bullet>& bullets, vector<Enemy>& enemies, vector<PowerUp>& powerups) {
    system("cls");
    setConsoleColor(12);
    wcout << "\n\n\n";
    wcout << L"   ╔═════════════════════════════╗\n";
    wcout << L"   ║                             ║░\n";
    wcout << L"   ║          GAME OVER          ║░\n";
    wcout << L"   ║                             ║░\n";
    wcout << L"   ╚═════════════════════════════╝░\n";
    wcout << L"    ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░\n\n";
    wcout << "  * Score: " << player.score << "\n  * Enemy kills: " << player.kills << L"\n  * Killed by: ";
    player.cause_of_death == 1 ? wcout << L"Enemy crashing you\n\n" : wcout << L"Enemy bullet\n\n";
    wcout << L"   Press any key to proceed to the menu.\n";
    enemies.clear();
    bullets.clear();
    powerups.clear();
    Sleep(2000);
    _getch();
    resetPlayerStats(player, bullets, enemies);
    setConsoleColor(7);
}

void initMap(char map[MAP_HEIGHT][MAP_WIDTH]) { // initializes map fields
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) map[i][j] = ' ';
    }
}

// game objects manipulation 

void createBullet(Player& player, vector<Bullet>& bullets, vector<Enemy>& enemies, bool is_enemy) {   
    if (is_enemy) {
        for (auto& enemy : enemies) {
            enemy.bullet.isActive = true;
            enemy.bullet.is_enemy = true;
            enemy.bullet.pos = enemy.pos;
            if (static_cast<int>(enemy.pos.y) > static_cast<int>(player.pos.y)) enemy.bullet.direction = 'w';
            else if (static_cast<int>(enemy.pos.x) > static_cast<int>(player.pos.x)) enemy.bullet.direction = 'a';
            else if (static_cast<int>(enemy.pos.y) < static_cast<int>(player.pos.y)) enemy.bullet.direction = 's';
            else if (static_cast<int>(enemy.pos.x) < static_cast<int>(player.pos.x)) enemy.bullet.direction = 'd';
        }
    }
    else {
        Bullet bullet;
        bullet.isActive = true;
        bullet.is_enemy = false;
        bullet.pos = player.pos;
        bullet.direction = player.facing;
        bullets.emplace_back(bullet);
    }
}

void moveBullet(vector<Bullet>& bullets) { // moves bullet one field per tick
    for(Bullet& bullet : bullets) {
        if (bullet.isActive) {
            if (bullet.direction == 'w' && bullet.pos.y > 0) bullet.pos.y -= 1.0f;
            else if (bullet.direction == 'a' && bullet.pos.x > 0) bullet.pos.x -= 1.0f;
            else if (bullet.direction == 's' && bullet.pos.y < MAP_HEIGHT - 1) bullet.pos.y += 1.0f;
            else if (bullet.direction == 'd' && bullet.pos.x < MAP_WIDTH - 1) bullet.pos.x += 1.0f;
            else bullet.isActive = false;
        }
    }
}

void moveEnemyBullet(Enemy& enemy) { // moves enemy bullet one field per tick
    if (enemy.bullet.isActive) {
        if (enemy.bullet.direction == 'w' && enemy.bullet.pos.y > 0) enemy.bullet.pos.y -= 0.5f;
        else if (enemy.bullet.direction == 'a' && enemy.bullet.pos.x > 0) enemy.bullet.pos.x -= 0.5f;
        else if (enemy.bullet.direction == 's' && enemy.bullet.pos.y < MAP_HEIGHT - 1) enemy.bullet.pos.y += 0.5f;
        else if (enemy.bullet.direction == 'd' && enemy.bullet.pos.x < MAP_WIDTH - 1) enemy.bullet.pos.x += 0.5f;
        else enemy.bullet.isActive = false;
    }
}

void createEnemy(Player& player, vector<Enemy>& enemies) { // initializes enemy object
    Enemy enemy;
    enemy.hp = 100;
    unsigned int rnd_x, rnd_y;
    rnd_x = rand() % MAP_WIDTH;
    rnd_y = rand() % MAP_HEIGHT;
    if (rnd_x != player.pos.x && rnd_y != player.pos.y) {
        enemy.pos.x = rnd_x;
        enemy.pos.y = rnd_y;
    }
    else {
        enemy.pos.x = rnd_x + rand() % 3 - 2;
        enemy.pos.y = rnd_y + rand() % 3 - 2;
    }
    enemies.emplace_back(enemy);
}

void spawnEnemies(Player& player, vector<Enemy>& enemies, unsigned int count) {
    for (size_t i = 0; i < count; i++)
        createEnemy(player, enemies);
}

void moveEnemy(Player& player, vector<Enemy>& enemies) {
    bool occupied[MAP_HEIGHT][MAP_WIDTH] = { false };

    for (auto& e : enemies)
        occupied[static_cast<int>(e.pos.y)][static_cast<int>(e.pos.x)] = true;

    // iterate every enemy
    for (auto& e : enemies) {

        int oldX = static_cast<int>(e.pos.x);
        int oldY = static_cast<int>(e.pos.y);

        // determine enemy direction
        int dx = 0, dy = 0;

        if (fabs(player.pos.x - e.pos.x) > fabs(player.pos.y - e.pos.y))
            dx = (player.pos.x > e.pos.x) ? 1 : -1;
        else
            dy = (player.pos.y > e.pos.y) ? 1 : -1;

        int newX = oldX + dx;
        int newY = oldY + dy;

        // check if other enemy doesn't occupy the same position
        if (newX >= 0 && newX < MAP_WIDTH && newY >= 0 && newY < MAP_HEIGHT && !occupied[newY][newX]) {
            // updating if position is occupied 
            occupied[oldY][oldX] = false;
            occupied[newY][newX] = true;

            // move enemy
            e.pos.x += dx * 0.2f;
            e.pos.y += dy * 0.2f;
        }
    }
}

void createRocket(Player& player, Rocket& rocket) { // initializes rocket object when 'r' or 'R' key pressed
    rocket.isActive = true;
    rocket.pos = player.pos;
    rocket.direction = player.facing;
}

void moveRocket(Rocket& rocket) { // moves bullet one field per tick
    if (rocket.isActive) {
        if (rocket.direction == 'w' && rocket.pos.y > 0) rocket.pos.y -= 1.0;
        else if (rocket.direction == 'a' && rocket.pos.x > 0) rocket.pos.x -= 1.0;
        else if (rocket.direction == 's' && rocket.pos.y < MAP_HEIGHT - 1) rocket.pos.y += 1.0;
        else if (rocket.direction == 'd' && rocket.pos.x < MAP_WIDTH - 1) rocket.pos.x += 1.0;
        else rocket.isActive = false;
    }
}

void createPowerUp(Player& player, vector<PowerUp>& powerups, int type) { // initializes power_up object
    PowerUp powerup;
    powerup.type = type;
    powerup.texture = (type == 1) ? '*' : '$';
    
    unsigned int x, y;   
    do {
        x = rand() % (MAP_WIDTH - 2) + 1;
        y = rand() % (MAP_HEIGHT - 2) + 1;
    } while (x == static_cast<int>(player.pos.x) && y == static_cast<int>(player.pos.y));

    powerup.pos.x = x;
    powerup.pos.y = y;

    powerups.emplace_back(powerup);
}

// game objects collisions

bool checkPlayerBulletAndEnemyCollision(Enemy& enemy, vector<Bullet>& bullets) {
    for (Bullet& bullet : bullets) {
        if (bullet.isActive && !bullet.is_enemy) {
            float dx = fabs(enemy.pos.x - bullet.pos.x);
            float dy = fabs(enemy.pos.y - bullet.pos.y);

            // solve tunneling problem
            if (dx < 0.6f && dy < 0.6f) {
                bullet.isActive = false;
                return true;
            }
        }
    }
    return false;
}

bool checkPlayerRocketAndEnemyCollision(Enemy& enemy, Rocket& rocket) { // checks if both the coordinates of enemy and player's rocket are equal (checks collision)
    return (rocket.isActive && static_cast<int>(enemy.pos.x) == static_cast<int>(rocket.pos.x) && static_cast<int>(enemy.pos.y) == static_cast<int>(rocket.pos.y));
}

bool checkEnemyBulletAndPlayerCollision(Player& player, Enemy& enemy) { // checks if both the coordinates of enemy's bullet and player are equal (checks collision)
    return (enemy.bullet.isActive && enemy.bullet.is_enemy && static_cast<int>(player.pos.x) == static_cast<int>(enemy.bullet.pos.x) && static_cast<int>(player.pos.y) == static_cast<int>(enemy.bullet.pos.y));
}

bool checkPlayerAndEnemyCollision(Player& player, Enemy& enemy) { // checks if both the coordinates of enemy and player are equal (checks collision)
    return (static_cast<int>(enemy.pos.x) == static_cast<int>(player.pos.x) && static_cast<int>(enemy.pos.y) == static_cast<int>(player.pos.y));
}

bool checkPlayerAndPowerUpCollision(Player& player, PowerUp& power_up) { // checks if both the coordinates of powerup and player are equal (checks collision)
    return (static_cast<int>(power_up.pos.x) == static_cast<int>(player.pos.x) && static_cast<int>(power_up.pos.y) == static_cast<int>(player.pos.y));
}

//checks whether the player distance from enemy is least or equal than 14 
//and if the player and enemy are in the same line to make a shot possible

bool playerNearEnemy(Player& player, Enemy& enemy) {
    if (static_cast<int>(player.pos.x) == static_cast<int>(enemy.pos.x) && abs(player.pos.y - enemy.pos.y) <= 14) {
        return true; // same column
    }
    else if (static_cast<int>(player.pos.y) == static_cast<int>(enemy.pos.y) && abs(player.pos.x - enemy.pos.x) <= 14) {
        return true; // same row
    }
    return false;
}

// drawing map
void drawMap(char map[MAP_HEIGHT][MAP_WIDTH], Player& player, vector<Bullet>& bullets, Rocket& rocket, vector<Enemy>& enemies, vector<PowerUp>& powerups, Data& data) {
    system("cls");

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

            if (!drawn && i == static_cast<int>(player.pos.y) && j == static_cast<int>(player.pos.x)) {
                setConsoleColor(2); // set green color for player
                wcout << '@'; // draw player 
                setConsoleColor(7); // reset to white color
                drawn = true;
            }

            if (!drawn) { // draw player's bullets 
                for (auto& bullet : bullets) {
                    if (bullet.isActive && i == static_cast<int>(bullet.pos.y) && j == static_cast<int>(bullet.pos.x)) {
                        setConsoleColor(8); // set gray color for bullet
                        if (bullet.direction == 'a' || bullet.direction == 'd') wcout << L"─"; // horizontal bullet
                        else if (bullet.direction == 'w' || bullet.direction == 's') wcout << L"│"; // vertical bullet
                        setConsoleColor(7); // reset to white color
                        drawn = true;
                        break;
                    }
                }
            }

            // draw player's rocket
            if (!drawn && rocket.isActive && i == static_cast<int>(rocket.pos.y) && j == static_cast<int>(rocket.pos.x)) {
                setConsoleColor(8); // set gray color for rocket
                if (rocket.direction == 'w') wcout << L"↑"; // rocket with up direction
                else if (rocket.direction == 'a') wcout << L"←"; // rocket with left direction
                else if (rocket.direction == 's') wcout << L"↓"; // rocket with down direction
                else if (rocket.direction == 'd') wcout << L"→"; // rocket with right direction                
                setConsoleColor(7); // reset to white color
                drawn = true;
            }

            // draw enemies
            if (!drawn) {
                for (auto& enemy : enemies) {
                    if (i == static_cast<int>(enemy.pos.y) && j == static_cast<int>(enemy.pos.x)) {
                        setConsoleColor(enemy.isHit ? 13 : (enemy.hp > 30 ? 4 : 6)); // set red color for enemy if enemy's hp > 30 else set yellow
                        wcout << 'X';
                        setConsoleColor(7); // reset to white color
                        drawn = true;
                        break;
                    }
                }
            }

            // draw enemy bullets for each enemy
            if (!drawn) {
                for (auto& enemy : enemies) {
                    if (enemy.bullet.isActive && i == static_cast<int>(enemy.bullet.pos.y) && j == static_cast<int>(enemy.bullet.pos.x)) { // draw enemy's bullet
                        setConsoleColor(12); // set light red color for rocket
                        if (enemy.bullet.direction == 'a' || enemy.bullet.direction == 'd') wcout << L"─"; // horizontal bullet
                        else if (enemy.bullet.direction == 'w' || enemy.bullet.direction == 's') wcout << L"│"; // vertical bullet
                        setConsoleColor(7); // reset to white color
                        drawn = true;
                    }
                }
            }
            
            // draw powerups
            if (!drawn) {
                for (auto& powerup : powerups) {
                    if (!drawn && i == static_cast<int>(powerup.pos.y) && j == static_cast<int>(powerup.pos.x)) {
                        setConsoleColor(9);  // set blue color for powerup
                        wcout << powerup.texture;
                        setConsoleColor(7); // reset to white color
                        drawn = true;
                    }
                }
            }
            if (!drawn) wcout << map[i][j];
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
    wcout << "Enemy kills: " << player.kills << endl;
    wcout << "Bullets fired: " << player.bullets_fired << endl;
    wcout << "Bullets: " << player.bullets << endl;
    wcout << "Missiles fired: " << player.rockets_fired << endl;
    wcout << "Missiles: " << player.rockets << endl;
    wcout << "Score: " << player.score << endl;
    wcout << "Health: ";

    // draw player HP {
    for (int i = 0; i < player.hp / 10; i++) wcout << L"█";
    for (int i = 0; i < 10 - player.hp / 10; i++) wcout << L"░";
    setConsoleColor(8);

    // display messages
 
    static unsigned int enemy_killed_message_counter = 0;
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

    static unsigned int power_up_collected_message_counter = 0;
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
    if (player.hp <= 20) {
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

    char map[MAP_HEIGHT][MAP_WIDTH];
    Player player;
    vector<Bullet>bullets;
    Rocket rocket;
    vector<Enemy> enemies;
    vector<PowerUp> powerups;
    Data data;

    char key;

    setupConsole();
    char menu_option;
    while (true) {
        menu();
        menu_option = _getch();
        switch (menu_option) {
        case '1':
            resetPlayerStats(player, bullets, enemies);                    
            rocket.isActive = false;

            data.display_no_bullets = false;            
            data.display_no_rockets = false;
            data.display_enemy_killed = false;
            data.powerup_and_player_collision = false;
            data.powerup_spawn_condition = false;

            player.bullets_fired = 0;
            player.rockets_fired = 0;
            player.cause_of_death = 0;
            initMap(map);
            spawnEnemies(player, enemies, 4);
            while (true) {
                if (_kbhit()) { // reading pressed keys
                    key = _getch();
                    if ((key == 'w' || key == 'W') && player.pos.y > 0) { // up
                        player.pos.y--;
                        player.facing = 'w';
                    }
                    if ((key == 'a' || key == 'A') && player.pos.x > 0) { // left
                        player.pos.x--;
                        player.facing = 'a';
                    }
                    if ((key == 's' || key == 'S') && player.pos.y < MAP_HEIGHT - 1) { // down
                        player.pos.y++;
                        player.facing = 's';
                    }
                    if ((key == 'd' || key == 'D') && player.pos.x < MAP_WIDTH - 1) { // right
                        player.pos.x++;
                        player.facing = 'd';
                    }
                    if ((key == 'f' || key == 'F')) {
                        if (player.bullets == 0) {
                            data.display_no_bullets = true;
                        }
                        else {
                            createBullet(player, bullets, enemies, false); // shoot a bullet
                            player.bullets--;
                            player.bullets_fired++;
                        }
                    }
                    if ((key == 'r' || key == 'R') && !(rocket.isActive)) {
                        if (player.rockets == 0) {
                            data.display_no_rockets = true;
                        }
                        else {
                            createRocket(player, rocket); // shoot a rocket
                            player.rockets--;
                            player.rockets_fired++;
                        }
                    }
                    // test only
                    if (key == 't' || key == 'T')
                        spawnEnemies(player, enemies, 4);
                }

                // iterate enemies vector
                for (auto& enemy : enemies) {
                    if (checkPlayerBulletAndEnemyCollision(enemy, bullets)) {
                        enemy.isHit = true;
                        enemy.hp -= 30;
                        player.score += 10;
                        if (enemy.hp <= 0) 
                            data.powerup_spawn_condition = true;
                    }
                    if (checkPlayerRocketAndEnemyCollision(enemy, rocket)) {
                        rocket.isActive = false;
                        enemy.hp = 0;
                    }
                    if (enemy.hp <= 0) {
                        enemy.bullet.isActive = false;
                        player.score += 50;
                        player.kills++;
                        data.display_enemy_killed = true;
                        data.powerup_spawn_condition = true;
                        if (data.powerup_spawn_condition) {
                            int temp_rand = rand() % 4;
                            if (temp_rand == 0 || temp_rand == 2) 
                                createPowerUp(player, powerups, 0);
                            else if (temp_rand == 1) 
                                createPowerUp(player, powerups, 1);
                            data.powerup_spawn_condition = false;
                        }
                    }                   
                    moveEnemyBullet(enemy);
                    if (playerNearEnemy(player, enemy) && !enemy.bullet.isActive) createBullet(player, bullets, enemies, true);
                    if (checkEnemyBulletAndPlayerCollision(player, enemy)) {
                        player.hp -= 20;
                        enemy.bullet.isActive = false;
                    }
                    if (checkPlayerAndEnemyCollision(player, enemy)) {
                        player.cause_of_death = 1;
                        player.hp = 0;
                    }
                    enemy.isHit = false;
                }
                enemies.erase(
                    remove_if(enemies.begin(), enemies.end(),
                        [](const Enemy& e) { return e.hp <= 0; }),
                    enemies.end());

                // iterate powerups vector
                for (auto it = powerups.begin(); it != powerups.end();) {
                    if (checkPlayerAndPowerUpCollision(player, *it)) {
                        if (it->type == 0) {
                            player.hp += 20;
                        }
                        else {
                            player.bullets += rand() % 10 + 11;
                            player.rockets += rand() % 4 + 2;
                        }
                        data.powerup_and_player_collision = true;
                        it = powerups.erase(it);
                    }
                    else
                        ++it;
                }

                // check if player has not been killed
                if (player.hp <= 0) {
                    gameOver(player, bullets, enemies, powerups);
                    break;
                }

                // move entities
                moveEnemy(player, enemies);
                moveBullet(bullets);
                moveRocket(rocket);

                // draw map and set some game data false
                drawMap(map, player, bullets, rocket, enemies, powerups, data);   

                data.powerup_and_player_collision = false;
                data.display_no_bullets = false;
                data.display_no_rockets = false;
                data.display_enemy_killed = false;
                
                if (player.hp > 100) player.hp = 100;

                // framerate and game speed
                Sleep(40);
            }
            break;
        case '2':
            // display controls
            while (1) {
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
        case '5':
            // display information about the author
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
            // display quit screen
            while (1)
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
                if (quit == 'y' || quit == 'Y') return 0; // exits the application if provided character is 't'
                else if (quit == 'n' || quit == 'N') {
                    setConsoleColor(7);
                    break; // returns to main menu loop if provided character is 'n'               
                }
            }
            break;
        }
    }
}

