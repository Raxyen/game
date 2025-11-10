#include <iostream>
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

const int MAP_HEIGHT = 15;
const int MAP_WIDTH = 30;

using namespace std;

// sound

void playEnemyHitSound() { thread([] { Beep(300, 50); }).detach(); }

void playEnemyKilledSound() {
    thread([] {
        Beep(300, 50);
        Beep(400, 50);
        }).detach();
}

void playBulletSound() {
    thread([] {
        Beep(1000, 30);
        }).detach();
}

void playRocketSound() { thread([] { Beep(60, 1000); }).detach(); }

void playPowerUpPickedSound() {
    thread([] {
        Beep(400, 50);
        Beep(500, 50);
        Beep(400, 50);
        Beep(500, 50);
        }).detach();
}

class Position {
private:
    float x, y;
public:
    Position() : x(0), y(0) {} // default
    Position(float p_x, float p_y) : x(p_x), y(p_y) {}

    inline float getX() { return x; }
    inline float getY() { return y; }
    inline void moveX(float dx) { this->x += dx; }
    inline void moveY(float dy) { this->y += dy; }

    Position& operator=(const Position& other) {
        if (this != &other) {
            this->x = other.x;
            this->y = other.y;
        }
        return *this;
    }
};

class Projectile {
protected:
    Position pos;
    unsigned int damage;
    float speed;
    bool is_enemy;
    bool is_active;
    char direction;
public:
    Projectile() {} // default
    Projectile(Position p_pos, unsigned int p_damage, float p_speed, bool p_is_enemy, char p_direction)
        : pos(p_pos), damage(p_damage), speed(p_speed), is_enemy(p_is_enemy), direction(p_direction), is_active(true) {}

    inline Position getPosition() { return pos; }
    inline void setPosition(Position pos) { this->pos = pos; }

    inline char getDirection() { return direction; }

    void activate() { is_active = true; }
    void deactivate() { is_active = false; }

    bool isActive() { return is_active; }
    bool isEnemy() { return is_enemy; }

    unsigned int getDamage() { return this->damage; }

    void move() { // moves projectile
        if (this->is_active) {
            if (direction == 'w' && pos.getY() > 0) pos.moveY(-speed);
            else if (direction == 'a' && pos.getX() > 0) pos.moveX(-speed);
            else if (direction == 's' && pos.getY() < MAP_HEIGHT - 1) pos.moveY(speed);
            else if (direction == 'd' && pos.getX() < MAP_WIDTH - 1) pos.moveX(speed);
            else deactivate();
        }
    }
};

class Bullet : public Projectile {
public:
    Bullet() = default;

    Bullet(Position p_pos, unsigned int p_damage, bool p_is_enemy, char p_direction)
        : Projectile(p_pos, p_damage, 1.0f, p_is_enemy, p_direction) {}
};

class Rocket : public Projectile {
public:
    Rocket(Position p_pos, unsigned int p_damage, bool p_is_enemy, char p_direction)
        : Projectile(p_pos, p_damage, 0.7f, p_is_enemy, p_direction) {}
};

class Player {
private:
    Position pos;
    vector<Bullet> bullets;
    char direction;
    int hp;
    unsigned int kills;
    unsigned int score;
    unsigned int bullets_count;
    unsigned int rockets_count;
    unsigned int bullets_fired;
    unsigned int rockets_fired;
    int cause_of_death;
public:
    Player() { reset(); }

    inline Position getPosition() { return pos; }
    inline void setPosition(Position pos) { this->pos = pos; }
    
    inline vector<Bullet>& getBullets() { return bullets; }
    inline const vector<Bullet>& getBullets() const { return bullets; }

    inline char getDirection() { return direction; }
    inline void setDirection(char direction) { this->direction = direction; }

    inline int getHP() { return hp; }

    inline unsigned int getBulletsCount() { return bullets_count; }
    inline unsigned int getRocketsCount() { return rockets_count; }

    inline void setCauseOfDeath(int cause) { this->cause_of_death = cause; }

    void modifyHealth(int health) { this->hp += health; }
    void modifyScore(int score) { this->score += score; }

    void modifyBulletsCount(int bullets_count) { this->bullets_count += bullets_count; }
    void modifyRocketsCount(int rockets_count) { this->rockets_count += rockets_count; }

    void modifyRocketsFired(int rockets_fired) { this->rockets_fired += rockets_fired; }

    void kill() { kills++; }

    void normalizeHP() { if (hp > 100) hp = 100; }

    void move(float dx, float dy) {
        pos.moveX(dx);
        pos.moveY(dy);
    }

    void shoot() {
        Bullet bullet(getPosition(), 30, false, getDirection());
        bullets.emplace_back(bullet);
        bullets_count--;
        bullets_fired++;
        thread(playBulletSound).detach();
    }

    void reset() {
        direction = 'w';
        kills = 0;
        score = 0;
        hp = 100;
        bullets_count = 30;
        rockets_count = 2;
        bullets_fired = 0;
        rockets_fired = 0;
        cause_of_death = 0;
        setPosition(Position(10.0f, 5.0f));
    }

