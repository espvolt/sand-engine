#include "include/entity.hpp"
#include "include/sand.hpp"
#include <math.h>

#define sign(x) abs(x) / x
std::map<int, Entity*> entities;
int currentEntityCount = 0;
Entity* createEntity() {
    Entity* res = new Entity();
    res->entityId = currentEntityCount;
    currentEntityCount += 1;

    entities[res->entityId] = res;
    return res; 
}

Entity* createPlayer() {
    Entity* res = createEntity();
    res->entityType = EntityType::PLAYER;
    res->boundingBox = sf::Vector2f(2, 4);

    return res;
}


void stepEntity(Entity* entity, sf::Vector2i position, int stepHeight) {
    Particle* particle = getParticleAt(position);

    if (particle == NULL) {
        return;
    }

    Particle* particleAbove = getParticleAt(sf::Vector2i(particle->position) + sf::Vector2i(0, -1)); 
    for (int i = 0; i < stepHeight; i++) {
        Particle* particleAbove = getParticleAt(sf::Vector2i(particle->position) + sf::Vector2i(0, -1 * (i + 1)));
        if (particleAbove != NULL) {
            float cornerPosition = entity->boundingBox.x / 2 - sign(entity->velocity.x) * entity->boundingBox.x / 2;

            entity->velocity.x = 0;
            entity->position.x = particle->position.x + cornerPosition;
            return;
        }
    }

    entity->position.y -= stepHeight;
}

void updatePlayer(Entity* entity, sf::Time& dt) {
    sf::Vector2i bottomPosition = sf::Vector2i(entity->position + sf::Vector2f(0, entity->boundingBox.y));

    entity->velocity += sf::Vector2f(0, 9.81) * dt.asSeconds();

    // bottom collision
    bool collided = false;
    if (entity->velocity.y > 0) {
        bool collided = false;
        float steps = entity->boundingBox.x + 1;
        for (int i = 0; i < entity->velocity.y; i++) {
            for (int j = 0; j < steps; j++) {
                Particle* currentParticle = getParticleAt(bottomPosition + sf::Vector2i(j, i));

                if (currentParticle != NULL) {
                    entity->position.y = currentParticle->position.y - entity->boundingBox.y;
                    entity->velocity.y = 0;
                    collided = true;
                    break;
                }
            }

            if (collided) {
                break;
            }
        }
    }
    entity->position.y += entity->velocity.y;
    // auto-step
    float nextVelX = entity->velocity.x;

    if (abs(nextVelX) > 0) {
        float vSign = sign(nextVelX);
        float cornerPosition = entity->boundingBox.x / 2 + vSign * entity->boundingBox.x / 2;
        // printf("%f\n", cornerPosition);

        if (abs(nextVelX) < 1) {
            sf::Vector2i position = sf::Vector2i(entity->position) + sf::Vector2i(cornerPosition + vSign * .1, entity->boundingBox.y - 1);
            stepEntity(entity, position, 1);
        } else {
            for (int i = 0; i < abs(nextVelX); i++) {
                sf::Vector2i position = sf::Vector2i(entity->position) + sf::Vector2i(i * vSign + cornerPosition, entity->boundingBox.y - 1);
                stepEntity(entity, position, 1);
            }
        }
        
    }
    entity->position.x += entity->velocity.x;
    

    entity->velocity.x *= .9;

    if (abs(entity->velocity.x) < .1) {
        entity->velocity.x = 0;
    }


    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        entity->velocity += sf::Vector2f(entity->speed, 0) * dt.asSeconds();
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        entity->velocity += sf::Vector2f(-entity->speed, 0) * dt.asSeconds();
    }
    
    if (abs(entity->velocity.x) > entity->maxSpeed) {
        entity->velocity.x = abs(entity->velocity.x) / entity->velocity.x * entity->maxSpeed;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        entity->velocity += sf::Vector2f(0, -50) * dt.asSeconds();
    }
}

sf::RectangleShape playerShape = sf::RectangleShape({2, 4});

void drawPlayer(sf::RenderTarget& target, Entity* entity) {
    sf::Vector2f drawPosition = entity->position - camera.position;
    playerShape.setFillColor(sf::Color(255, 0, 0));     
    drawPosition.x *= camera.scale.x;
    drawPosition.y *= camera.scale.y;

    playerShape.setPosition(drawPosition);
    playerShape.setScale(camera.scale);
    target.draw(playerShape);
}

void drawEntities(sf::RenderTarget& target) {
    for (auto it = entities.begin(); it != entities.end(); it++) {
        Entity* current = it->second;

        switch(current->entityType) {
            case(EntityType::PLAYER):
                drawPlayer(target, current);
                break;
        }
    }
}



void updateEntities(sf::Time& dt) {
    for (auto it = entities.begin(); it != entities.end(); it++) {
        Entity* current = it->second;

        switch (current->entityType) {
            case (EntityType::PLAYER):
                updatePlayer(current, dt);
                break;
        }
    }
}

void centerCamera(Entity* entity) {
    sf::Vector2f offset = sf::Vector2f(renderSize / 2);
    offset.x /= camera.scale.x;
    offset.y /= camera.scale.y;
    camera.position = entity->position - offset;

} 