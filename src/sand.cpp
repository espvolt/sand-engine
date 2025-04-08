#include "include/sand.hpp"
#include "include/entity.hpp"
#include <iostream>

std::map<int, Chunk*> world;
std::vector<Chunk*> activeChunks;

std::map<int, TextureChunkResource*> chunkResources;
std::vector<int> availableChunks;
int chunkResourceId = 0;

sf::Vector2i currentWorldSize;
sf::Vector2i renderSize;

sf::Texture* worldTexture = nullptr;
Camera camera;

Particle WORLD_BORDER_PARTICLE = Particle();

bool currentUpdateCycle = true;

Chunk* createChunk(sf::Vector2i offset) {
    Chunk* res = new Chunk();
    res->chunkOffset = offset;
    // res->texturePixels = new uint8_t[64 * 64]();
    return res;
}

Chunk* addNewChunkWorld(sf::Vector2i offset) {
    Chunk* newChunk = createChunk(offset);
    std::cout << "Created CHUNK " << offset.x << ", " << offset.y << " WITH KEY " << getWorldKey(offset) << " -> " << newChunk << std::endl;
    world[getWorldKey(offset)] = newChunk;
    return newChunk;
}

void initializeChunkRandomly(Chunk& chunk) {
    sf::Vector2f chunkPositionOffset = sf::Vector2f(CHUNK_SIZE * chunk.chunkOffset);
    
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            float random = rand() / (float) RAND_MAX;

            if (random > .8) {
                Particle* newParticle = new Particle();

                newParticle->position = chunkPositionOffset + sf::Vector2f(j, i);
                newParticle->drawColor = sf::Color(255, 255, 255);
                newParticle->particleId = ParticleTypes::DIRT;
                chunk.particles[i][j] = newParticle;
            }
        }
    }
}
void initializeWorld(sf::Vector2f worldSize) {
    // world size is in Chunks
    WORLD_BORDER_PARTICLE.particleId = -2;
    for (int i = 0; i < worldSize.x; i++) {
        for (int j = 0; j < worldSize.y; j++) {
            Chunk* newChunk = addNewChunkWorld({i, j});
            initializeChunkRandomly(*newChunk);
        }
    }

    std::cout << (world.find(4) == world.end()) << std::endl;
    if (worldTexture != nullptr) {
        delete worldTexture;
    }

}
sf::RectangleShape drawShape = sf::RectangleShape({1, 1});


int getChunkResource() {
    if (availableChunks.size() == 0) { // gotta create a new chunk resource;
        TextureChunkResource* newResource = new TextureChunkResource();

        newResource->id = chunkResourceId;
        chunkResourceId += 1;

        sf::Texture* newTexture = new sf::Texture();
        newTexture->create(CHUNK_SIZE, CHUNK_SIZE);

        sf::Sprite* newSprite = new sf::Sprite();
        newSprite->setTexture(*newTexture);
        newSprite->setTextureRect({0, 0, 64, 64});
        newResource->sprite = newSprite;
        newResource->texture = newTexture; // must be cleaned up latr lol

        chunkResources[newResource->id] = newResource;

        std::cout << "NEW CHUNK RESOURCE " << newResource->id << " CREATED WITH TEXTURE ";
        std::cout << newTexture << " AND SPRITE " << newSprite << std::endl; 

        return newResource->id;
    } else {
        int res = availableChunks[availableChunks.size() - 1];
        availableChunks.pop_back();
        return res;
    }
}

uint8_t convertToUint(sf::Color color) {
    return color.r * 65565;
}

void drawChunk(sf::RenderTarget* target, Chunk* chunk) {
    if (!doesChunkExist(chunk->chunkOffset)) {
        return;
    }
    
    drawShape.setScale(camera.scale);

    sf::Vector2f drawChunkOffset = sf::Vector2f(CHUNK_SIZE * chunk->chunkOffset);
    int k = 0; // current particle index
    
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            Particle* currentParticle = chunk->particles[i][j];

            if (currentParticle == NULL || currentParticle->particleId == -1) { // this particle is AIR
                continue;
            }
     
            sf::Vector2f screenPosition = drawChunkOffset + sf::Vector2f(j, i) - camera.position;

            screenPosition.x *= camera.scale.x;
            screenPosition.y *= camera.scale.y;

            drawShape.setPosition(screenPosition);
            drawShape.setFillColor(currentParticle->drawColor);
            target->draw(drawShape);
        }
    }
}

