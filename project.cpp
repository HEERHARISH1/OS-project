#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>

// Function declarations for thread tasks
void gameEngine(sf::RenderWindow* window, sf::Sprite* startScreen);
void userInterface();
void ghostController(int ghostId);

int main() {
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(1600, 900), "Pac-Man Start Screen");

    // Load the start screen texture
    sf::Texture startScreenTexture;
    if (!startScreenTexture.loadFromFile("startpic.png")) { // Adjust the path as necessary
        std::cout << "Failed to load Pac-Man start screen texture" << std::endl;
        return -1;
    }

    // Create a sprite for the start screen
    sf::Sprite startScreen(startScreenTexture);

    // Start the game engine thread, passing the window and the start screen sprite
    std::thread engineThread(gameEngine, &window, &startScreen);

    // Create threads for each ghost controller
    const int numberOfGhosts = 4; // Starting with 4 ghosts
    std::thread ghostThreads[numberOfGhosts];
    for (int i = 0; i < numberOfGhosts; ++i) {
        ghostThreads[i] = std::thread(ghostController, i);
    }

    // Wait for threads to finish
    engineThread.join();
    for (int i = 0; i < numberOfGhosts; ++i) {
        ghostThreads[i].join();
    }

    return 0;
}

void gameEngine(sf::RenderWindow* window, sf::Sprite* startScreen) {
    // Display start screen until space is pressed
    bool startGame = false;
    while (window->isOpen() && !startGame) {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                window->close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                startGame = true;
            }
        }

        window->clear();
        window->draw(*startScreen);
        window->display();
    }

    // Main game loop
    while (window->isOpen()) {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                window->close();
        }

        window->clear();
        // Add game rendering logic here
        window->display();
    }
}

void ghostController(int ghostId) {
    // Ghost behavior logic for each ghost based on ghostId
    while (true) {
        // Simulate ghost actions
    }
}

void userInterface() {
    // Handle UI updates, user inputs other than game engine controls
}

