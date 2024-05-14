#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <atomic>
#include <random>
#include <thread>

// Constants
constexpr int LINE_THICKNESS = 2;
constexpr int MAP_WIDTH = 41;
constexpr int MAP_HEIGHT = 22;
constexpr int TILE_SIZE = 30;
constexpr int MAZE_WIDTH = 21;
int totalPellets = 152;  // Initialize with the total number of pellets
int score = 0;
int lives = 3; // Initialize lives with 3
sf::Text scoreText;
sf::Text livesText;

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


void moveGhostRandomly(Ghost &ghost, std::mt19937 &eng) {
    std::uniform_int_distribution<> distr(0, 3); // Define the range for direction

    int newX = ghost.x + ghost.dx;
    int newY = ghost.y + ghost.dy;

    // Check if the new position is a wall or out of bounds, then choose a new direction
    if (newX < 0 || newX >= MAP_WIDTH || newY < 0 || newY >= MAP_HEIGHT || gameBoard[newY][newX].type == CellType::Wall) {
        while (true) {
            int direction = distr(eng); // 0=up, 1=down, 2=left, 3=right
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

            // Check again for a valid new direction
            if (newX >= 0 && newX < MAP_WIDTH && newY >= 0 && newY < MAP_HEIGHT && gameBoard[newY][newX].type != CellType::Wall) {
                break; // Found a valid direction, exit the loop
            }
        }
    }

    // Update ghost position
    ghost.x = newX;
    ghost.y = newY;
}

void drawPellet(sf::RenderWindow& window, int x, int y) {
    sf::CircleShape pellet(TILE_SIZE / 8); // Smaller circle for pellets
    pellet.setFillColor(sf::Color::Yellow);
    pellet.setPosition(x * TILE_SIZE + TILE_SIZE / 2 - pellet.getRadius(), 
                       y * TILE_SIZE + TILE_SIZE / 2 - pellet.getRadius());

    window.draw(pellet);
}

