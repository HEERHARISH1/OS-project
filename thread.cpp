#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <atomic>
#include <random>
#include <mutex>
#include <pthread.h>
#include <X11/Xlib.h>  // Include Xlib for XInitThreads


// Constants
constexpr int LINE_THICKNESS = 2;
constexpr int MAP_WIDTH = 41;
constexpr int MAP_HEIGHT = 22;
constexpr int TILE_SIZE = 30;
constexpr int MAZE_WIDTH = 21;
int totalPellets = 152;  // Initialize with the total number of pellets
int score = 0;
int lives = 3; // Initialize lives with 3
int pacmanX = 9, pacmanY = 16;
sf::Text scoreText;
sf::Text livesText;
sf::Keyboard::Key lastDirection = sf::Keyboard::Right; // Store last direction of movement

// Define symbols for game board elements
enum class CellType { Wall, Path, Pellet, PowerPellet, Pacman, Ghost };

struct Cell {
    std::atomic<CellType> type;
    std::atomic<char> character; // Character representation on the map
};

std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> gameBoard;

struct Ghost {
    int x, y;
    int dx = 0, dy = 0; // Store direction as well
    char number;
};

std::vector<Ghost> ghosts = {
    {10, 10, '1'}, // Adjust initial positions as needed
    {2, 10, '2'},
    {10, 15, '3'}
};

std::random_device rd;
std::mt19937 eng(rd());

// Mutexes for thread synchronization
std::mutex gameBoardMutex;
std::mutex scoreMutex;
std::mutex livesMutex;

std::array<std::string, MAP_HEIGHT> map_sketch = {
    " ###################                     ",
    " #........#........#                     ",
    " #o##.###.#.###.##o#                     ",
    " #.................#                     ",
    " #.##.#.#####.#.##.#                     ",
    " #....#...#...#....#                     ",
    " ####.### # ###.####                     ",
    "    #.#   0   #.#                        ",
    "#####.# #   # #.#####                    ",
    "     .  #   #  .                         ",
    "     .  #   #  .                         ",
    "#####.# ##### #.#####                    ",
    "    #.#       #.#                        ",
    " ####.# ##### #.####                     ",
    " #........#........#                     ",
    " #.##.###.#.###.##.#                     ",
    " #o.#.....P.....#.o#                     ",
    " ##.#.###.#.###.#.##                     ",
    " #........#........#                     ",
    " #.##.###.#.###.##.#                     ",
    " #o.................#                     ",
    " ###################                     ",
};

// Function prototypes
void handlePacmanMovement(sf::Keyboard::Key &lastDirection, int &pacmanX, int &pacmanY, std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> &gameBoard, int &score);
void checkPacmanCollision(int &pacmanX, int &pacmanY, std::vector<Ghost> &ghosts, int &lives);
void drawGameOverScreen(sf::RenderWindow &window, int finalScore);
void drawYouWonScreen(sf::RenderWindow &window, int finalScore);
void drawGameElements(sf::RenderWindow &window, int x, int y, std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> &gameBoard);
void moveGhostRandomly(Ghost &ghost, std::mt19937 &eng);
sf::Color getGhostColor(char number);
void drawGhost(sf::RenderWindow &window, int x, int y, sf::Color color);
void drawPacman(sf::RenderWindow& window, int x, int y);

void *inputHandlingThread(void *arg);
void *gameStateUpdateThread(void *arg);
void *renderingThread(void *arg);

int main() {
    // Initialize X11 threading
    XInitThreads();

    // Initialize the game board
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            switch (map_sketch[y][x]) {
                case '#':
                    gameBoard[y][x].type = CellType::Wall;
                    break;
                case '.':
                    gameBoard[y][x].type = CellType::Pellet;
                    break;
                case 'o':
                    gameBoard[y][x].type = CellType::PowerPellet;
                    break;
                case 'P':
                    gameBoard[y][x].type = CellType::Pacman;
                    pacmanX = x;
                    pacmanY = y;
                    break;
                default:
                    gameBoard[y][x].type = CellType::Path;
                    break;
            }
        }
    }

    // Load font
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font arial.ttf. Ensure the file is in the correct directory." << std::endl;
        return -1;
    }

    // Initialize score and lives text
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 10);

    livesText.setFont(font);
    livesText.setCharacterSize(24);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition(10, 40);

    // Create threads
    pthread_t inputThread, gameStateThread, renderThread;

    pthread_create(&inputThread, NULL, inputHandlingThread, NULL);
    pthread_create(&gameStateThread, NULL, gameStateUpdateThread, NULL);
    pthread_create(&renderThread, NULL, renderingThread, NULL);

    // Main game loop
    while (true) {
        if (totalPellets == 0 || lives <= 0) {
            break;
        }
    }

    // Join threads
    pthread_join(inputThread, NULL);
    pthread_join(gameStateThread, NULL);
    pthread_join(renderThread, NULL);

    return 0;
}

