#pragma once
#include <SFML/Graphics.hpp>
#include <map>

enum EntityType {
    PLAYER
};

struct Entity {
    int entityId;
    EntityType entityType;  
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f boundingBox;
    sf::Vector2f boundingBoxOrigin;
    float speed = 5;
    float maxSpeed = 10;
};

extern std::map<int, Entity*> entities;
extern int currentEntityCount;
Entity* createPlayer();
void drawEntities(sf::RenderTarget& target);
void updateEntities(sf::Time& dt);
void centerCamera(Entity*);