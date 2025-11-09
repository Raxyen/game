#include <iostream>
#include <vector>
#include <cstdlib>
#include <conio.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>

#define MAP_HEIGHT 15
#define MAP_WIDTH 30

using namespace std;

struct Position {
    float x, y;
};

struct Player {
    Position pos;
    char facing;
    int hp = 100;
    unsigned int kills = 0;
    unsigned int score = 0;
    unsigned int bullets;
    unsigned int rockets;
    int cause_of_death;
};

struct Bullet {
    Position pos;
    bool isActive = false;
    char direction;
};

struct Enemy {
    Position pos;
    Bullet bullet;
    bool isActive = false;
    bool isHit = false;
    char direction;
    int hp;
};

struct Rocket {
    Position pos;
    bool isActive = false;
    char direction;
};

struct PowerUp {
    Position pos;
    bool isActive = false;
    int type;
};

struct Data {
    wstring message1 = L"\nEnemy eliminated!";
    wstring message2 = L"\nPowerup collected!";
    wstring message3 = L"\nYou are hurt. Hide.";
    wstring message4 = L"\nYou're out of bullets!";
    wstring message5 = L"\nYou're out of missiles!";
    bool display_no_bullets;
    bool display_no_rockets;
};

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

void resetPlayerStats(Player& player, vector<Bullet>& bullets, vector<Enemy>& enemies) {
    player.hp = 100;
    player.kills = 0;
    for (auto i : bullets) i.isActive = false;
    Enemy enemy;
    for (auto enemy : enemies) {
        enemy.isActive = false;
        enemy.bullet.isActive = false;
    }
    player.pos.x = 10.0f; // starting x position of the player
    player.pos.y = 5.0f; // starting y position of the player
    player.bullets = 50;
    player.rockets = 10;
    player.score = 0;
}

void gameOver(Player& player, vector<Bullet>& bullets, vector<Enemy>& enemies) {
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
    for (auto& enemy : enemies) {
        enemy.isActive = false;
        enemy.bullet.isActive = false;
    }
    for (auto bullet : bullets) 
        bullet.isActive = false;
    Sleep(2000);
    char t = _getch();
    resetPlayerStats(player, bullets, enemies);
    setConsoleColor(7);
}

void initMap(char map[MAP_HEIGHT][MAP_WIDTH]) { // initializes map fields
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) map[i][j] = ' ';
    }
}

void createBullet(Player& player, vector<Bullet>& bullets) { // initializes bullet object when 'f' or 'F' key pressed
    Bullet bullet;
    bullet.isActive = true;
    bullet.pos = player.pos;
    bullet.direction = player.facing;
    bullets.push_back(bullet);
}


void moveBullet(vector<Bullet>& bullets) { // moves bullet one field per tick
    for(Bullet& bullet : bullets) {
        if (bullet.isActive) {
            if (bullet.direction == 'w' && bullet.pos.y > 0) bullet.pos.y -= 1.0;
            else if (bullet.direction == 'a' && bullet.pos.x > 0) bullet.pos.x -= 1.0;
            else if (bullet.direction == 's' && bullet.pos.y < MAP_HEIGHT - 1) bullet.pos.y += 1.0;
            else if (bullet.direction == 'd' && bullet.pos.x < MAP_WIDTH - 1) bullet.pos.x += 1.0;
            else bullet.isActive = false;
        }
    }
}