void *inputHandlingThread(void *arg) {
    while (true) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) lastDirection = sf::Keyboard::Left;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) lastDirection = sf::Keyboard::Right;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) lastDirection = sf::Keyboard::Up;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) lastDirection = sf::Keyboard::Down;

        sf::sleep(sf::milliseconds(100)); // To prevent high CPU usage
    }
    return NULL;
}

void *gameStateUpdateThread(void *arg) {
    while (lives > 0) {
        {
            std::lock_guard<std::mutex> lock(gameBoardMutex);
            handlePacmanMovement(lastDirection, pacmanX, pacmanY, gameBoard, score);
            checkPacmanCollision(pacmanX, pacmanY, ghosts, lives);

            for (auto &ghost : ghosts) {
                moveGhostRandomly(ghost, eng);
            }
        }

        sf::sleep(sf::milliseconds(200)); // Update game state periodically
    }
    return NULL;
}

void *renderingThread(void *arg) {
    sf::RenderWindow window(sf::VideoMode(MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE), "SFML Maze Game");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);

        {
            std::lock_guard<std::mutex> lock(gameBoardMutex);
            for (int y = 0; y < MAP_HEIGHT; ++y) {
                for (int x = 0; x < MAP_WIDTH; ++x) {
                    drawGameElements(window, x, y, gameBoard);
                }
            }

            for (const auto& ghost : ghosts) {
                sf::Color ghostColor = getGhostColor(ghost.number);
                drawGhost(window, ghost.x, ghost.y, ghostColor);
            }

            drawPacman(window, pacmanX, pacmanY);

            scoreText.setString("Score: " + std::to_string(score));
            livesText.setString("Lives: " + std::to_string(lives));
            window.draw(scoreText);
            window.draw(livesText);
        }

        if (lives <= 0) {
            drawGameOverScreen(window, score);
            window.close();
        }

        if (totalPellets == 0) {
            drawYouWonScreen(window, score);
            window.close();
        }

        window.display();
        sf::sleep(sf::milliseconds(200)); // Game speed control
    }
    return NULL;
}

void checkPacmanCollision(int &pacmanX, int &pacmanY, std::vector<Ghost> &ghosts, int &lives) {
    for (const auto& ghost : ghosts) {
        if (pacmanX == ghost.x && pacmanY == ghost.y) {
            lives--;
            if (lives > 0) {
                // Reset Pacman to initial position after collision
                gameBoard[pacmanY][pacmanX].type = CellType::Path; // Clear current position
                pacmanX = 9;
                pacmanY = 16;
                gameBoard[pacmanY][pacmanX].type = CellType::Pacman; // Set new position
            }
            break;
        }
    }
}

void handlePacmanMovement(sf::Keyboard::Key &lastDirection, int &pacmanX, int &pacmanY, std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> &gameBoard, int &score) {
    int newX = pacmanX, newY = pacmanY;
    if (lastDirection == sf::Keyboard::Left) newX--;
    if (lastDirection == sf::Keyboard::Right) newX++;
    if (lastDirection == sf::Keyboard::Up) newY--;
    if (lastDirection == sf::Keyboard::Down) newY++;

    if (gameBoard[newY][newX].type != CellType::Wall) {
        gameBoard[pacmanY][pacmanX].type = CellType::Path;
        pacmanX = newX;
        pacmanY = newY;

        if (gameBoard[pacmanY][pacmanX].type == CellType::Pellet) {
            score++;
            totalPellets--;
        } else if (gameBoard[pacmanY][pacmanX].type == CellType::PowerPellet) {
            score += 10; // Power pellet gives more points
            totalPellets--;
        }

        gameBoard[pacmanY][pacmanX].type = CellType::Pacman;
    }
}

void drawGameOverScreen(sf::RenderWindow &window, int finalScore) {
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font arial.ttf. Ensure the file is in the correct directory." << std::endl;
        return;
    }

    sf::Text gameOverText("Game Over!\nFinal Score: " + std::to_string(finalScore), font, 50);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setStyle(sf::Text::Bold);

    sf::FloatRect textRect = gameOverText.getLocalBounds();
    gameOverText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    gameOverText.setPosition(sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f));

    window.clear();
    window.draw(gameOverText);
    window.display();

    sf::sleep(sf::seconds(5)); // Display for 5 seconds
}

