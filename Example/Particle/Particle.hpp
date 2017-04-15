#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <Entity/Core/SystemWithDeletion.hpp>
#include <Entity/Core/Property.hpp>
#include <SFML/Graphics.hpp>

namespace Example
{
namespace Particle
{

struct Particle: Entity::Base<Particle>
{
    using Entity::Base<Particle>::Base;
    static std::string name()
    {
        return "Particle";
    }
};

struct Data
{
    sf::Vector2f pos;
    sf::Vector2f vel;

    void update()
    {
        pos += vel;
    }
};

struct ParticleContour: std::array<sf::Vertex, 6>
{
    void set(sf::Vector2f pos, sf::Color color)
    {
        std::array<sf::Vertex, 6>& array = *this;
        array =
        {
            sf::Vertex{pos, color, {}},
            sf::Vertex{{pos.x+10.f, pos.y+10.f}, color, {}},
            sf::Vertex{{pos.x+10.f, pos.y+10.f}, color, {}},
            sf::Vertex{{pos.x+10.f, pos.y}, color, {}},
            sf::Vertex{{pos.x+10.f, pos.y}, color, {}},
            sf::Vertex{pos, color, {}}
        };
    }
};

template <template <typename> class SystemType>
struct ParticlesView : public sf::Drawable
{
    Entity::Property<Particle, ParticleContour, SystemType> property;
    ParticlesView(SystemType<Particle>& sys):
        property(Entity::makeProperty<ParticleContour>(sys))
    {}
    void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(reinterpret_cast<const sf::Vertex*>(property.data()), property.size()*6, sf::PrimitiveType::Lines, states);
    }
};

class ParticleSystem
{
public:
    ParticleSystem()
        : m_sys(),
          m_data(Entity::makeProperty<Data>(m_sys)),
          m_life(Entity::makeProperty<uint8_t>(m_sys)),
          m_view(m_sys)
    {}
    void addParticles(uint32_t num, sf::Vector2f position)
    {
        std::uniform_real_distribution<float> velocityDistribution{-10.0, 10.0};
        std::uniform_int_distribution<uint8_t> lifeDistribution{0, 100};
        const std::vector<Particle> particles = [&](int n)
        {
            std::vector<Particle> particles(n);
            for(int i = 0; i < num; ++i)
            {
                particles[i] = m_sys.add();
            }
            return particles;
        }(100);
        for(auto particle: particles)
            m_data[particle]          = {position, {velocityDistribution(m_randomDevice), velocityDistribution(m_randomDevice)}};
        for(auto particle: particles)
            m_life[particle]          = 255 - lifeDistribution(m_randomDevice);
        for(auto particle: particles)
            m_view.property[particle].set(position, sf::Color::Blue);
    }
    std::size_t size() const
    {
        return m_sys.size();
    }

    void update()
    {
        // Update life
        ranges::for_each(m_life.asRange(), [](std::uint8_t& qnt)
        {
            --qnt;
        });

        // Kill dead entities
        m_toKill = m_sys.asRange() | ranges::view::filter([&](Particle par)
        {
            return m_life[par] == 0;
        });

        ranges::for_each(m_toKill, [&](Particle par)
        {
            m_sys.erase(par);
        });

        m_toKill.resize(0);

        // Update position
        ranges::for_each(m_data.asRange(), &Data::update);

        // Update shapes
        ranges::for_each(ranges::view::zip(m_view.property.asRange(), m_data.asRange(), m_life.asRange()), [&](std::tuple<ParticleContour&, const Data&, const uint8_t> tuple)
        {
            ParticleContour& shape           = std::get<0>(tuple);
            const Data& data                 = std::get<1>(tuple);
            const uint8_t life               = std::get<2>(tuple);
            shape.set(data.pos, sf::Color{255, 255-life, 0});
        });
    }

    void draw(sf::RenderWindow& window) const
    {
        window.draw(m_view);
    }

private:
    Entity::SystemWithDeletion<Particle> m_sys;
    decltype(Entity::makeProperty<Data>(m_sys)) m_data;
    decltype(Entity::makeProperty<uint8_t>(m_sys)) m_life;
    ParticlesView<Entity::SystemWithDeletion> m_view;
    std::random_device m_randomDevice;

    std::vector<Particle> m_toKill;
};

}
}

#endif // PARTICLE_HPP