void createEnemy(Player& player, vector<Enemy>& enemies) { // initializes enemy object
    Enemy enemy;
    enemy.isActive = true;
    enemy.hp = 100;
    unsigned int rnd_x, rnd_y;
    rnd_x = rand() % MAP_WIDTH + 1;
    rnd_y = rand() % MAP_HEIGHT + 1;
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

void moveEnemy(Player& player, Enemy& enemy) { // moves enemy random way
    if (enemy.isActive) {
        if (player.pos.y < enemy.pos.y && abs(player.pos.x - enemy.pos.x) < abs(player.pos.y - enemy.pos.y)) enemy.pos.y -= 0.2f;
        else if (player.pos.x < enemy.pos.x && abs(player.pos.x - enemy.pos.x) > abs(player.pos.y - enemy.pos.y)) enemy.pos.x -= 0.2f;
        else if (player.pos.y > enemy.pos.y  && abs(player.pos.x - enemy.pos.x) < abs(player.pos.y - enemy.pos.y)) enemy.pos.y += 0.2f;
        else if (player.pos.x > enemy.pos.x && abs(player.pos.x - enemy.pos.x) > abs(player.pos.y - enemy.pos.y)) enemy.pos.x += 0.2f;    
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

void createPowerUp(Player& player, PowerUp& power_up) { // initializes power_up object
    power_up.isActive = true;
    unsigned int rnd_x, rnd_y;
    rnd_x = rand() % MAP_WIDTH + 1;
    rnd_y = rand() % MAP_HEIGHT + 1;
    if (rnd_x != player.pos.x && rnd_y != player.pos.y) {
        power_up.pos.x = rnd_x;
        power_up.pos.y = rnd_y;
    }
    else {
        power_up.pos.x = rnd_x + rand() % 3 - 2;
        power_up.pos.y = rnd_y + rand() % 3 - 2;
    }
}

bool checkPlayerBulletAndEnemyCollision(Enemy& enemy, vector<Bullet>& bullets) {
    for (Bullet& bullet : bullets) {
        if (bullet.isActive && enemy.isActive && static_cast<int>(enemy.pos.x) == static_cast<int>(bullet.pos.x) && static_cast<int>(enemy.pos.y) == static_cast<int>(bullet.pos.y)) {
            bullet.isActive = false;
            return true;
        }
    }
    return false;
}

bool checkPlayerRocketAndEnemyCollision(Enemy& enemy, Rocket& rocket) { // checks if both the coordinates of enemy and player's rocket are equal (checks collision)
    return (rocket.isActive && static_cast<int>(enemy.pos.x) == static_cast<int>(rocket.pos.x) && static_cast<int>(enemy.pos.y) == static_cast<int>(rocket.pos.y));
}

bool checkEnemyBulletAndPlayerCollision(Player& player, Enemy& enemy) { // checks if both the coordinates of enemy's bullet and player are equal (checks collision)
    return (enemy.bullet.isActive && static_cast<int>(player.pos.x) == static_cast<int>(enemy.bullet.pos.x) && static_cast<int>(player.pos.y) == static_cast<int>(enemy.bullet.pos.y));
}

bool checkPlayerAndEnemyCollision(Player& player, Enemy& enemy) { // checks if both the coordinates of enemy and player are equal (checks collision)
    return (enemy.isActive && static_cast<int>(enemy.pos.x) == static_cast<int>(player.pos.x) && static_cast<int>(enemy.pos.y) == static_cast<int>(player.pos.y));
}

bool checkPlayerAndPowerUpCollision(Player& player, PowerUp& power_up) { // checks if both the coordinates of powerup and player are equal (checks collision)
    return (power_up.isActive && static_cast<int>(power_up.pos.x) == static_cast<int>(player.pos.x) && static_cast<int>(power_up.pos.y) == static_cast<int>(player.pos.y));
}

void EnemyKilled(Player& player, Enemy& enemy) {
    static bool kill_saved = false;
    if (!kill_saved) {
        player.kills++;
        player.score += 50;
        kill_saved = true;
    }
}

void createEnemyBullet(Player& player, Enemy& enemy) { // creates enemy bullet 
    if(enemy.isActive) {
        enemy.bullet.isActive = true;
        enemy.bullet.pos = enemy.pos;
        if ((int)(enemy.pos.y) > (int)(player.pos.y)) enemy.bullet.direction = 'w';
        else if ((int)(enemy.pos.x) > (int)(player.pos.x)) enemy.bullet.direction = 'a';
        else if ((int)(enemy.pos.y) < (int)(player.pos.y)) enemy.bullet.direction = 's';
        else if ((int)(enemy.pos.x) < (int)(player.pos.x)) enemy.bullet.direction = 'd';
    }
}

void moveEnemyBullet(Enemy& enemy) { // moves enemy bullet one field per tick
    if (enemy.bullet.isActive) {
        if (enemy.bullet.direction == 'w' && enemy.bullet.pos.y > 0) enemy.bullet.pos.y -= 0.5;
        else if (enemy.bullet.direction == 'a' && enemy.bullet.pos.x > 0) enemy.bullet.pos.x -= 0.5;
        else if (enemy.bullet.direction == 's' && enemy.bullet.pos.y < MAP_HEIGHT - 1) enemy.bullet.pos.y += 0.5;
        else if (enemy.bullet.direction == 'd' && enemy.bullet.pos.x < MAP_WIDTH - 1) enemy.bullet.pos.x += 0.5;
        else enemy.bullet.isActive = false;
    }
}

bool playerNearEnemy(Player& player, Enemy& enemy) {
    if ((int)(player.pos.x) == (int)(enemy.pos.x) && abs(player.pos.y - enemy.pos.y) <= 14) {
        return true; // same column
    }
    else if ((int)(player.pos.y) == (int)(enemy.pos.y) && abs(player.pos.x - enemy.pos.x) <= 14) {
        return true; // same row
    }
    return false;
}

void drawMap(char map[MAP_HEIGHT][MAP_WIDTH], Player& player, vector<Bullet>& bullets, Rocket& rocket, vector<Enemy>& enemies, PowerUp& healing_power_up, PowerUp& ammo_power_up, Data& data, bool power_up_and_player_collision, unsigned int bullet_counter, unsigned int rocket_counter) {
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
                for (Bullet& bullet : bullets) {
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

            if (!drawn && rocket.isActive && i == static_cast<int>(rocket.pos.y) && j == static_cast<int>(rocket.pos.x)) { // draw player's rockets
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
                for (Enemy& enemy : enemies) {
                    if (enemy.isActive && i == static_cast<int>(enemy.pos.y) && j == static_cast<int>(enemy.pos.x)) {
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
                for (auto enemy : enemies) {
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
            if (!drawn && healing_power_up.isActive && i == static_cast<int>(healing_power_up.pos.y) && j == static_cast<int>(healing_power_up.pos.x)) { // healing
                setConsoleColor(9);  // set blue color for powerup
                wcout << '$';
                setConsoleColor(7); // reset to white color
                drawn = true;
            }
            if (!drawn && ammo_power_up.isActive && i == static_cast<int>(ammo_power_up.pos.y) && j == static_cast<int>(ammo_power_up.pos.x)) { // ammo
                setConsoleColor(9); // set blue color for powerup
                wcout << '*';
                setConsoleColor(7); // reset to white color
                drawn = true;
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
    wcout << "Bullets fired: " << bullet_counter << endl;
    wcout << "Bullets: " << player.bullets << endl;
    wcout << "Missiles fired: " << rocket_counter << endl;
    wcout << "Missiles: " << player.rockets << endl;
    wcout << "Score: " << player.score << endl;
    wcout << "Health: ";

    // draw player HP {
    for (int i = 0; i < player.hp / 10; i++) wcout << L"█";
    for (int i = 0; i < 10 - player.hp / 10; i++) wcout << L"░";
    setConsoleColor(8);

    // display messages
    static unsigned int enemy_killed_message_counter = 0; 
    for (auto enemy : enemies) {
        if (enemy.hp <= 0) {
            enemy_killed_message_counter++;
            if (enemy_killed_message_counter < 10) {
                if (enemy_killed_message_counter % 2) {
                    setConsoleColor(4);
                    wcout << data.message1;
                    setConsoleColor(7);
                }
            }
        }
        else enemy_killed_message_counter = 0;
    }


    static unsigned int power_up_collected_message_counter = 0;
    if (power_up_and_player_collision) power_up_collected_message_counter++;
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
    PowerUp healing_power_up;
    PowerUp ammo_power_up;
    Data data;
    char key;
    unsigned int bullet_counter;
    unsigned int rocket_counter;
    bool power_up_spawn_condition;
    bool power_up_collision;

    healing_power_up.type = 0;
    ammo_power_up.type = 1;

    setupConsole();
    char menu_option;
    while (true) {
        menu();
        menu_option = _getch();
        switch (menu_option) {
        case '1':
            resetPlayerStats(player, bullets, enemies);
            healing_power_up.isActive = false;
            ammo_power_up.isActive = false;
            power_up_spawn_condition = false;
            power_up_collision = false;
            rocket.isActive = false;
            data.display_no_bullets = false;
            bullet_counter = 0;
            rocket_counter = 0;     
            player.cause_of_death = 0;
            initMap(map);
            for (size_t i = 0; i < 10; i++)
                createEnemy(player, enemies);
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
                            createBullet(player, bullets); // shoot a bullet
                            player.bullets--;
                            bullet_counter++;
                        }
                    }
                    if ((key == 'r' || key == 'R') && !(rocket.isActive)) {
                        if (player.rockets == 0) {
                            data.display_no_rockets = true;
                        }
                        else {
                            createRocket(player, rocket); // shoot a rocket
                            player.rockets--;
                            rocket_counter++;
                        }
                    }
                }
                for (auto& enemy : enemies) {
                    if (checkPlayerBulletAndEnemyCollision(enemy, bullets)) {
                        enemy.isHit = true;
                        enemy.hp -= 30;
                        player.score += 10;
                        if (enemy.hp <= 0) power_up_spawn_condition = true;
                    }
                    if (checkPlayerRocketAndEnemyCollision(enemy, rocket)) {
                        rocket.isActive = false;
                        enemy.hp -= 100;
                        if (enemy.hp <= 0) power_up_spawn_condition = true;
                    }
                    if (enemy.hp <= 0) {
                        enemy.isActive = false;
                        enemy.bullet.isActive = false;
                        EnemyKilled(player, enemy);
                        if (power_up_spawn_condition == true) {
                            int temp_rand = rand() % 4;
                            if (temp_rand == 0 || temp_rand == 2) createPowerUp(player, healing_power_up);
                            else if (temp_rand == 1) createPowerUp(player, ammo_power_up);
                            power_up_spawn_condition = false;
                        }
                    }
                    moveEnemy(player, enemy);
                    moveEnemyBullet(enemy);
                    if (playerNearEnemy(player, enemy) && !enemy.bullet.isActive) createEnemyBullet(player, enemy);
                    if (checkEnemyBulletAndPlayerCollision(player, enemy)) {
                        player.hp -= 20;
                        enemy.bullet.isActive = false;
                    }
                    if (checkPlayerAndEnemyCollision(player, enemy)) {
                        player.cause_of_death = 1;
                        gameOver(player, bullets, enemies);
                        break;
                    }
                    enemy.isHit = false;
                }
                moveBullet(bullets);
                moveRocket(rocket);
                if (player.hp <= 0) {
                    gameOver(player, bullets, enemies);
                    break;
                }
                if ((healing_power_up.isActive) && checkPlayerAndPowerUpCollision(player, healing_power_up)) {
                    player.hp += 20;
                    power_up_collision = true;
                    healing_power_up.isActive = false;
                }
                if ((ammo_power_up.isActive) && checkPlayerAndPowerUpCollision(player, ammo_power_up)) {

                    player.bullets += rand() % 10 + 11;
                    player.rockets += rand() % 4 + 2;
                    power_up_collision = true;
                    ammo_power_up.isActive = false;
                }
                initMap(map);
                drawMap(map, player, bullets, rocket, enemies, healing_power_up, ammo_power_up, data, power_up_collision, bullet_counter, rocket_counter);               
                power_up_collision = false;
                data.display_no_bullets = false;
                data.display_no_rockets = false;
                if (player.hp > 100) player.hp = 100;
                Sleep(40);
            }
            break;
        case '2':
            while (1) {
                system("cls"); // displays controls
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
            system("cls"); // displays game rules 
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
            system("cls");
            wcout << "\n\n\n"; // displays information about the author
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

