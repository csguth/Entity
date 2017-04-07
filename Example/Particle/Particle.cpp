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
using ParticleContour = std::array<sf::Vertex, 6>;

template <template <typename> class SystemType>
struct ParticlesView : public sf::Drawable
{
    Property<Particle, ParticleContour, SystemType> property;
    ParticlesView(SystemType<Particle>& sys):
        property(makeProperty<ParticleContour>(sys))
    {

    }
    void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        target.draw(reinterpret_cast<const sf::Vertex*>(property.data()), property.size()*6, sf::PrimitiveType::Lines, states);
    }
};

#include <chrono>
using namespace std::chrono;

int main(int argc, char *argv[])
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");

    SystemWithDeletion<Particle> sys;
    auto data   = makeProperty<Data>(sys);
    auto life   = makeProperty<uint8_t>(sys);
    ParticlesView<SystemWithDeletion> view{sys};


    std::random_device device;
    std::uniform_real_distribution<float> dist{-1.0, 1.0};
    std::uniform_int_distribution<uint8_t> dist2{0, 100};

    sf::Font font;
    font.loadFromFile("/usr/share/qtcreator/fonts/SourceCodePro-Regular.ttf");
    sf::Text entityCount;
    entityCount.setFont(font);
    entityCount.setFillColor(sf::Color::Red);

    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setFillColor(sf::Color::Green);
    fpsText.setPosition(0, 20);

    auto pos2vertexArray = [](sf::Vector2f pos, sf::Color color)
    {
        ParticleContour vertices;
        vertices[0].position = pos;
        vertices[0].color    = color;
        vertices[1].position = {pos.x+10.f, pos.y+10.f};
        vertices[1].color    = color;
        vertices[2] = vertices[1];
        vertices[3].position = {pos.x+10.f, pos.y};
        vertices[3].color    = color;
        vertices[4] = vertices[3];
        vertices[5] = vertices[0];
        return vertices;
    };

    std::vector<Particle> toKill;
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

            const std::vector<Particle> particles = [&](int n)
            {
                std::vector<Particle> particles(n);
                for(int i = 0; i < n; ++i)
                {
                    particles[i] = sys.add();
                }
                return particles;
            }(10);
            for(auto particle: particles)
                data[particle]          = {position, {dist(device), -1.0}};
            for(auto particle: particles)
                life[particle]          = 255 - dist2(device);
            for(auto particle: particles)
                view.property[particle] = pos2vertexArray(position, sf::Color::Blue);
        }
        // Clear screen
        window.clear();
        entityCount.setString({std::to_string(sys.size())});


        // Update life
        ranges::for_each(life.asRange(), [](std::uint8_t& qnt)
        {
            --qnt;
        });

        // Kill dead entities
        toKill = sys.asRange() | ranges::view::filter([&](Particle par)
        {
            return life[par] == 0;
        });

        ranges::for_each(toKill, [&](Particle par)
        {
            sys.erase(par);
        });

        toKill.resize(0);

        // Update position
        ranges::for_each(data.asRange(), [](Data& data)
        {
            data.pos += data.vel;
        });

        // Update shapes
        ranges::for_each(ranges::view::zip(view.property.asRange(), data.asRange(), life.asRange()), [&](std::tuple<ParticleContour&, const Data&, const uint8_t> tuple)
        {
            ParticleContour& shape = std::get<0>(tuple);
            const Data& data                 = std::get<1>(tuple);
            const uint8_t life               = std::get<2>(tuple);
            shape = pos2vertexArray(data.pos, sf::Color{255, 255-life, 0});
        });

        auto tick = high_resolution_clock::now();
        auto timeSinceLastTick = static_cast<float>(duration_cast<nanoseconds>(tick-lastTick).count());


        fpsText.setString("FPS: " + std::to_string(1.0e9/timeSinceLastTick));

        // Draw particles
        window.draw(view);

        // Update the window
        window.draw(entityCount);
        window.draw(fpsText);
        window.display();
        lastTick = tick;
    }
    return EXIT_SUCCESS;
}