    void info() {
        wcout << "Enemy kills: " << kills << endl;
        wcout << "Bullets fired: " << bullets_fired << endl;
        wcout << "Bullets: " << bullets_count << endl;
        wcout << "Missiles fired: " << rockets_fired << endl;
        wcout << "Missiles: " << rockets_count << endl;
        wcout << "Score: " << score << endl;
        wcout << "Health: ";
    }

    void finalInfo() {
        wcout << "  * Score: " << score << "\n  * Enemy kills: " << kills << L"\n  * Killed by: ";
        cause_of_death == 1 ? wcout << L"Enemy crashing you\n\n" : wcout << L"Enemy bullet\n\n";
        wcout << L"   Press any key to proceed to the menu.\n";
    }
};

class Enemy {
private:
    Position pos;
    Bullet bullet;
    float speed;
    char texture;
    bool is_hit;
    char direction;
    int hp;
public:
    Enemy(char p_direction, Player& player) :
        is_hit(false), direction(p_direction), hp(100), speed(0.2f), texture('X') {
        unsigned int x = rand() % (MAP_WIDTH - 2) + 1;
        unsigned int y = rand() % (MAP_HEIGHT - 2) + 1;
        if (x != player.getPosition().getX() && y != player.getPosition().getY()) {
            pos = Position(x, y);
        }
        else {
            pos = Position(x + rand() % 3 - 2, y + rand() % 3 - 2);
        }
        bullet = Bullet(getPosition(), 20, true, ' ');
        bullet.deactivate();
    }

    inline Position getPosition() { return pos; }
    inline Bullet& getBullet() { return bullet; }
    inline const Bullet& getBullet() const { return bullet; }
    inline void setBullet(Bullet bullet) { this->bullet = bullet; }
    inline float getSpeed() { return speed; }
    inline int getHP() { return hp; }
    inline char getTexture() { return texture; }

    bool isHit() { return is_hit; }
    void setIsHit(bool is_hit) { this->is_hit = is_hit; }

    void modifyHealth(int health) { this->hp += health; }

    void move(float dx, float dy) {
        pos.moveX(dx);
        pos.moveY(dy);
    }

    void shoot(Player& player) {
        float dx = player.getPosition().getX() - getPosition().getX();
        float dy = player.getPosition().getY() - getPosition().getY();

        char temp_dir;
        if (fabs(dx) > fabs(dy))
            temp_dir = (dx > 0) ? 'd' : 'a'; // horizontally
        else
            temp_dir = (dy > 0) ? 's' : 'w'; // vertically

        bullet = Bullet(getPosition(), 20, true, temp_dir);
        bullet.activate();
    }
};

class PowerUp {
private:
    Position pos;
    int type;
    char texture;
public:
    PowerUp(Player& player, int p_type) : type(p_type) {
        texture = (type == 1) ? '*' : '$';
        unsigned int x, y;
        do {
            x = rand() % (MAP_WIDTH - 2) + 1;
            y = rand() % (MAP_HEIGHT - 2) + 1;
        } while (x == static_cast<int>(player.getPosition().getX()) && y == static_cast<int>(player.getPosition().getY()));
        pos = Position(static_cast<float>(x), static_cast<float>(y));
    }

