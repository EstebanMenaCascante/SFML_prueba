#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>  // Para manejar la música
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>

class Ball {
public:
    sf::CircleShape shape;
    float speed;

    Ball(float radius, float speed, float startX) {
        shape.setRadius(radius);
        shape.setFillColor(sf::Color(128, 128, 128)); // Color gris
        shape.setPosition(startX, -radius * 2); // Empieza fuera de la ventana
        this->speed = speed;
    }

    // Actualizar posición de la bola
    void update(float deltaTime) {
        shape.move(0, speed * deltaTime);
    }
};

class Bullet {
public:
    sf::RectangleShape shape;
    float speed;

    Bullet(float startX, float startY) {
        shape.setSize(sf::Vector2f(10, 20)); // Tamaño del disparo
        shape.setFillColor(sf::Color::Yellow); // Color amarillo
        shape.setPosition(startX, startY); // Posición inicial
        speed = 500.0f; // Velocidad del disparo
    }

    // Actualizar la posición de la bala (sube)
    void update(float deltaTime) {
        shape.move(0, -speed * deltaTime);
    }
};

void resetGame(std::vector<Ball>& balls, std::vector<Bullet>& bullets, sf::RectangleShape& ship, sf::Clock& gameClock, float& ballSpeedMultiplier, bool& gameOver, float& spawnTimer, float& speedIncreaseInterval) {
    // Limpiar las bolas y las balas
    balls.clear();
    bullets.clear();

    // Reiniciar la nave
    ship.setPosition(375, 500); // Posición inicial de la nave

    // Reiniciar el reloj del juego
    gameClock.restart();

    // Reiniciar el multiplicador de velocidad de las bolas
    ballSpeedMultiplier = 1.0f;

    // Reiniciar el temporizador de generación de bolas
    spawnTimer = 0.0f;

    // Reiniciar el intervalo de incremento de velocidad
    speedIncreaseInterval = 30.0f;

    // Reiniciar el estado de "Game Over"
    gameOver = false;
}

