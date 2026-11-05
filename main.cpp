#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

// Configurable constants for large scale maps
const int MAP_ROWS = 25;
const int MAP_COLS = 25;
const int TILE_SIZE = 40; 

// 1D Arrays tracking delivery jobs status parameters
float orderTimers[3] = {45.0f, 40.0f, 35.0f};
bool orderCollected[3] = {false, false, false};
bool orderCompleted[3] = {false, false, false};

// Destination coordinates parallel matching systems
int pickupCoordsX[3] = {3, 21, 12};
int pickupCoordsY[3] = {3, 4, 20};
int dropoffCoordsX[3] = {21, 3, 21};
int dropoffCoordsY[3] = {20, 21, 21};

// User-Defined Procedural Function Prototypes
void createProceduralAssets(sf::RenderTexture& textureStore);
void generateCityGrid(int map[MAP_ROWS][MAP_COLS]);
void moveScooter(float& px, float& py, float vx, float vy, float speed, float dt, const int map[MAP_ROWS][MAP_COLS]);
void evaluateObjectives(float px, float py, int& currentOrder, int& gameScore, bool& winState);
void drawScene(sf::RenderWindow& window, const int map[MAP_ROWS][MAP_COLS], float px, float py, int currentOrder, sf::Sprite& tileSprite, sf::Sprite& scooterSprite);

int main() {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8; // Smooths edges for realistic curves

    sf::RenderWindow window(sf::VideoMode(800, 600), "Last Delivery: Premium Realistic Edition", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    sf::View camera(sf::FloatRect(0, 0, 800, 600));

    // Dynamic High-Fidelity Asset Database
    sf::RenderTexture textureStore;
    if (!textureStore.create(160, 40)) {
        return -1;
    }
    createProceduralAssets(textureStore);

    sf::Sprite tileSprite(textureStore.getTexture());
    sf::Sprite scooterSprite(textureStore.getTexture());
    
    // Crop out the high-detail vehicle + character sprite
    scooterSprite.setTextureRect(sf::IntRect(120, 0, 40, 40));
    scooterSprite.setOrigin(20, 20); // Center pivot for realistic steering rotations

    int cityGrid[MAP_ROWS][MAP_COLS];
    generateCityGrid(cityGrid);

    float playerPixelX = 40.0f * 1.0f + 20.0f;
    float playerPixelY = 40.0f * 12.0f + 20.0f;
    float linearSpeed = 220.0f;

    int activeOrderIndex = 0;
    int currentScore = 0;
    bool hasWonGame = false;

    sf::Clock gameClock;

    while (window.isOpen()) {
        float deltaTime = gameClock.restart().asSeconds();
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        if (!hasWonGame && activeOrderIndex < 3) {
            orderTimers[activeOrderIndex] -= deltaTime;
            if (orderTimers[activeOrderIndex] <= 0.0f) {
                std::cout << "Time ran out! Mission Failed.\n";
                window.close();
            }

            float moveX = 0.0f;
            float moveY = 0.0f;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    moveY = -1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  moveY = 1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  moveX = -1.0f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) moveX = 1.0f;

            if (moveX != 0.0f || moveY != 0.0f) {
                moveScooter(playerPixelX, playerPixelY, moveX, moveY, linearSpeed, deltaTime, cityGrid);
                
                // Rotates the entire vehicle and driver together realistically
                float targetAngle = std::atan2(moveY, moveX) * 180.0f / 3.14159f;
                scooterSprite.setRotation(targetAngle + 90.0f); 
            }

            evaluateObjectives(playerPixelX, playerPixelY, activeOrderIndex, currentScore, hasWonGame);
        }

        camera.setCenter(playerPixelX, playerPixelY);
        window.setView(camera);

        drawScene(window, cityGrid, playerPixelX, playerPixelY, activeOrderIndex, tileSprite, scooterSprite);
    }

    return 0;
}

