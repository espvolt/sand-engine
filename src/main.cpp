#include <SFML/Graphics.hpp>
#include <iostream>
#include "include/sand.hpp"
#include "include/entity.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({1280, 780}), "Title");
    renderSize = sf::Vector2i(window.getSize());

    initializeWorld({16, 4});
    Entity* player = createPlayer();
    sf::Clock deltaClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        sf::Time dt = deltaClock.restart();

        window.clear();
        doCameraControl(); // debug function
        centerCamera(player);
        doSandPlacement(sf::Mouse::getPosition(window));
        setActiveChunksFromCamera();
        
        drawEntities(window);
        drawChunks(&window);

        updatePhysics(dt);
        updateEntities(dt);
        window.display();
    }
}   