int main() {
    // Configurar la ventana
    sf::RenderWindow window(sf::VideoMode(800, 600), "Galaga");

    // Configurar la "navecita" (un cuadrado)
    sf::RectangleShape ship(sf::Vector2f(50, 50));
    ship.setFillColor(sf::Color::Blue);
    ship.setPosition(375, 500); // Posición inicial

    // Variables para movimiento de la navecita
    float shipSpeed = 300.0f;

    // Crear una lista de bolas grises
    std::vector<Ball> balls;

    // Crear una lista de balas disparadas
    std::vector<Bullet> bullets;

    // Reloj para controlar el tiempo
    sf::Clock clock;
    sf::Clock gameClock; // Para contar el tiempo total jugado
    float spawnTimer = 0.0f;
    float spawnInterval = 1.0f; // Intervalo de 1 segundo para crear bolas
    float speedIncreaseInterval = 30.0f; // Aumentar velocidad cada 30 segundos
    float ballSpeedMultiplier = 1.0f; // Multiplicador para la velocidad de las bolas

    // Generar números aleatorios
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    bool gameOver = false;
    float survivalTime = 0.0f; // Variable para guardar el tiempo de supervivencia

    // ---- Reproducir música ----
    sf::Music music;
    if (!music.openFromFile("/Users/emena/Downloads/Original_Tetris_theme.ogg")) {
        std::cerr << "Error al cargar la música\n";
        return -1; // Si no puede cargar la música, termina el programa
    }

    music.setLoop(true); // Repetir la música en bucle
    music.play();        // Iniciar la reproducción de la música

    // Bucle principal del juego
    while (window.isOpen()) {
        // Manejar eventos
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (!gameOver) {
            // Obtener el tiempo transcurrido
            float deltaTime = clock.restart().asSeconds();

            // Mover la navecita en todas direcciones
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                ship.move(-shipSpeed * deltaTime, 0);
                if (ship.getPosition().x < 0) // Limitar a la ventana
                    ship.setPosition(0, ship.getPosition().y);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                ship.move(shipSpeed * deltaTime, 0);
                if (ship.getPosition().x + ship.getSize().x > window.getSize().x) // Limitar a la ventana
                    ship.setPosition(window.getSize().x - ship.getSize().x, ship.getPosition().y);
            }

            // Disparar una bala al presionar la barra espaciadora
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                bullets.push_back(Bullet(ship.getPosition().x + ship.getSize().x / 2 - 5, ship.getPosition().y));
            }

            // Actualizar balas
            for (auto& bullet : bullets) {
                bullet.update(deltaTime);
            }

            // Crear nuevas bolas grises en intervalos regulares
            spawnTimer += deltaTime;
            if (spawnTimer >= spawnInterval) {
                float radius = static_cast<float>(std::rand() % 30 + 20); // Tamaños entre 20 y 50 píxeles
                float speed = (static_cast<float>(std::rand() % 100 + 100)) * ballSpeedMultiplier; // Velocidad ajustada por el multiplicador
                float startX = static_cast<float>(std::rand() % (window.getSize().x - static_cast<int>(radius * 2)));
                balls.push_back(Ball(radius, speed, startX));
                spawnTimer = 0.0f;
            }

            // Aumentar la velocidad de las bolas cada 30 segundos
            if (gameClock.getElapsedTime().asSeconds() >= speedIncreaseInterval) {
                ballSpeedMultiplier += 0.2f; // Aumentar la velocidad en un 20%
                speedIncreaseInterval += 30.0f; // Próxima vez que se aumente será en otros 30 segundos
            }

            // Actualizar posiciones de las bolas
            for (auto& ball : balls) {
                ball.update(deltaTime);
            }

            // Comprobar colisiones entre balas y bolas
            for (size_t i = 0; i < bullets.size(); ++i) {
                for (size_t j = 0; j < balls.size(); ++j) {
                    if (bullets[i].shape.getGlobalBounds().intersects(balls[j].shape.getGlobalBounds())) {
                        // Eliminar la bola y la bala
                        balls.erase(balls.begin() + j);
                        bullets.erase(bullets.begin() + i);
                        break;
                    }
                }
            }

            // Comprobar colisiones entre la navecita y las bolas
            for (const auto& ball : balls) {
                if (ship.getGlobalBounds().intersects(ball.shape.getGlobalBounds())) {
                    gameOver = true; // Colisión detectada, fin del juego
                    survivalTime = gameClock.getElapsedTime().asSeconds(); // Guardar el tiempo jugado
                    break;
                }
            }

            // Limpiar la ventana y dibujar
            window.clear();
            window.draw(ship);

            for (const auto& ball : balls) {
                window.draw(ball.shape);
            }

            for (const auto& bullet : bullets) {
                window.draw(bullet.shape);
            }

            window.display();
        }
        else {
            // Imprimir "Game Over" y el tiempo jugado
            window.clear();

            sf::Font font;
            if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
                std::cerr << "Error al cargar la fuente\n";
            }

            sf::Text gameOverText;
            gameOverText.setFont(font);
            gameOverText.setString("Game Over");
            gameOverText.setCharacterSize(50);
            gameOverText.setFillColor(sf::Color::Red);
            gameOverText.setPosition(250, 200);

            window.draw(gameOverText);

            // Mostrar el tiempo jugado
            sf::Text timeText;
            timeText.setFont(font);
            timeText.setString("Time: " + std::to_string(static_cast<int>(survivalTime)) + " seconds");
            timeText.setCharacterSize(30);
            timeText.setFillColor(sf::Color::White);
            timeText.setPosition(250, 300);

            window.draw(timeText);
            window.display();

            // Reiniciar el juego al presionar Enter
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
                resetGame(balls, bullets, ship, gameClock, ballSpeedMultiplier, gameOver, spawnTimer, speedIncreaseInterval);
            }
        }
    }

    return 0;
}
