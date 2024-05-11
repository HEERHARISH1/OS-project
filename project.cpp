#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <array>
#include <string>
#include <mutex> // for std::mutex
#include <atomic> // for std::atomic

constexpr int LINE_THICKNESS = 5;
constexpr int MAP_WIDTH = 41;
constexpr int MAP_HEIGHT = 22;
constexpr int TILE_SIZE = 30;

// Define symbols for game board elements
enum class CellType { Wall, Path, Pellet, PowerPellet, Pacman, Ghost };

struct Cell {
    std::atomic<CellType> type;
    std::atomic<char> character; // Character representation on the map
};

// Shared game board accessible to all threads
std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> gameBoard;

// Mutex for accessing shared resources
std::mutex mutex;

std::array<std::string, MAP_HEIGHT> map_sketch = {
    " ###################                     ",
    " #........#........#                     ",
    " #o##.###.#.###.##o#                     ",
    " #.................#                     ",
    " #.##.#.#####.#.##.#                     ",
    " #....#...#...#....#                     ",
    " ####.### # ###.####                     ",
    "    #.#   0   #.#                        ",
    "#####.# ##=## #.#####                    ",
    "     .  #   #  .                         ",
    "     .  #123#  .                         ",
    "#####.# ##### #.#####                    ",
    "    #.#       #.#                        ",
    " ####.# ##### #.####                     ",
    " #........#........#                     ",
    " #.##.###.#.###.##.#                     ",
    " #o.#.....P.....#.o#                     ",
    " ##.#.#.#####.#.#.##                     ",
    " #....#...#...#....#                     ",
    " #.######.#.######.#                     ",
    " #.................#                     ",
    " ###################                     "
};

sf::Color getGhostColor(char number) {
    switch (number) {
        case '1': return sf::Color::Red;
        case '2': return sf::Color::Green;
        case '3': return sf::Color::Blue;
        default:  return sf::Color::White;
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

void drawPacman(sf::RenderWindow& window, int x, int y) {
    sf::CircleShape pacman(TILE_SIZE / 3); // Smaller circle for Pac-Man
    pacman.setFillColor(sf::Color::Yellow);
    pacman.setPosition(x * TILE_SIZE + TILE_SIZE / 3, y * TILE_SIZE + TILE_SIZE / 3);

    window.draw(pacman);
}

void gameEngine(sf::RenderWindow& window) {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        mutex.lock(); // Lock access to shared resources

        window.clear(sf::Color::Black);  // Set background to black

        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                char cell = map_sketch[y][x];
                if (cell == '1' || cell == '2' || cell == '3') {
                    sf::Color ghostColor = getGhostColor(cell);
                    drawGhost(window, x, y, ghostColor);
                } else if (cell == 'P') { // Pac-Man
                    drawPacman(window, x, y);
                } else if (cell == '#') {
                    // Drawing walls
                    sf::RectangleShape wall(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                    wall.setFillColor(sf::Color::Magenta);
                    wall.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                    window.draw(wall);
                }
            }
        }

        mutex.unlock(); // Unlock access to shared resources

        window.display();
    }
}

void userInterface(sf::RenderWindow& window) {
    while (window.isOpen()) {
        // UI handling goes here
    }
}

void ghostController(char ghostNumber) {
    while (true) {
        // Ghost behavior logic goes here
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Example sleep
    }
}

int main() {
    // Initialize all cells to paths
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            gameBoard[y][x].type = CellType::Path;
            gameBoard[y][x].character = '.';
        }
    }

    // Place walls according to map sketch
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (map_sketch[y][x] == '#') {
                gameBoard[y][x].type = CellType::Wall;
                gameBoard[y][x].character = '#';
            }
        }
    }

    // Place pellets and power pellets
    // Implementation depends on your game's mechanics

    // Initialize starting positions for Pac-Man and Ghosts
    gameBoard[1][1].type = CellType::Pacman;
    gameBoard[1][1].character = 'P'; // Pac-Man starting position

    gameBoard[MAP_HEIGHT / 2][MAP_WIDTH / 2].type = CellType::Ghost;
    gameBoard[MAP_HEIGHT / 2][MAP_WIDTH / 2].character = 'G'; // Ghost starting position

    sf::RenderWindow window(sf::VideoMode(MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE), "SFML Maze Game");

    sf::Texture startScreenTexture;
    if (!startScreenTexture.loadFromFile("startpic.png")) {
        std::cout << "Failed to load start screen texture." << std::endl;
        return -1;
    }

    sf::Sprite startScreen(startScreenTexture);

    bool startGame = false;
    while (window.isOpen() && !startGame) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
                startGame = true;
        }

        window.clear();
        window.draw(startScreen);
        window.display();
    }

    // Launching threads
    std::thread engineThread(gameEngine, std::ref(window));
    std::thread uiThread(userInterface, std::ref(window));
    std::thread ghost1Thread(ghostController, '1');
    std::thread ghost2Thread(ghostController, '2');
    std::thread ghost3Thread(ghostController, '3');

    // Joining threads
    engineThread.join();
    uiThread.join();
    ghost1Thread.join();
    ghost2Thread.join();
    ghost3Thread.join();

    return 0;
}