    inline Position getPosition() { return pos; }
    inline char getTexture() { return texture; }
    inline int getType() { return type; }
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

void resetPlayerStats(Player& player, vector<Enemy>& enemies) {

    for (auto& bullet : player.getBullets()) 
        bullet.deactivate();
    for (auto& enemy : enemies)
        enemy.getBullet().deactivate();
}

void gameOver(Player& player, vector<Enemy>& enemies, vector<PowerUp>& powerups) {
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
    player.getBullets().clear();
    powerups.clear();
    Sleep(2000);
    _getch();
    resetPlayerStats(player, enemies);
    setConsoleColor(7);
}

void initMap(char map[MAP_HEIGHT][MAP_WIDTH]) { // initializes map fields
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) map[i][j] = ' ';
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

bool checkPlayerBulletAndEnemyCollision(Player& player, Enemy& enemy) {
    for (Bullet& bullet : player.getBullets()) {
        if (bullet.isActive() && !bullet.isEnemy()) {
            float dx = fabs(enemy.getPosition().getX() - bullet.getPosition().getX());
            float dy = fabs(enemy.getPosition().getY() - bullet.getPosition().getY());

            // solve tunneling problem
            if (dx < 0.6f && dy < 0.6f) {
                bullet.deactivate();
                return true;
            }
        }
    }
    return false;
}

bool checkPlayerRocketAndEnemyCollision(Enemy& enemy, Rocket& rocket) { // checks if both the coordinates of enemy and player's rocket are equal (checks collision)
    return (rocket.isActive() && static_cast<int>(enemy.getPosition().getX()) == static_cast<int>(rocket.getPosition().getX()) && static_cast<int>(enemy.getPosition().getY()) == static_cast<int>(rocket.getPosition().getY()));
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

//checks whether the player distance from enemy is least or equal than 14 
//and if the player and enemy are in the same line to make a shot possible

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
void drawMap(char map[MAP_HEIGHT][MAP_WIDTH], Player& player, Rocket& rocket, vector<Enemy>& enemies, vector<PowerUp>& powerups, Data& data) {
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

            if (!drawn && i == static_cast<int>(player.getPosition().getY()) && j == static_cast<int>(player.getPosition().getX())) {
                setConsoleColor(2); // set green color for player
                wcout << '@'; // draw player 
                setConsoleColor(7); // reset to white color
                drawn = true;
            }

            if (!drawn) { // draw player's bullets 
                for (auto& bullet : player.getBullets()) {
                    if (bullet.isActive() && i == static_cast<int>(bullet.getPosition().getY()) && j == static_cast<int>(bullet.getPosition().getX())) {
                        setConsoleColor(8); // set gray color for bullet
                        if (bullet.getDirection() == 'a' || bullet.getDirection() == 'd') wcout << L"─"; // horizontal bullet
                        else if (bullet.getDirection() == 'w' || bullet.getDirection() == 's') wcout << L"│"; // vertical bullet
                        setConsoleColor(7); // reset to white color
                        drawn = true;
                        break;
                    }
                }
            }

            // draw player's rocket
            if (!drawn && rocket.isActive() && i == static_cast<int>(rocket.getPosition().getY()) && j == static_cast<int>(rocket.getPosition().getX())) {
                setConsoleColor(8); // set gray color for rocket
                if (rocket.getDirection() == 'w') wcout << L"↑"; // rocket with up direction
                else if (rocket.getDirection() == 'a') wcout << L"←"; // rocket with left direction
                else if (rocket.getDirection() == 's') wcout << L"↓"; // rocket with down direction
                else if (rocket.getDirection() == 'd') wcout << L"→"; // rocket with right direction                
                setConsoleColor(7); // reset to white color
                drawn = true;
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
                        if (enemy.getBullet().getDirection() == 'a' || enemy.getBullet().getDirection() == 'd') wcout << L"─"; // horizontal bullet
                        else if (enemy.getBullet().getDirection() == 'w' || enemy.getBullet().getDirection() == 's') wcout << L"│"; // vertical bullet
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
    player.info();

    // draw player HP {
    for (int i = 0; i < player.getHP() / 10; i++) wcout << L"█";
    for (int i = 0; i < 10 - player.getHP() / 10; i++) wcout << L"░";
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

    char map[MAP_HEIGHT][MAP_WIDTH];
    Player player;
    Rocket rocket(Position(), 100, false, ' ');
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
            player.reset();

            data.display_no_bullets = false;            
            data.display_no_rockets = false;
            data.display_enemy_killed = false;
            data.powerup_and_player_collision = false;
            data.powerup_spawn_condition = false;

            initMap(map);
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
                            player.shoot(); // shoot a bullet
                        }
                    }
                    if ((key == 'r' || key == 'R') && !(rocket.isActive())) {
                        if (player.getRocketsCount() == 0) {
                            data.display_no_rockets = true;
                        }
                        else {
                            rocket = Rocket(player.getPosition(), 100, false, player.getDirection()); // shoot a rocket
                            rocket.activate();
                            player.modifyRocketsCount(-1);
                            thread(playRocketSound).detach();
                            player.modifyRocketsFired(1);
                        }
                    }
                    // test only
                    if (key == 't' || key == 'T')
                        spawnEnemies(player, enemies, 4);
                }

                // move bullets
                for (auto& bullet : player.getBullets()) {
                    bullet.move();
                }

                // iterate enemies vector
                for (auto& enemy : enemies) {
                    if (checkPlayerBulletAndEnemyCollision(player, enemy)) {
                        thread(playEnemyHitSound).detach();
                        enemy.setIsHit(true);
                        enemy.modifyHealth(-30);
                        player.modifyScore(10);
                        if (enemy.getHP() <= 0) 
                            data.powerup_spawn_condition = true;
                    }
                    if (checkPlayerRocketAndEnemyCollision(enemy, rocket)) {
                        thread(playEnemyHitSound).detach();
                        rocket.deactivate();
                        enemy.modifyHealth(-static_cast<int>(rocket.getDamage()));
                    }
                    if (enemy.getHP() <= 0) {
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
                    if (playerNearEnemy(player, enemy) && !enemy.getBullet().isActive()) enemy.shoot(player);
                    if (checkEnemyBulletAndPlayerCollision(player, enemy)) {
                        player.modifyHealth(-20);
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
                    gameOver(player, enemies, powerups);
                    break;
                }

                // move entities
                moveEnemies(player, enemies);
                rocket.move();

                // draw map and set some game data false
                drawMap(map, player, rocket, enemies, powerups, data);   

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

