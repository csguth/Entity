#include <iostream>
#include <map>
#include <set>

#include <Entity/Graph/Graph.hpp>
#include <Entity/Graph/Dijkstra.hpp>
#include <Entity/Core/KeyWrapper.hpp>

using namespace Entity::Graph;

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
    using namespace ranges;
    
    auto cities = Entity::makeKeyWrapper<std::string_view>(d.m_vertices);
    auto travels = d.makeArcProperty<Travel>();
    auto weigths = d.makeArcProperty<int>();

    auto addTravel = [&](std::string_view from, std::string_view to, Travel travel)
    {
      auto cityFrom = cities.addOrGet(from);
      auto cityTo = cities.addOrGet(to);
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

    auto cityCompanyAndPrice = views::transform([&](Arc arc)
    {
        return std::string{cities.keys[d.target(arc)]} + ": " + travels[arc].company.name + "(" + formatPrice(travels[arc].price) + ")";
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
    dijkstra.run(cities.addOrGet("Cambridge"));

    auto target = cities.addOrGet("Cambridge Return");

    auto shortestPath = ShortestPathView<SmartDigraph>{dijkstra, target};
    
    std::cout << "Best Trip (Total = " << formatPrice(shortestPath.weight()) << "): " << std::endl;
    
    auto path = (std::move(shortestPath) | cityCompanyAndPrice);
    
    for (auto p : path)
    {
        std::cout << p << "\n";
    }
//    std::vector<std::string>(
    
//    std::cout << path << "\n";
    
//    auto pathVector = std::vector<std::string>(path.base.begin(), path.end().base());
//    auto reversed = std::move(pathVector) | views::reverse;
//    std::cout << reversed << "\n";
    
//    auto resultedRange =  shortestPath | cityCompanyAndPrice | views::reverse;
//    std::cout << resultedRange << std::endl;
    return 0;
}
