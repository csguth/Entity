#include "Particle.hpp"

#include <SFML/Graphics.hpp>
#include <Entity/SystemWithDeletion.hpp>
#include <Entity/Property.hpp>

#include <boost/serialization/strong_typedef.hpp>
BOOST_STRONG_TYPEDEF(Entity::Base, Particle)

#include <random>

struct Data
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    int life{255};
};

using namespace Entity;



int main(int argc, char *argv[])
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");

    SystemWithDeletion<Particle> sys;
    auto data   = makeProperty<Data>(sys);
    auto shapes = makeProperty<sf::CircleShape>(sys);

    std::random_device device;
    std::uniform_real_distribution<float> dist{-1.0, 1.0};

    sf::Font font;
    font.loadFromFile("/usr/share/qtcreator/fonts/SourceCodePro-Regular.ttf");
    sf::Text entityCount;
    entityCount.setFont(font);
    entityCount.setFillColor(sf::Color::Red);

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
            auto particle  = sys.add();
            data[particle]   = {position, {dist(device), -1.0}};
            shapes[particle] = []()
            {
                auto shape = sf::CircleShape(10);
                shape.setFillColor(sf::Color::Blue);
                return shape;
            }();
        }
        // Clear screen
        window.clear();
        entityCount.setString({std::to_string(sys.size())});

        ranges::for_each(data.asRange(), [](Data& data)
        {
            data.pos += data.vel;
            --data.life;
        });

        ranges::for_each(ranges::view::zip(shapes.asRange(), data.asRange()), [&](std::tuple<sf::CircleShape&, const Data&> el)
        {
            std::get<0>(el).setPosition(std::get<1>(el).pos);
            std::get<0>(el).setFillColor(sf::Color(std::get<1>(el).life, 0, 255-std::get<1>(el).life));
        });

        ranges::for_each(shapes.asRange(), [&](const sf::CircleShape& shape)
        {
            window.draw(shape);
        });

        // Update the window
        window.draw(entityCount);
        window.display();
    }
    return EXIT_SUCCESS;
}


