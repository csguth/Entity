#include "Particle.hpp"

#include <SFML/Graphics.hpp>
#include <Entity/SystemWithDeletion.hpp>
#include <Entity/Property.hpp>

#include <boost/serialization/strong_typedef.hpp>
BOOST_STRONG_TYPEDEF(Entity::Base, Particle)

#include <random>
#include <chrono>
using namespace std::chrono;

struct Data
{
    sf::Vector2f pos;
    sf::Vector2f vel;

    void update()
    {
        pos += vel;
    }
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



int main(int argc, char *argv[])
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode::getFullscreenModes()[0], "SFML window");
    //window.setFramerateLimit(30);
    SystemWithDeletion<Particle> sys;
    auto data   = makeProperty<Data>(sys);
    auto life   = makeProperty<uint8_t>(sys);
    ParticlesView<SystemWithDeletion> view{sys};


    std::random_device device;
    std::uniform_real_distribution<float> dist{-10.0, 10.0};
    std::uniform_int_distribution<int> dist2{0, 100};
	std::uniform_int_distribution<int> dist3{ 100, 255 };

    sf::Font font;
    //font.loadFromFile("/usr/share/qtcreator/fonts/SourceCodePro-Regular.ttf");
	font.loadFromFile("C:\\Windows\\\Fonts\\\Arial.ttf");
    sf::Text entityCount;
    entityCount.setFont(font);
    entityCount.setFillColor(sf::Color::Cyan);

    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setFillColor(sf::Color::Green);
    fpsText.setPosition(0, 20);

    auto pos2vertexArray = [](sf::Vector2f pos, sf::Color color)
    {
        const ParticleContour vertices =
        {
            sf::Vertex{pos, color, {}},
            sf::Vertex{{pos.x+10.f, pos.y+10.f}, color, {}},
            sf::Vertex{{pos.x+10.f, pos.y+10.f}, color, {}},
            sf::Vertex{{pos.x+10.f, pos.y}, color, {}},
            sf::Vertex{{pos.x+10.f, pos.y}, color, {}},
            sf::Vertex{pos, color, {}}
        };
        return vertices;
    };

    std::vector<Particle> toKill;
    auto lastTick = system_clock::now();
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
            }(10000);
            for(auto particle: particles)
                data[particle]          = {position, {dist(device), -2.0f*std::abs(dist(device))}};
            for(auto particle: particles)
                life[particle]          = 255 - dist2(device);
            for(auto particle: particles)
				view.property[particle] = pos2vertexArray(position, sf::Color {0, 0, 0, dist3(device)});
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
        ranges::for_each(data.asRange(), &Data::update);

        // Update shapes
        ranges::for_each(ranges::view::zip(view.property.asRange(), data.asRange(), life.asRange()), [&](std::tuple<ParticleContour&, const Data&, const uint8_t> tuple)
        {
            ParticleContour& shape = std::get<0>(tuple);
            const Data& data                 = std::get<1>(tuple);
            const uint8_t life               = std::get<2>(tuple);
            shape = pos2vertexArray(data.pos, sf::Color{255, 255-life, 0, shape[0].color.a});
        });

        auto tick = system_clock::now();
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


