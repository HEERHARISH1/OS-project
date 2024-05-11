#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include <string>
#include <atomic> // for std::atomic

constexpr int LINE_THICKNESS = 2; // Reduced thickness of the maze lines
constexpr int MAP_WIDTH = 41;
constexpr int MAP_HEIGHT = 22;
constexpr int TILE_SIZE = 30;

// Define symbols for game board elements
enum class CellType { Wall, Path, Pellet, PowerPellet, Pacman, Ghost };

struct Cell {
    std::atomic<CellType> type;
    std::atomic<char> character; // Character representation on the map
};

std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT> gameBoard;

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

int main() {
    // Initialize all cells to paths and locate Pacman
    int pacmanX = 1, pacmanY = 1; // Initial Pacman position
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (map_sketch[y][x] == 'P') {
                pacmanX = x;
                pacmanY = y;
                gameBoard[y][x].type = CellType::Pacman;
            } else if (map_sketch[y][x] == '#') {
                gameBoard[y][x].type = CellType::Wall;
            } else if (map_sketch[y][x] == '1' || map_sketch[y][x] == '2' || map_sketch[y][x] == '3') {
                gameBoard[y][x].type = CellType::Ghost;
            } else {
                gameBoard[y][x].type = CellType::Path;
            }
            gameBoard[y][x].character = map_sketch[y][x];
        }
    }

    sf::RenderWindow window(sf::VideoMode(MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE), "SFML Maze Game");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::W:
                        if (pacmanY > 0 && gameBoard[pacmanY - 1][pacmanX].type != CellType::Wall) pacmanY--;
                        break;
                    case sf::Keyboard::S:
                        if (pacmanY < MAP_HEIGHT - 1 && gameBoard[pacmanY + 1][pacmanX].type != CellType::Wall) pacmanY++;
                        break;
                    case sf::Keyboard::A:
                        if (pacmanX > 0 && gameBoard[pacmanY][pacmanX - 1].type != CellType::Wall) pacmanX--;
                        break;
                    case sf::Keyboard::D:
                        if (pacmanX < MAP_WIDTH - 1 && gameBoard[pacmanY][pacmanX + 1].type != CellType::Wall) pacmanX++;
                        break;
                    default:
                        break;
                }
            }
        }

        window.clear(sf::Color::Black); // Set background to black

        // Draw the game elements
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                if (gameBoard[y][x].type == CellType::Wall) {
                    sf::RectangleShape wall(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                    wall.setFillColor(sf::Color::Magenta);
                    wall.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                    window.draw(wall);
                } else if (gameBoard[y][x].type == CellType::Pacman) {
                    drawPacman(window, pacmanX, pacmanY); // Redraw Pacman at the new location
                } else if (gameBoard[y][x].type == CellType::Ghost) {
                    sf::Color ghostColor = getGhostColor(gameBoard[y][x].character);
                    drawGhost(window, x, y, ghostColor);
                }
            }
        }

        window.display();
    }

    return 0;
}