void drawChunks(sf::RenderTarget* target) {
    sf::Vector2f windowEnd = sf::Vector2f(renderSize) + camera.position;
    // std::cout << windowEnd.x << " " << windowEnd.y;

    sf::Vector2f scaled = camera.position;
    scaled.x *= camera.scale.x;
    scaled.y *= camera.scale.y;

    for (auto it = world.begin(); it != world.end(); it++) {
        Chunk* current = it->second;
        sf::Vector2i renderPosition = CHUNK_SIZE * current->chunkOffset;

        renderPosition.x *= camera.scale.x;
        renderPosition.y *= camera.scale.y; 

        if (renderPosition.x - scaled.x < windowEnd.x && renderPosition.y - scaled.y < windowEnd.y) {
            drawChunk(target, current);
        }
    }
}

bool doesChunkExist(sf::Vector2i chunkOffset) {
    // std::cout << chunkOffset.x << ", " << chunkOffset.y << std::endl;
    // std::cout << "?" << world.find(getWorldKey(chunkOffset))->first << std::endl;
    // std::cout << "tried to access with key" << getWorldKey(chunkOffset) << std::endl; 
    // std::cout << (world.find(getWorldKey(chunkOffset)) == world.end()) << std::endl;
    return world.find(getWorldKey(chunkOffset)) != world.end();
}

int getWorldKey(sf::Vector2i offset) {
    return 32768 * offset.x + offset.y;
}

Particle* getParticleAt(sf::Vector2i position) {
    sf::Vector2i chunkPosition = position / CHUNK_SIZE;

    if (!doesChunkExist(chunkPosition)) {
        return &WORLD_BORDER_PARTICLE;
    }
    Chunk* chunk = world[getWorldKey(chunkPosition)];
    sf::Vector2i relativePosition = getChunkRelativePosition(position);
    return chunk->particles[relativePosition.y][relativePosition.x];
}

void setParticleAt(Particle* newParticle, sf::Vector2i position) {
    sf::Vector2i chunkPosition = position / CHUNK_SIZE;

    if (!doesChunkExist(chunkPosition)) {
        return;
    }

    Chunk* chunk = world[getWorldKey(chunkPosition)];

    sf::Vector2i relativePosition = getChunkRelativePosition(position);
    newParticle->position = sf::Vector2f(position);
    chunk->drawUpdate = true;
    chunk->particles[relativePosition.y][relativePosition.x] = newParticle;
}

Chunk* getChunk(sf::Vector2i position) {
    if (!doesChunkExist(position)) {
        return NULL;
    }

    return world[getWorldKey(position)];
}

sf::Vector2i getChunkRelativePosition(sf::Vector2i position) {
    sf::Vector2i res;
    res.x = position.x % CHUNK_SIZE;
    res.y = position.y % CHUNK_SIZE;
    return res;
}

void moveParticle(Particle* particle, sf::Vector2f position) {
    Chunk* startChunk = getChunk(sf::Vector2i(particle->position) / CHUNK_SIZE);
    Chunk* endChunk = getChunk(sf::Vector2i(position) / CHUNK_SIZE);
    // std::cout << endChunk << " " << (int) (endChunk == NULL) << std::endl;
    if (endChunk == NULL) {
        return;
    }
    sf::Vector2i toRemove = getChunkRelativePosition(sf::Vector2i(particle->position));
    sf::Vector2i toAdd = getChunkRelativePosition(sf::Vector2i(position));   

    startChunk->drawUpdate = true;
    endChunk->drawUpdate = true;

    startChunk->particles[toRemove.y][toRemove.x] = NULL;
    endChunk->particles[toAdd.y][toAdd.x] = particle;
    particle->position = position;
}


