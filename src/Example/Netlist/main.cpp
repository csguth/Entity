#include <iostream>
#include <Entity/Core/SystemWithDeletion.hpp>
#include <Entity/Core/KeyWrapper.hpp>
#include <Entity/Core/Composition.hpp>
#include <deque>

using namespace Entity;

ENTITY_ENTITY_DECLARATION(Pin);
ENTITY_ENTITY_DECLARATION(Net);
ENTITY_ENTITY_DECLARATION(CellInst);
ENTITY_ENTITY_DECLARATION(CellDecl);

class Netlist final
{
public:
    Netlist() :
        m_cellInsts(),
        m_nets(),
        m_pins(),
        m_pinsWeakAdapter(m_pins),
        m_cellInstsMap(m_cellInsts),
        m_netsMap(m_nets),
        m_pinsMap(m_pins),
        m_cellsPins(m_cellInsts, m_pins),
        m_netsPins(m_nets, m_pinsWeakAdapter)
    {

    }

    ~Netlist()
    {

    }

    CellInst addOrGetCell(std::string name)
    {
        return m_cellInstsMap.addOrGet(name);
    }

    Net addOrGetNet(std::string name)
    {
        return m_netsMap.addOrGet(name);
    }

    Pin addOrGetPin(CellInst cell, std::string name)
    {
        auto fullName = m_cellInstsMap.key(cell) + ":" + std::move(name);
        auto pin = m_pinsMap.addOrGet(fullName);
        m_cellsPins.addChild(cell, pin);
        return pin;
    }

    void connect(Net net, Pin pin)
    {
        m_netsPins.addChild(net, pin);
    }

    void disconnect(Pin pin)
    {
        m_netsPins.removeChild(m_netsPins.parent(pin), pin);
    }

    Net net(Pin pin)
    {
        return m_netsPins.parent(pin);
    }

    auto pins(Net net)
    {
        return m_netsPins.children(net);
    }

    auto pins(CellInst cell)
    {
        return m_cellsPins.children(cell);
    }

    std::string name(Pin pin) const
    {
        return m_pinsMap.key(pin);
    }

    std::string name(CellInst cell) const
    {
        return m_cellInstsMap.key(cell);
    }

    std::string name(Net net) const
    {
        return m_netsMap.key(net);
    }

private:
    SystemWithDeletion<CellInst> m_cellInsts;
    SystemWithDeletion<Net> m_nets;
    SystemWithDeletion<Pin> m_pins;
    WeakAdapter<Pin> m_pinsWeakAdapter;

    decltype(makeKeyWrapper<std::string>(m_cellInsts)) m_cellInstsMap;
    decltype(makeKeyWrapper<std::string>(m_nets)) m_netsMap;
    decltype(makeKeyWrapper<std::string>(m_pins)) m_pinsMap;

    decltype(makeComposition<Both>(m_cellInsts, m_pins)) m_cellsPins;
    decltype(makeComposition<Both>(m_nets, m_pinsWeakAdapter)) m_netsPins;

};

struct Cell2
{
    CellInst c;
    Pin a, b, z;
};

struct Cell1
{
    CellInst c;
    Pin a, z;
};

struct Flop
{
    CellInst c;
    Pin ck, d, q;
};

int main(int argc, char *argv[])
{
/*       u1
        +---+    u2      ff       u3
inp1 +-^+   |n1 +---+   +---+    +---+
        | x +--^+   |n2 |   |n3  |   |
inp2 +-^+   |   | + +--^+ F +-+--> ! +--> out
        +---+ +->   | +->   | |  |   |
              | +---+ | +---+ |  +---+
 clk +----------------+       |
              +---------------+
*/

    Netlist nl;

    auto addCell1 = [&](std::string name) -> Cell1
    {
        auto cell = nl.addOrGetCell(std::move(name));
        return {cell, nl.addOrGetPin(cell, "a"), nl.addOrGetPin(cell, "z")};
    };

    auto addCell2 = [&](std::string name) -> Cell2
    {
        auto cell = nl.addOrGetCell(std::move(name));
        return {cell, nl.addOrGetPin(cell, "a"), nl.addOrGetPin(cell, "b"), nl.addOrGetPin(cell, "z")};
    };

    auto addFlop = [&](std::string name) -> Flop
    {
        auto cell = nl.addOrGetCell(std::move(name));
        return {cell, nl.addOrGetPin(cell, "ck"), nl.addOrGetPin(cell, "d"), nl.addOrGetPin(cell, "q")};
    };

    auto addPI = [&](std::string name) -> Net
    {
        auto net = nl.addOrGetNet(name);
        auto pin = nl.addOrGetPin(nl.addOrGetCell("!PI"), std::move(name));
        nl.connect(net, pin);
        return net;
    };

    auto addPO = [&](std::string name) -> Net
    {
        auto net = nl.addOrGetNet(name);
        auto pin = nl.addOrGetPin(nl.addOrGetCell("!PO"), std::move(name));
        nl.connect(net, pin);
        return net;
    };

    auto inp1 = addPI("inp1");
    auto inp2 = addPI("inp2");
    auto clk = addPI("clk");
    auto out = addPO("out");
    auto n1 = nl.addOrGetNet("n1");
    auto n2 = nl.addOrGetNet("n2");
    auto n3 = nl.addOrGetNet("n3");

    auto u1 = addCell2("u1");
    auto u2 = addCell2("u2");
    auto u3 = addCell1("u3");
    auto ff = addFlop("ff");

    nl.connect(inp1, u1.a);
    nl.connect(inp2, u1.b);
    nl.connect(n1, u1.z);

    nl.connect(n1, u2.a);
    nl.connect(n3, u2.b);
    nl.connect(n2, u2.z);

    nl.connect(clk, ff.ck);
    nl.connect(n2, ff.d);
    nl.connect(n3, ff.q);

    nl.connect(n3, u3.a);
    nl.connect(out, u3.z);






    return 0;
}
