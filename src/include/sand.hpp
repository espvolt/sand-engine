#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>

const int CHUNK_SIZE = 64;

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color drawColor;
    short particleId = -1;
    bool updated = false;
};

struct Camera {
    sf::Vector2f position;
    sf::Vector2f scale = sf::Vector2f(1, 1);
};

struct Chunk { // chunks will be 64x64 matrix of pixels
    Particle* particles[64][64];
    uint8_t texturePixels[64 * 64 + 32];
    sf::Sprite chunkSprite;
    sf::Vector2i chunkOffset;
    int textureId = -1; // client side;
    bool drawUpdate = false;
};

struct TextureChunkResource {
    bool used;
    int id;
    sf::Texture* texture;
    sf::Sprite* sprite;
};

enum ParticleTypes {
    DIRT,
};

extern std::map<int, Chunk*> world;
extern sf::Vector2i WINDOW_SIZE;
extern std::map<int, TextureChunkResource*> chunkResources;
extern std::vector<int> availableChunks;
extern std::vector<Chunk*> activeChunks;
extern sf::Vector2i currentWorldSize;
extern sf::Vector2i renderSize;
extern Camera camera;
extern sf::Texture* drawTexture;

int getWorldKey(sf::Vector2i offset);
Chunk* createChunk(sf::Vector2i offset);
Chunk* addNewChunkWorld(sf::Vector2i offset);
void initializeWorld(sf::Vector2f worldSize);
void initializeChunkRandomly(Chunk* chunk);
void drawChunk(sf::RenderTarget* target, Chunk* chunkOffset);
void drawChunks(sf::RenderTarget* target);
sf::Vector2i getChunkRelativePosition(sf::Vector2i position);
void updatePhysics(sf::Time& time);
void setActiveChunksFromCamera();
void doCameraControl();
void doSandPlacement(sf::Vector2i mousePosition);
bool doesChunkExist(sf::Vector2i offset);
Particle* getParticleAt(sf::Vector2i position);