// Generates high-fidelity vehicles, roads, and characters dynamically in VRAM
void createProceduralAssets(sf::RenderTexture& textureStore) {
    textureStore.clear(sf::Color(0, 0, 0, 0));
    sf::RectangleShape canvas(sf::Vector2f(40, 40));

    // Tile 0: Premium Asphalt Road with lane divider lines
    canvas.setFillColor(sf::Color(35, 37, 41));
    canvas.setPosition(0, 0);
    textureStore.draw(canvas);
    sf::RectangleShape line(sf::Vector2f(14, 2));
    line.setFillColor(sf::Color(241, 196, 15)); // Vibrant highway yellow
    line.setPosition(13, 19);
    textureStore.draw(line);

    // Tile 1: Modern City Skyscraper Facade with multiple illuminated glass windows
    canvas.setFillColor(sf::Color(44, 62, 80));
    canvas.setOutlineColor(sf::Color(26, 37, 48));
    canvas.setOutlineThickness(-2);
    canvas.setPosition(40, 0);
    textureStore.draw(canvas);
    sf::RectangleShape windowGlow(sf::Vector2f(8, 10));
    windowGlow.setFillColor(sf::Color(241, 196, 15, 220));
    windowGlow.setPosition(48, 8);  textureStore.draw(windowGlow);
    windowGlow.setPosition(64, 8);  textureStore.draw(windowGlow);
    windowGlow.setPosition(48, 24); textureStore.draw(windowGlow);
    windowGlow.setPosition(64, 24); textureStore.draw(windowGlow);

    // Tile 2: Glowing Cyberpunk Neon Objective Destination Ring
    canvas.setFillColor(sf::Color(35, 37, 41));
    canvas.setOutlineThickness(0);
    canvas.setPosition(80, 0);
    textureStore.draw(canvas);
    sf::CircleShape glowRing(14);
    glowRing.setFillColor(sf::Color(46, 204, 113, 40)); // Translucent soft green glow inner core
    glowRing.setOutlineColor(sf::Color(46, 204, 113));   // Vivid Neon Emerald Green
    glowRing.setOutlineThickness(3);
    glowRing.setPosition(86, 6);
    textureStore.draw(glowRing);

    // TILE 3: HIGH-FIDELITY REALISTIC VEHICLE + CHARACTER DRIVER SPRITE
    // 1. Scooter Chassis Assembly Layer
    sf::RectangleShape tire(sf::Vector2f(6, 10));
    tire.setFillColor(sf::Color(20, 20, 20)); // Deep black rubber tires
    tire.setPosition(137, 2); textureStore.draw(tire);  // Front Wheel
    tire.setPosition(137, 28); textureStore.draw(tire); // Rear Wheel
    
    sf::RectangleShape body(sf::Vector2f(10, 24));
    body.setFillColor(sf::Color(231, 76, 60)); // Premium Gloss Crimson Body Work
    body.setPosition(135, 8);
    textureStore.draw(body);

    sf::RectangleShape handlebars(sf::Vector2f(22, 3));
    handlebars.setFillColor(sf::Color(52, 73, 94)); // Steel finish handles
    handlebars.setPosition(129, 9);
    textureStore.draw(handlebars);

    // 2. Engaging Top-Down Human Driver Character Layer (Seated inside vehicle bounds)
    sf::CircleShape helmet(6);
    helmet.setFillColor(sf::Color(52, 152, 219)); // Deep blue aerodynamic driver safety helmet
    helmet.setPosition(134, 14);
    textureStore.draw(helmet);

    sf::RectangleShape shoulders(sf::Vector2f(14, 6));
    shoulders.setFillColor(sf::Color(44, 44, 44)); // Dark protective motorcycle jacket outline
    shoulders.setPosition(133, 19);
    textureStore.draw(shoulders);

    // Realistic extended arms interacting with steering handles
    sf::RectangleShape leftArm(sf::Vector2f(3, 8));
    leftArm.setFillColor(sf::Color(44, 44, 44));
    leftArm.setPosition(131, 11);
    textureStore.draw(leftArm);

    sf::RectangleShape rightArm(sf::Vector2f(3, 8));
    rightArm.setFillColor(sf::Color(44, 44, 44));
    rightArm.setPosition(146, 11);
    textureStore.draw(rightArm);

    textureStore.display();
}

void generateCityGrid(int map[MAP_ROWS][MAP_COLS]) {
    for (int r = 0; r < MAP_ROWS; ++r) {
        for (int c = 0; c < MAP_COLS; ++c) {
            if (r == 0 || r == MAP_ROWS - 1 || c == 0 || c == MAP_COLS - 1) {
                map[r][c] = 1; 
            } else if (r % 4 == 0 || c % 6 == 0) {
                map[r][c] = 0; 
            } else {
                map[r][c] = 1; 
            }
        }
    }
    for(int i = 0; i < 3; i++) {
        map[pickupCoordsY[i]][pickupCoordsX[i]] = 2;
    }
}