void drawYouWonScreen(sf::RenderWindow &window, int finalScore) {
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font arial.ttf. Ensure the file is in the correct directory." << std::endl;
        return;
    }

    sf::Text youWonText("You Won!\nFinal Score: " + std::to_string(finalScore), font, 50);
    youWonText.setFillColor(sf::Color::Green);
    youWonText.setStyle(sf::Text::Bold);

    sf::FloatRect textRect = youWonText.getLocalBounds();
    youWonText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    youWonText.setPosition(sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f));

    window.clear();
    window.draw(youWonText);
    window.display();

    sf::sleep(sf::seconds(5)); // Display for 5 seconds
}

void drawGameElements(sf::RenderWindow &window, int x, int y, std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> &gameBoard) {
    sf::RectangleShape rectangle(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    rectangle.setPosition(x * TILE_SIZE, y * TILE_SIZE);

    sf::CircleShape pellet(TILE_SIZE / 6);
    pellet.setPosition((x + 0.5f) * TILE_SIZE - pellet.getRadius(), (y + 0.5f) * TILE_SIZE - pellet.getRadius());

    sf::CircleShape powerPellet(TILE_SIZE / 3);
    powerPellet.setPosition((x + 0.5f) * TILE_SIZE - powerPellet.getRadius(), (y + 0.5f) * TILE_SIZE - powerPellet.getRadius());

    switch (gameBoard[y][x].type) {
        case CellType::Wall:
            rectangle.setFillColor(sf::Color::Blue);
            window.draw(rectangle);
            break;
        case CellType::Path:
            rectangle.setFillColor(sf::Color::Black);
            window.draw(rectangle);
            break;
        case CellType::Pellet:
            rectangle.setFillColor(sf::Color::Black);
            window.draw(rectangle);
            pellet.setFillColor(sf::Color::Yellow);
            window.draw(pellet);
            break;
        case CellType::PowerPellet:
            rectangle.setFillColor(sf::Color::Black);
            window.draw(rectangle);
            powerPellet.setFillColor(sf::Color::Magenta);
            window.draw(powerPellet);
            break;
        case CellType::Pacman: {
            rectangle.setFillColor(sf::Color::Black);
            window.draw(rectangle);
            sf::CircleShape pacman(TILE_SIZE / 3);
            pacman.setFillColor(sf::Color::Yellow);
            pacman.setPosition(x * TILE_SIZE + TILE_SIZE / 3, y * TILE_SIZE + TILE_SIZE / 3);
            window.draw(pacman);
            break;
        }
        case CellType::Ghost:
            rectangle.setFillColor(sf::Color::Black);
            window.draw(rectangle);
            // Ghost color and drawing logic should be handled elsewhere
            break;
    }
}

void drawPacman(sf::RenderWindow& window, int x, int y) {
    sf::CircleShape pacman(TILE_SIZE / 3);
    pacman.setFillColor(sf::Color::Yellow);
    pacman.setPosition(x * TILE_SIZE + TILE_SIZE / 3, y * TILE_SIZE + TILE_SIZE / 3);
    window.draw(pacman);
}

void moveGhostRandomly(Ghost &ghost, std::mt19937 &eng) {
    std::uniform_int_distribution<> distr(0, 3);

    int newX = ghost.x + ghost.dx;
    int newY = ghost.y + ghost.dy;

    if (newX < 0 || newX >= MAP_WIDTH || newY < 0 || newY >= MAP_HEIGHT || gameBoard[newY][newX].type == CellType::Wall) {
        while (true) {
            int direction = distr(eng);
            ghost.dx = 0;
            ghost.dy = 0;
            switch (direction) {
                case 0: ghost.dy = -1; break;
                case 1: ghost.dy = 1; break;
                case 2: ghost.dx = -1; break;
                case 3: ghost.dx = 1; break;
            }

            newX = ghost.x + ghost.dx;
            newY = ghost.y + ghost.dy;

            if (newX >= 0 && newX < MAP_WIDTH && newY >= 0 && newY < MAP_HEIGHT && gameBoard[newY][newX].type != CellType::Wall) {
                break;
            }
        }
    }

    ghost.x = newX;
    ghost.y = newY;
}

sf::Color getGhostColor(char number) {
    switch (number) {
        case '1': return sf::Color::Red;
        case '2': return sf::Color::Blue;
        case '3': return sf::Color::Cyan;
        default: return sf::Color::White;
    }
}

void drawGhost(sf::RenderWindow& window, int x, int y, sf::Color color) {
    sf::CircleShape head(TILE_SIZE / 4);
    head.setFillColor(color);
    head.setPosition(x * TILE_SIZE + TILE_SIZE / 4, y * TILE_SIZE + TILE_SIZE / 8);

    sf::RectangleShape body(sf::Vector2f(TILE_SIZE / 2, TILE_SIZE / 2));
    body.setFillColor(color);
    body.setPosition(x * TILE_SIZE + TILE_SIZE / 4, y * TILE_SIZE + TILE_SIZE * 3 / 8);

    window.draw(head);
    window.draw(body);
}

