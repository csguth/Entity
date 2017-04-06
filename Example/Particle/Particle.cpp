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
};

using namespace Entity;



int main(int argc, char *argv[])
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");
    window.setFramerateLimit(30);

    SystemWithDeletion<Particle> sys;
    auto data   = makeProperty<Data>(sys);
    auto life   = makeProperty<uint8_t>(sys);
    auto shapes = makeProperty<sf::CircleShape>(sys);

    std::random_device device;
    std::uniform_real_distribution<float> dist{-1.0, 1.0};
    std::uniform_int_distribution<uint8_t> dist2{0, 100};

    sf::Font font;
    font.loadFromFile("/usr/share/qtcreator/fonts/SourceCodePro-Regular.ttf");
    sf::Text entityCount;
    entityCount.setFont(font);
    entityCount.setFillColor(sf::Color::Red);

    std::vector<Particle> toKill;
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
            for(int i = 0; i < 2; ++i)
            {
                auto particle    = sys.add();
                data[particle]   = {position, {dist(device), -1.0}};
                life[particle]   = 255 - dist2(device);
                shapes[particle] = []()
                {
                    auto shape = sf::CircleShape(10);
                    return shape;
                }();
            }
        }
        // Clear screen
        window.clear();
        entityCount.setString({std::to_string(sys.size())});


        ranges::for_each(life.asRange(), [](std::uint8_t& qnt)
        {
            --qnt;
        });

        toKill = sys.asRange() | ranges::view::filter([&](Particle par)
        {
            return life[par] == 0;
        });

        ranges::for_each(toKill, [&](Particle par)
        {
            sys.erase(par);
        });

        toKill.resize(0);


        ranges::for_each(data.asRange(), [](Data& data)
        {
            data.pos += data.vel;
        });

        ranges::for_each(ranges::view::zip(shapes.asRange(), data.asRange(), life.asRange()), [&](std::tuple<sf::CircleShape&, const Data&, const uint8_t> tuple)
        {
            sf::CircleShape& shape = std::get<0>(tuple);
            const Data& data       = std::get<1>(tuple);
            const uint8_t life     = std::get<2>(tuple);
            shape.setPosition(data.pos);
            shape.setFillColor(sf::Color(255, 255-life, 0));
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