void moveScooter(float& px, float& py, float vx, float vy, float speed, float dt, const int map[MAP_ROWS][MAP_COLS]) {
    float length = std::sqrt(vx * vx + vy * vy);
    if (length != 0) { vx /= length; vy /= length; }

    float nextX = px + (vx * speed * dt);
    float nextY = py + (vy * speed * dt);

    int checkG_X = static_cast<int>(nextX) / TILE_SIZE;
    int checkG_Y = static_cast<int>(nextY) / TILE_SIZE;

    if (checkG_X >= 0 && checkG_X < MAP_COLS && checkG_Y >= 0 && checkG_Y < MAP_ROWS) {
        if (map[checkG_Y][checkG_X] != 1) {
            px = nextX;
            py = nextY;
        }
    }
}

void evaluateObjectives(float px, float py, int& currentOrder, int& gameScore, bool& winState) {
    if (currentOrder >= 3) return;

    int currentGridX = static_cast<int>(px) / TILE_SIZE;
    int currentGridY = static_cast<int>(py) / TILE_SIZE;

    if (!orderCollected[currentOrder]) {
        if (currentGridX == pickupCoordsX[currentOrder] && currentGridY == pickupCoordsY[currentOrder]) {
            orderCollected[currentOrder] = true;
            std::cout << "Package picked up by driver! Proceeding to target dropoff coordinates.\n";
        }
    } else {
        if (currentGridX == dropoffCoordsX[currentOrder] && currentGridY == dropoffCoordsY[currentOrder]) {
            orderCompleted[currentOrder] = true;
            gameScore += 500 + static_cast<int>(orderTimers[currentOrder] * 10);
            std::cout << "Delivery Successful! Current Team Score: " << gameScore << "\n";
            currentOrder++;
            if (currentOrder >= 3) winState = true;
        }
    }
}

void drawScene(sf::RenderWindow& window, const int map[MAP_ROWS][MAP_COLS], float px, float py, int currentOrder, sf::Sprite& tileSprite, sf::Sprite& scooterSprite) {
window.clear(sf::Color(16, 18, 22)); // High-contrast smooth premium dark backdrop
int centerGridX = static_cast(px) / TILE_SIZE;
int centerGridY = static_cast(py) / TILE_SIZE;
int minRow = std::max(0, centerGridY - 9);
int maxRow = std::min(MAP_ROWS - 1, centerGridY + 9);
int minCol = std::max(0, centerGridX - 11);
int maxCol = std::min(MAP_COLS - 1, centerGridX + 11);
// Floor Base Pass
for (int r = minRow; r <= maxRow; ++r) {
for (int c = minCol; c <= maxCol; ++c) {
int tileType = map[r][c];
if (currentOrder < 3 && orderCollected[currentOrder]) {
if (c == dropoffCoordsX[currentOrder] && r == dropoffCoordsY[currentOrder]) {
tileType = 2; // Swaps roads with glowing neon drop off locations dynamically
}
}
if (tileType != 1) {
tileSprite.setPosition(c * TILE_SIZE, r * TILE_SIZE);
tileSprite.setTextureRect(sf::IntRect(tileType * 40, 0, 40, 40));
tileSprite.setColor(sf::Color::White);
window.draw(tileSprite);
}
}
}
// Architecture Shadow and Overhang Roof Layer Pass (Creates depth elements)
for (int r = minRow; r <= maxRow; ++r) {
for (int c = minCol; c <= maxCol; ++c) {
if (map[r][c] == 1) {
sf::RectangleShape dropShadow(sf::Vector2f(40, 40));
dropShadow.setFillColor(sf::Color(0, 0, 0, 100)); // Smooth semi-translucent soft shadows
dropShadow.setPosition(c * TILE_SIZE + 5, r * TILE_SIZE + 5);
window.draw(dropShadow);
tileSprite.setPosition(c * TILE_SIZE, r * TILE_SIZE);
tileSprite.setTextureRect(sf::IntRect(40, 0, 40, 40));
tileSprite.setColor(sf::Color::White);
window.draw(tileSprite);
}
}
}
scooterSprite.setPosition(px, py);
window.draw(scooterSprite);
window.display();
}
