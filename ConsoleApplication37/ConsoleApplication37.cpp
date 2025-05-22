#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <queue>

using namespace std;

const int SIZE = 10;
const int ENEMY_HEALTH = 200;
const int ENEMY_ATTACK = 35;
const int CRIT_CHANCE = 20;

enum Cell { EMPTY, COIN, HEALTH, DEFENSE, ATTACK, ENEMY, WALL, EXIT };

struct Item {
    string type;
    int x, y;
};

struct Enemy {
    int x, y;
    int health;
    int attack;
};

struct Player {
    int x = 0, y = 0;
    int health = 100;
    int defense = 0;
    int attack = 10;
    int coins = 0;
};

vector<vector<Cell>> field(SIZE, vector<Cell>(SIZE, EMPTY));
Player player;
vector<Item> items;
vector<Enemy> enemies;
bool inMaze = false;
bool level3 = false;
int steps = 0;

void resetPlayer() {
    player.x = 0;
    player.y = 0;
    player.health = 100;
    player.defense = 0;
    player.attack = 10;
    player.coins = 0;
}

void generateField(bool isLevel3 = false) {
    srand(static_cast<unsigned int>(time(nullptr)));

    items.clear();
    enemies.clear();
    for (auto& row : field) fill(row.begin(), row.end(), EMPTY);

    auto place = [](Cell cellType, int count, string name) {
        for (int i = 0; i < count; ++i) {
            int x, y;
            do {
                x = rand() % SIZE;
                y = rand() % SIZE;
            } while (field[y][x] != EMPTY || (x == player.x && y == player.y));
            field[y][x] = cellType;
            items.push_back({ name, x, y });
        }
        };

    place(COIN, 5, "coin");
    place(HEALTH, 3, "health");
    place(DEFENSE, 2, "defense");
    place(ATTACK, 3, "attack");

    for (int i = 0; i < 5; ++i) {
        int x, y;
        do {
            x = rand() % SIZE;
            y = rand() % SIZE;
        } while (field[y][x] != EMPTY || (x == player.x && y == player.y));
        field[y][x] = ENEMY;
        enemies.push_back({ x, y, isLevel3 ? ENEMY_HEALTH * 4 : ENEMY_HEALTH, isLevel3 ? ENEMY_ATTACK * 4 : ENEMY_ATTACK });
    }
}

void printField() {
    for (int y = 0; y < SIZE; ++y) {
        for (int x = 0; x < SIZE; ++x) {
            if (x == player.x && y == player.y) {
                cout << 'P';
            }
            else {
                switch (field[y][x]) {
                case EMPTY: cout << '.'; break;
                case COIN: cout << 'C'; break;
                case HEALTH: cout << 'H'; break;
                case DEFENSE: cout << 'D'; break;
                case ATTACK: cout << 'A'; break;
                case ENEMY: cout << 'E'; break;
                case WALL: cout << '#'; break;
                case EXIT: cout << 'X'; break;
                }
            }
        }
        cout << endl;
    }
    cout << "HP: " << player.health << " DEF: " << player.defense << " ATK: " << player.attack << " COINS: " << player.coins << " STEPS: " << steps << endl;
}

void dropRandomItem() {
    vector<string> types = { "coin", "health", "defense", "attack" };
    string type = types[rand() % types.size()];

    cout << "\nПісля поразки ви отримали предмет: " << type << endl;

    if (type == "coin") player.coins++;
    else if (type == "health") player.health += 10;
    else if (type == "defense") player.defense += 5;
    else if (type == "attack") player.attack += 5;
}

void fight(Enemy& enemy) {
    cout << "\nПочинається бій з ворогом!\n";
    Player temp = player;
    Enemy tempEnemy = enemy;

    while (temp.health > 0 && tempEnemy.health > 0) {
        int damage = temp.attack;
        if ((rand() % 100) < CRIT_CHANCE) damage *= 2;
        tempEnemy.health -= damage;
        cout << "Ви нанесли " << damage << " шкоди. Залишилось у ворога: " << max(0, tempEnemy.health) << endl;

        if (tempEnemy.health > 0) {
            int enemyDamage = max(0, tempEnemy.attack - temp.defense);
            temp.health -= enemyDamage;
            cout << "Ворог атакував вас на " << enemyDamage << ". Ваше здоров'я: " << max(0, temp.health) << endl;
        }
    }

    if (temp.health <= 0) {
        cout << "Ви програли бій.\n";
        dropRandomItem();
    }
    else {
        cout << "Ви перемогли ворога!\n";
        field[enemy.y][enemy.x] = EMPTY;
        enemies.erase(remove_if(enemies.begin(), enemies.end(), [&](Enemy& e) {
            return e.x == enemy.x && e.y == enemy.y;
            }), enemies.end());
    }
}

