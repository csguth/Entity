#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <iostream>
#include "Netlist.hpp"
#include "Verilog.hpp"

TEST_CASE("Empty")
{
    Netlist nl;
    CHECK(nl.moduleDeclsSize() == 0);
    CHECK(nl.topLevel() == ModuleInst{});
}

TEST_CASE("Add module decl")
{
    Netlist nl;
    auto decl = nl.addOrGetModuleDecl("decl");
    CHECK(nl.moduleDeclsSize() == 1);
    CHECK(nl.name(decl) == "decl");
    CHECK(nl.instsSize(decl) == 0);
}

TEST_CASE("Add module inst and toplevel")
{
    Netlist nl;
    auto decl = nl.addOrGetModuleDecl("decl");
    auto inst = nl.addOrGetModuleInst(ModuleDecl{}, decl, "u1");
    CHECK(nl.name(inst) == "u1");
    CHECK(nl.instsSize(decl) == 1);
    CHECK(nl.parent(inst) == ModuleDecl{});
    CHECK(nl.decl(inst) == decl);
    CHECK(nl.topLevel() == ModuleInst{});
    nl.topLevel(inst);
    CHECK(nl.topLevel() == inst);
}

TEST_CASE("Add wire")
{
    Netlist nl;
    auto decl = nl.addOrGetModuleDecl("decl");
    auto inp = nl.addOrGetWire(decl, "inp");
    CHECK(nl.name(inp) == "decl.inp");
}

TEST_CASE("Add input")
{
    Netlist nl;
    auto decl = nl.addOrGetModuleDecl("decl");
    CHECK(nl.inputPortsSize(decl) == 0);
    CHECK(nl.wiresSize(decl) == 0);
    auto inp = nl.addOrGetInputPort(decl, "inp");
    CHECK(nl.name(nl.port(inp)) == "decl.inp");
    CHECK(nl.inputPortsSize(decl) == 1);
    CHECK(nl.wiresSize(decl) == 1);
    CHECK(nl.wire(inp) == nl.addOrGetWire(decl, "inp"));
    CHECK(nl.wiresSize(decl) == 1);
}

TEST_CASE("Add output")
{
    Netlist nl;
    auto decl = nl.addOrGetModuleDecl("decl");
    CHECK(nl.outputPortsSize(decl) == 0);
    CHECK(nl.wiresSize(decl) == 0);
    auto out = nl.addOrGetOutputPort(decl, "out");
    CHECK(nl.name(nl.port(out)) == "decl.out");
    CHECK(nl.outputPortsSize(decl) == 1);
    CHECK(nl.wiresSize(decl) == 1);
    CHECK(nl.wire(out) == nl.addOrGetWire(decl, "out"));
    CHECK(nl.wiresSize(decl) == 1);
}

TEST_CASE("Hierarchical Buffer")
{
    /*
        This test describes an hierarchical Buffer, composed of two inverters.
    */
    Netlist nl;
    auto invDecl = nl.addOrGetModuleDecl("INV");
    auto invA    = nl.addOrGetInputPort(invDecl, "A");
    auto invZ    = nl.addOrGetOutputPort(invDecl, "Z");

    auto bufDecl = nl.addOrGetModuleDecl("BUF");
    auto bufA    = nl.addOrGetInputPort(bufDecl, "A");
    auto bufZ    = nl.addOrGetOutputPort(bufDecl, "Z");

    auto inv0 = nl.addOrGetModuleInst(bufDecl, invDecl, "inv0");
    auto inv1 = nl.addOrGetModuleInst(bufDecl, invDecl, "inv1");
    auto wire = nl.addOrGetWire(bufDecl, "n0");

    nl.mapPort(inv0, nl.port(invA), nl.wire(bufA));
    nl.mapPort(inv0, nl.port(invZ), wire);
    nl.mapPort(inv1, nl.port(invA), wire);
    nl.mapPort(inv1, nl.port(invZ), nl.wire(bufZ));

    std::vector<ModuleInst> topLevel{nl.addOrGetModuleInst(ModuleDecl{}, bufDecl, "buf")};
    nl.topLevel(topLevel.front());

    ranges::for_each(ranges::views::concat(nl.children(bufDecl), topLevel), [&](ModuleInst inst)
    {
        std::cout << nl.name(inst) << " mapped ports:\n";
        ranges::for_each(nl.ports(inst), [&](MappedPort port)
        {
            std::cout << "  port " << nl.name(nl.port(port)) << " is mapped to wire " << nl.name(nl.wire(port)) << "\n";
        });
    });

}


