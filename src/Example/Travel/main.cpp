#include <iostream>
#include <map>
#include <set>

#include <Entity/Graph/Graph.hpp>
#include <Entity/Graph/Dijkstra.hpp>

using namespace Entity::Graph;

struct City
{
    std::string name;
};

std::ostream& operator<<(std::ostream& out, const City& c)
{
    return out << c.name;
}

struct Travel
{
    struct Company
    {
        std::string name;
        enum class Type
        {
            Flight,
            Train,
            Bus
        } type;
    } company;
    int price;
};

namespace ranges {
template <class T, class U>
std::ostream& operator << (std::ostream& os, common_pair<T, U> const& p) {
  return os << '(' << p.first << ", " << p.second << ')';
}
}

int main(int, char *[])
{
    SmartDigraph d;
    using namespace ranges::v3;

    auto cities = d.makeVertexProperty<City>();
    auto travels = d.makeArcProperty<Travel>();
    auto weigths = d.makeArcProperty<int>();

    std::map<std::string, Vertex> name2city;
    auto getOrAddCity = [&](std::string name)
    {
        auto result = name2city.find(name);
        if(result == name2city.end())
        {
            auto city = d.addVertex();
            cities[city] = {name};
            name2city[name] = city;
            return city;
        }
        return result->second;
    };

    auto addTravel = [&](std::string from, std::string to, Travel travel)
    {
      auto cityFrom = getOrAddCity(from);
      auto cityTo = getOrAddCity(to);
      auto arc = d.addArc(cityFrom, cityTo);
      travels[arc] = travel;
      weigths[arc] = travel.price;
    };

    auto formatPrice = [](int p)
    {
        auto const div = p/100;
        auto const mod = p%100;
        const std::string modStr{std::to_string(mod)};
        return "£" + std::to_string(div) + "." + (mod < 10 ? "0" + modStr : modStr);
    };

    auto cityCompanyAndPrice = view::transform([&](Arc arc)
    {
       return cities[d.target(arc)].name + ": " + travels[arc].company.name + "(" + formatPrice(travels[arc].price) + ")";
    });

    addTravel("Cambridge", "London/Luton", {{"Greater Anglia", Travel::Company::Type::Train}, 3880});
    addTravel("Cambridge", "London/Stansted", {{"Greater Anglia", Travel::Company::Type::Train}, 1010});
    addTravel("Cambridge", "London/Liverpool Street", {{"Greater Anglia", Travel::Company::Type::Train}, 1300});
    addTravel("Cambridge", "London/King's X", {{"Greater Anglia", Travel::Company::Type::Train}, 2360});
    addTravel("London/Liverpool Street", "London/Heathrow", {{"London Underground", Travel::Company::Type::Train}, 510});
    addTravel("London/King's X", "London/Heathrow", {{"London Underground", Travel::Company::Type::Train}, 510});
    addTravel("London/Luton", "Lisbon", {{"easyJet", Travel::Company::Type::Flight}, 7800});
    addTravel("London/Stansted", "Lisbon", {{"RyanAir", Travel::Company::Type::Flight}, 9400});
    addTravel("Lisbon", "Paris", {{"RyanAir", Travel::Company::Type::Flight}, 1900});
    addTravel("Paris", "Vienna", {{"Air France", Travel::Company::Type::Flight}, 4100});
    addTravel("Vienna", "Budapest", {{"Flixbus", Travel::Company::Type::Bus}, 1200});
    addTravel("Budapest", "Prague", {{"Flixbus", Travel::Company::Type::Bus}, 1700});
    addTravel("Prague", "Hamburg", {{"Eurowings", Travel::Company::Type::Flight}, 3638});
    addTravel("Prague", "Frankfurt", {{"Czech Airlines", Travel::Company::Type::Flight}, 4400});
    addTravel("Hamburg", "Bremen", {{"Trainline", Travel::Company::Type::Train}, 1948});
    addTravel("Frankfurt", "Bremen", {{"Deutsche Bahn", Travel::Company::Type::Train}, 1626});
    addTravel("Bremen", "Amsterdam", {{"Flixbus", Travel::Company::Type::Bus}, 1960});
    addTravel("Amsterdam", "Copenhagen", {{"Norwegian", Travel::Company::Type::Flight}, 6000});
    addTravel("Copenhagen", "London/Luton Return", {{"RyanAir", Travel::Company::Type::Flight}, 1716});
    addTravel("Copenhagen", "London/Heathrow Return", {{"SAS", Travel::Company::Type::Flight}, 3866});
    addTravel("Copenhagen", "London/Stansted Return", {{"easyJet", Travel::Company::Type::Flight}, 2221});
    addTravel("London/Stansted Return", "Cambridge Return", {{"Greater Anglia", Travel::Company::Type::Train}, 1010});

    Dijkstra<SmartDigraph> dijkstra(d, weigths);
    dijkstra.run(getOrAddCity("Cambridge"));

    auto target = getOrAddCity("Cambridge Return");

    ShortestPathView<SmartDigraph> shortestPath{dijkstra, target};
    const std::vector<std::string> output = (shortestPath | cityCompanyAndPrice);

    std::cout << "Best Trip (Total = " << formatPrice(shortestPath.weight()) << "): " << std::endl;
    std::cout << (output | view::reverse) << std::endl;
    return 0;
}