void generateMaze() {
    cout << "Переходимо на 2 рівень — лабіринт!\n";
    inMaze = true;
    vector<vector<Cell>> maze(SIZE, vector<Cell>(SIZE, WALL));
    int startX = 0, startY = 0;
    int endX = SIZE - 1, endY = SIZE - 1;

    auto isValid = [](int x, int y, const vector<vector<Cell>>& maze, vector<vector<bool>>& visited) {
        return x >= 0 && y >= 0 && x < SIZE && y < SIZE && maze[y][x] != WALL && !visited[y][x];
        };

    auto isMazeSolvable = [&](int sx, int sy, int ex, int ey, const vector<vector<Cell>>& maze) {
        vector<vector<bool>> visited(SIZE, vector<bool>(SIZE, false));
        queue<pair<int, int>> q;
        q.push({ sx, sy });
        visited[sy][sx] = true;

        int dx[] = { 0, 0, 1, -1 };
        int dy[] = { 1, -1, 0, 0 };

        while (!q.empty()) {
            pair<int, int> current = q.front(); q.pop();
            int cx = current.first, cy = current.second;

            if (cx == ex && cy == ey) return true;
            for (int i = 0; i < 4; ++i) {
                int nx = cx + dx[i], ny = cy + dy[i];
                if (isValid(nx, ny, maze, visited)) {
                    visited[ny][nx] = true;
                    q.push({ nx, ny });
                }
            }
        }

        return false;
        };

    do {
        for (int y = 0; y < SIZE; ++y)
            for (int x = 0; x < SIZE; ++x)
                maze[y][x] = (rand() % 100 < 30 ? WALL : EMPTY);
        maze[startY][startX] = EMPTY;
        maze[endY][endX] = EXIT;
    } while (!isMazeSolvable(startX, startY, endX, endY, maze));

    field = maze;
    player.x = startX;
    player.y = startY;
}

void movePlayer(char direction) {
    int dx = 0, dy = 0;
    direction = tolower(direction);
    if (direction == 'w') dy = -1;
    else if (direction == 's') dy = 1;
    else if (direction == 'a') dx = -1;
    else if (direction == 'd') dx = 1;
    else return;

    int newX = player.x + dx;
    int newY = player.y + dy;

    if (newX >= 0 && newX < SIZE && newY >= 0 && newY < SIZE && field[newY][newX] != WALL) {
        player.x = newX;
        player.y = newY;
        steps++;

        if (inMaze && field[newY][newX] == EXIT) {
            cout << "\nВи пройшли лабіринт!\n";
            inMaze = false;
            level3 = true;
            generateField(true);
            return;
        }

        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->x == player.x && it->y == player.y) {
                if (it->type == "coin") player.coins++;
                else if (it->type == "health") player.health += 10;
                else if (it->type == "defense") player.defense += 5;
                else if (it->type == "attack") player.attack += 5;
                field[player.y][player.x] = EMPTY;
                items.erase(it);
                return;
            }
        }

        for (auto& enemy : enemies) {
            if (enemy.x == player.x && enemy.y == player.y) {
                fight(enemy);
                return;
            }
        }
    }
}

int main() {
    system("chcp 1251 > nul");
    char choice;
    cout << "(N) Нова гра або (L) Завантажити: ";
    cin >> choice;
    choice = tolower(choice);

    if (choice == 'n') {
        resetPlayer();
        steps = 0;
        inMaze = false;
        level3 = false;
        generateField();
    }
    else if (choice == 'l') {
        cout << "Завантаження гри не реалізовано.\n";
        return 0;
    }
    else {
        cout << "Невірний вибір.\n";
        return 0;
    }

    while (true) {
        if (!inMaze && enemies.empty() && !level3) {
            generateMaze();
        }
        else if (!inMaze && enemies.empty() && level3) {
            cout << "\nГру пройдено за " << steps << " кроків. Вітаємо!\n";
            break;
        }

        printField();
        cout << "W/A/S/D для руху, Q для виходу: ";
        char cmd;
        cin >> cmd;
        if (tolower(cmd) == 'q') break;
        movePlayer(cmd);
    }
    return 0;
}