TEST_CASE("4 bit adder")
{
    Netlist nl;
    auto halfAdderDecl = nl.addOrGetModuleDecl("HALFADDER");
    auto halfAdderA = nl.addOrGetInputPort(halfAdderDecl, "A");
    auto halfAdderB = nl.addOrGetInputPort(halfAdderDecl, "B");
    auto halfAdderS = nl.addOrGetOutputPort(halfAdderDecl, "S");
    auto halfAdderCout = nl.addOrGetOutputPort(halfAdderDecl, "Cout");
    auto fullAdderDecl = nl.addOrGetModuleDecl("FULLADDER");
    auto fullAdderA = nl.addOrGetInputPort(fullAdderDecl, "A");
    auto fullAdderB = nl.addOrGetInputPort(fullAdderDecl, "B");
    auto fullAdderCin = nl.addOrGetInputPort(fullAdderDecl, "Cin");
    auto fullAdderS = nl.addOrGetOutputPort(fullAdderDecl, "S");
    auto fullAdderCout = nl.addOrGetOutputPort(fullAdderDecl, "Cout");
    auto fourBitAdderDecl = nl.addOrGetModuleDecl("ADDER4");
    std::array<Wire, 4> A
    {
        nl.wire(nl.addOrGetInputPort(fourBitAdderDecl, "A[0]")),
        nl.wire(nl.addOrGetInputPort(fourBitAdderDecl, "A[1]")),
        nl.wire(nl.addOrGetInputPort(fourBitAdderDecl, "A[2]")),
        nl.wire(nl.addOrGetInputPort(fourBitAdderDecl, "A[3]"))
    };
    std::array<Wire, 4> B
    {
        nl.wire(nl.addOrGetInputPort(fourBitAdderDecl, "B[0]")),
        nl.wire(nl.addOrGetInputPort(fourBitAdderDecl, "B[1]")),
        nl.wire(nl.addOrGetInputPort(fourBitAdderDecl, "B[2]")),
        nl.wire(nl.addOrGetInputPort(fourBitAdderDecl, "B[3]"))
    };
    std::array<Wire, 4> S
    {
        nl.wire(nl.addOrGetOutputPort(fourBitAdderDecl, "S[0]")),
        nl.wire(nl.addOrGetOutputPort(fourBitAdderDecl, "S[1]")),
        nl.wire(nl.addOrGetOutputPort(fourBitAdderDecl, "S[2]")),
        nl.wire(nl.addOrGetOutputPort(fourBitAdderDecl, "S[3]"))
    };
    std::array<Wire, 3> Carry
    {
        nl.wire(nl.addOrGetOutputPort(fourBitAdderDecl, "Carry[0]")),
        nl.wire(nl.addOrGetOutputPort(fourBitAdderDecl, "Carry[1]")),
        nl.wire(nl.addOrGetOutputPort(fourBitAdderDecl, "Carry[2]"))
    };
    Wire Cout = nl.wire(nl.addOrGetOutputPort(fourBitAdderDecl, "Cout"));

    auto ha = nl.addOrGetModuleInst(fourBitAdderDecl, halfAdderDecl, "ha");
    nl.mapPort(ha, nl.port(halfAdderA), A[0]);
    nl.mapPort(ha, nl.port(halfAdderB), B[0]);
    nl.mapPort(ha, nl.port(halfAdderS), S[0]);
    nl.mapPort(ha, nl.port(halfAdderCout), nl.addOrGetWire(fourBitAdderDecl, "Carry[0]"));
    auto fa0 = nl.addOrGetModuleInst(fourBitAdderDecl, fullAdderDecl, "fa0");
    nl.mapPort(fa0, nl.port(fullAdderA), A[1]);
    nl.mapPort(fa0, nl.port(fullAdderB), B[1]);
    nl.mapPort(fa0, nl.port(fullAdderS), S[1]);
    nl.mapPort(fa0, nl.port(fullAdderCin), Carry[0]);
    nl.mapPort(fa0, nl.port(fullAdderCout), Carry[1]);
    auto fa1 = nl.addOrGetModuleInst(fourBitAdderDecl, fullAdderDecl, "fa1");
    nl.mapPort(fa1, nl.port(fullAdderA), A[2]);
    nl.mapPort(fa1, nl.port(fullAdderB), B[2]);
    nl.mapPort(fa1, nl.port(fullAdderS), S[2]);
    nl.mapPort(fa1, nl.port(fullAdderCin), Carry[1]);
    nl.mapPort(fa1, nl.port(fullAdderCout), Carry[2]);
    auto fa2 = nl.addOrGetModuleInst(fourBitAdderDecl, fullAdderDecl, "fa2");
    nl.mapPort(fa2, nl.port(fullAdderA), A[3]);
    nl.mapPort(fa2, nl.port(fullAdderB), B[3]);
    nl.mapPort(fa2, nl.port(fullAdderS), S[3]);
    nl.mapPort(fa2, nl.port(fullAdderCin), Carry[2]);
    nl.mapPort(fa2, nl.port(fullAdderCout), Cout);

    std::vector<ModuleInst> topLevel{nl.addOrGetModuleInst(ModuleDecl{}, fourBitAdderDecl, "adder4")};
    nl.topLevel(topLevel.front());

    ranges::for_each(ranges::views::concat(nl.children(fourBitAdderDecl), topLevel), [&](ModuleInst inst)
    {
        std::cout << nl.name(inst) << " mapped ports:\n";
        ranges::for_each(nl.ports(inst), [&](MappedPort port)
        {
            std::cout << "  port " << nl.name(nl.port(port)) << " is mapped to wire " << nl.name(nl.wire(port)) << "\n";
        });
    });
}