void sandBehavior(Particle* particle) {
    sf::Vector2i belowPos = sf::Vector2i(particle->position) + sf::Vector2i(0, 1);
    Particle* particleBelow = getParticleAt(belowPos);
    // std::cout << "issue" << std::endl;
    // std::cout << particleBelow << std::endl;
    if (particleBelow == NULL) {
        particle->velocity += {0, 1};

        float currentY = 1;

        while (currentY <= particle->velocity.y) {
            sf::Vector2i currentPos = sf::Vector2i(particle->position) + sf::Vector2i(0, currentY);
            Particle* currentParticle = getParticleAt(currentPos);

            if (currentParticle != NULL || currentParticle == &WORLD_BORDER_PARTICLE) {
                particle->velocity = sf::Vector2f(0, 0);

                moveParticle(particle, sf::Vector2f(currentPos) + sf::Vector2f(0, -1));
                particle->updated = currentUpdateCycle;
                // std::cout << "made it early" << std::endl;
                return;
            }
            currentY += 1;
        }
        
        sf::Vector2f resPosition = particle->position + particle->velocity;
        // should always be null;
        moveParticle(particle, resPosition);
        particle->updated = currentUpdateCycle;
        // std::cout << "made it out" << std::endl;
        return;
    } else {
        sf::Vector2f particleRightPosition = particle->position + sf::Vector2f(1, 1);
        Particle* target = getParticleAt(sf::Vector2i(particleRightPosition));

        if (target == NULL) {
            moveParticle(particle, particleRightPosition);
            particle->updated = currentUpdateCycle;
            return;
        }

        sf::Vector2f particleLeftPosition = particle->position + sf::Vector2f(-1, 1);
        target = getParticleAt(sf::Vector2i(particleLeftPosition));
        
        if (target == NULL) {
            moveParticle(particle, particleLeftPosition);
            particle->updated = currentUpdateCycle;
            return;
        }
    }
}

void updatePhysics(sf::Time& dt) {
    for (auto it = activeChunks.begin(); it != activeChunks.end(); it++) {
        Chunk* currentChunk = *it;

        for (int i = 0; i < CHUNK_SIZE; i++) {
            for (int j = 0; j < CHUNK_SIZE; j++) {
                Particle* particle = currentChunk->particles[j][i];

                if (particle == NULL || particle->updated == currentUpdateCycle) {
                    continue;
                }

                sandBehavior(particle);
            }
        }

    }

    currentUpdateCycle = !currentUpdateCycle;
}

void doCameraControl() { // debug function
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Equal)) {
        camera.scale += sf::Vector2f(.1, .1);
    } 
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Dash)) {
        camera.scale -= sf::Vector2f(.1, .1);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        camera.position.y -= 1;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        camera.position.y += 1;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        camera.position.x += 1;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        camera.position.x -= 1;
    }
}

void doSandPlacement(sf::Vector2i mousePosition) {
    if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        return;
    }

    sf::Vector2f scaledCamera = camera.position;
    scaledCamera.x *= camera.scale.x;
    scaledCamera.y *= camera.scale.y;

    sf::Vector2f worldPosition = sf::Vector2f(mousePosition + sf::Vector2i(scaledCamera));

    worldPosition.x /= camera.scale.x;
    worldPosition.y /= camera.scale.y;

    Particle* mouseParticle = getParticleAt(sf::Vector2i(worldPosition));
    std::cout << mouseParticle << std::endl;

    if (mouseParticle == NULL) {
        Particle* newParticle = new Particle();
        newParticle->particleId = ParticleTypes::DIRT;
        newParticle->drawColor = sf::Color(255, 255, 255);
        setParticleAt(newParticle, sf::Vector2i(worldPosition));
    }
}

void setAllParticlesColor(Chunk* chunk, sf::Color color) {
    for (int i = 0; i < CHUNK_SIZE; i++) {
        for (int j = 0; j < CHUNK_SIZE; j++) {
            if (chunk->particles[i][j] == NULL) {
                continue;
            }
            chunk->particles[i][j]->drawColor = color;
        }
    }
}

void setActiveChunksFromCamera() {
    // for (auto it = activeChunks.begin(); it != activeChunks.end(); it++) {
    //     setAllParticlesColor(*it, sf::Color(255, 0, 0));
    // } // debug

    activeChunks.clear();
    sf::Vector2f chunksToSearch = sf::Vector2f(renderSize.x / camera.scale.x, renderSize.y / camera.scale.y);
    sf::Vector2i chunkStart = sf::Vector2i(camera.position) / CHUNK_SIZE;
    // size of the window from the pespective of the user

    chunksToSearch.x /= CHUNK_SIZE;
    chunksToSearch.y /= CHUNK_SIZE;
    
    // std::cout << chunksToSearch.x << " " << chunksToSearch.y << std::endl;

    for (int i = 0; i < chunksToSearch.x + 1; i++) {
        for (int j = 0; j < chunksToSearch.y + 1; j++) {
            int chunkKey = getWorldKey(chunkStart + sf::Vector2i(i, j));
            if (world.find(chunkKey) != world.end()) {
                activeChunks.push_back(world[chunkKey]);
                // setAllParticlesColor(world[chunkKey], sf::Color(255, 0, 255));
            }
        }
    }
}