void drawGameElements(sf::RenderWindow& window, int x, int y, const std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT>& gameBoard) {
    switch (gameBoard[y][x].type) {
        case CellType::Wall: {
            sf::RectangleShape wall(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            wall.setFillColor(sf::Color::Magenta);
            wall.setPosition(x * TILE_SIZE, y * TILE_SIZE);
            window.draw(wall);
            break;
        }
        case CellType::Pellet:
        case CellType::PowerPellet:
            drawPellet(window, x, y);
            break;
        case CellType::Pacman:
            drawPacman(window, x, y);
            break;
        case CellType::Ghost: {
            sf::Color ghostColor = getGhostColor(gameBoard[y][x].character);
            drawGhost(window, x, y, ghostColor);
            break;
        }
        default:
            break;
    }
}


//////////////////////////////

void drawGameOverScreen(sf::RenderWindow& window, int finalScore) {
    sf::Font font;
    if (!font.loadFromFile("Arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
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

void drawYouWonScreen(sf::RenderWindow& window, int finalScore) {
    sf::Font font;
    if (!font.loadFromFile("Arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
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


void handlePacmanMovement(sf::Keyboard::Key key, int& pacmanX, int& pacmanY, std::array<std::array<Cell, MAP_WIDTH>, MAP_HEIGHT>& gameBoard, int& score) {
    int newX = pacmanX, newY = pacmanY;
    switch (key) {
        case sf::Keyboard::W: newY--; break;
        case sf::Keyboard::S: newY++; break;
        case sf::Keyboard::A: newX--; break;
        case sf::Keyboard::D: newX++; break;
        default: return;  // No movement key pressed
    }
    
    if (newX >= 0 && newX < MAP_WIDTH && newY >= 0 && newY < MAP_HEIGHT) {
        CellType cellType = gameBoard[newY][newX].type;
        if (cellType != CellType::Wall) {
            if (cellType == CellType::Pellet || cellType == CellType::PowerPellet) {
                gameBoard[newY][newX].type = CellType::Path; // Consume the pellet
                score++;  // Increment score
                totalPellets--;  // Decrement total pellets count
            }
            // Update Pacman's position
            gameBoard[pacmanY][pacmanX].type = CellType::Path; // Clear old position
            pacmanX = newX;
            pacmanY = newY;
            gameBoard[pacmanY][pacmanX].type = CellType::Pacman; // Set new position
        }
    }
}



///////////////////////////////////////////



bool checkPacmanCollision(sf::RenderWindow& window, int& pacmanX, int& pacmanY, std::vector<Ghost>& ghosts, int& lives, int& score) {
    for (const auto& ghost : ghosts) {
        if (pacmanX == ghost.x && pacmanY == ghost.y) {
            lives--;

            if (lives == 0) {
                drawGameOverScreen(window, score);
                return true; // Game should end
            }
        }
    }
    return false; // Game continues
}


int main() {
    // Initialize all cells to paths and locate Pacman

    // Load font
    sf::Font font;
    if (!font.loadFromFile("Arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }
 // Initialize text elements
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);

    sf::Text livesText;
    livesText.setFont(font);
    livesText.setCharacterSize(24);
    livesText.setFillColor(sf::Color::White);

    // Define tile size and indices
    constexpr int TILE_SIZE = 30;
    int textTileX = 30;
    int textTileY = 10;

    // Calculate positions
    float scoreX = TILE_SIZE * textTileX;
    float scoreY = TILE_SIZE * textTileY;

    float livesX = TILE_SIZE * textTileX;
    float livesY = TILE_SIZE * textTileY + 30;  // Example: 30 pixels below the score

    // Center text horizontally
    sf::FloatRect scoreTextRect = scoreText.getLocalBounds();
    scoreText.setOrigin(scoreTextRect.width / 2.0f, scoreTextRect.height / 2.0f);
    scoreText.setPosition(scoreX, scoreY);

    sf::FloatRect livesTextRect = livesText.getLocalBounds();
    livesText.setOrigin(livesTextRect.width / 2.0f, livesTextRect.height / 2.0f);
    livesText.setPosition(livesX, livesY);

  
   

    int pacmanX = 1, pacmanY = 1; // Initial Pacman position
for (int y = 0; y < MAP_HEIGHT; ++y) {
    for (int x = 0; x < MAZE_WIDTH; ++x) {
        char ch = map_sketch[y][x];
        gameBoard[y][x].character = ch;
        switch (ch) {
            case 'P':
                pacmanX = x;
                pacmanY = y;
                gameBoard[y][x].type = CellType::Pacman;
                break;
            case '#':
                gameBoard[y][x].type = CellType::Wall;
                break;
            case '.':
                gameBoard[y][x].type = CellType::Pellet;
                break;
            case 'o':
                gameBoard[y][x].type = CellType::PowerPellet;
                break;
            default:
                gameBoard[y][x].type = CellType::Path;
                break;
        }
    }
}

    sf::RenderWindow window(sf::VideoMode(MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE), "SFML Maze Game");

    // Main game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                handlePacmanMovement(event.key.code, pacmanX, pacmanY, gameBoard, score);
            }
        }

        if (checkPacmanCollision(window, pacmanX, pacmanY, ghosts, lives, score)) {
            drawGameOverScreen(window, score);
            break;
        }
if (totalPellets == 0) {
    drawYouWonScreen(window, score);
    break;  // Exit the game loop
}

        window.clear(sf::Color::Black);
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            for (int x = 0; x < MAZE_WIDTH; ++x) {
                drawGameElements(window, x, y, gameBoard);
            }
        }
// Move each ghost randomly
        for (auto &ghost : ghosts) {
            moveGhostRandomly(ghost, eng);
        }
        for (const auto& ghost : ghosts) {
            sf::Color ghostColor = getGhostColor(ghost.number);
            drawGhost(window, ghost.x, ghost.y, ghostColor);
        }

        scoreText.setString("Score: " + std::to_string(score));
        livesText.setString("Lives: " + std::to_string(lives));
        window.draw(scoreText);
        window.draw(livesText);

        window.display();
        sf::sleep(sf::milliseconds(200)); // Game speed control
    }

    return 0;
}
