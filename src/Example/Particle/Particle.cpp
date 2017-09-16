#include "Particle.hpp"

#include <random>
#include <chrono>

using namespace Entity;
using namespace std::chrono;
using namespace Example::Particle;

int main(int, char *[])
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");
    window.setFramerateLimit(30);

    ParticleSystem system;

    sf::Font font;
    font.loadFromFile("/usr/share/qtcreator/fonts/SourceCodePro-Regular.ttf");
    sf::Text entityCount;
    entityCount.setFont(font);
    entityCount.setFillColor(sf::Color::Red);

    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setFillColor(sf::Color::Green);
    fpsText.setPosition(0, 20);

    auto lastTick = high_resolution_clock::now();
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();
        }
        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
            const sf::Vector2f position{window.mapPixelToCoords(sf::Mouse::getPosition(window))};
            system.addParticles(100, position);
        }
        // Clear screen
        window.clear();
        entityCount.setString({std::to_string(system.size())});

        system.update();

        auto tick = high_resolution_clock::now();
        auto timeSinceLastTick = static_cast<float>(duration_cast<nanoseconds>(tick-lastTick).count());

        fpsText.setString("FPS: " + std::to_string(1.0e9/timeSinceLastTick));

        system.draw(window);

        // Update the window
        window.draw(entityCount);
        window.draw(fpsText);
        window.display();
        lastTick = tick;
    }
    return EXIT_SUCCESS;
}


