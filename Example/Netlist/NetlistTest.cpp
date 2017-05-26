#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "Netlist.hpp"

struct Decl
{
    ModuleDecl module;
    InputPort inp1, inp2, clk;
    OutputPort out;
    Wire inp1n, inp2n, clkn, outn;
};

struct SimpleFixture
{
    Decl addModuleDecl(std::string name)
    {
        ModuleDecl decl;
        return {
            decl = nl.addOrGetModuleDecl(name),
            nl.addOrGetInputPort(decl, "inp1"),
            nl.addOrGetInputPort(decl, "inp2"),
            nl.addOrGetInputPort(decl, "clk"),
            nl.addOrGetOutputPort(decl, "out"),
            nl.addOrGetWire(decl, "inp1"),
            nl.addOrGetWire(decl, "inp2"),
            nl.addOrGetWire(decl, "clk"),
            nl.addOrGetWire(decl, "out")
        };
    }

    SimpleFixture()
    {
        simple = addModuleDecl("simple");
    }

    Netlist nl;
    Decl simple;
};

#include <iostream>

void print(Netlist& nl)
{
    ranges::for_each(nl.moduleDecls(), [&](auto decl)
    {
        using namespace ranges;
        auto getNameWithoutParentName = [&](auto el)
        {
            auto str = nl.name(el);
            return str.substr(str.find_last_of('.') + 1);
        };

        auto name = view::transform(getNameWithoutParentName);

        auto prepend = [](std::string prefix)
        {
            return view::transform([=](auto str2)
            {
                return prefix + str2;
            });
        };

        auto append = [](std::string suffix)
        {
            return view::transform([=](auto str2)
            {
                return str2 + suffix;
            });
        };

        auto instAndName = view::transform([&](auto inst)
        {
            return nl.name(nl.decl(inst)) + " " + getNameWithoutParentName(inst);
        });

        std::cout << "module " << nl.name(decl) << " (";
        auto rng = view::concat((nl.inputPorts(decl) | name),
                                (nl.outputPorts(decl) | name));
        ranges::copy(view::intersperse(rng, ", "),
                     ostream_iterator<std::string>(std::cout));
        std::cout << ")\n\n";
        auto inputPorts = nl.inputPorts(decl) | name | prepend("input ") | append(";");
        auto outputPorts = nl.outputPorts(decl) | name | prepend("output ") | append(";");
        ranges::copy(view::intersperse(inputPorts, "\n"),
                     ostream_iterator<std::string>(std::cout));
        std::cout << "\n\n";
        ranges::copy(view::intersperse(outputPorts, "\n"),
                     ostream_iterator<std::string>(std::cout));
        std::cout << "\n\n";
        if (nl.instsSize(decl))
        {
            ranges::copy(view::intersperse(nl.insts(decl) | instAndName, "\n"),
                         ostream_iterator<std::string>(std::cout));
            std::cout << "\n\n";
        }
        std::cout << "endmodule\n";
    });
}

TEST_CASE_METHOD(SimpleFixture, "Add module decl")
{
    CHECK(nl.addOrGetModuleDecl("simple") == simple.module);
    CHECK(nl.addOrGetInputPort(simple.module, "inp1") == simple.inp1);
    CHECK(nl.addOrGetInputPort(simple.module, "inp2") == simple.inp2);
    CHECK(nl.addOrGetInputPort(simple.module, "clk") == simple.clk);
    CHECK(nl.addOrGetOutputPort(simple.module, "out") == simple.out);
    CHECK(nl.inputPortsSize(simple.module) == 3);
    CHECK(nl.outputPortsSize(simple.module) == 1);
    CHECK(nl.moduleDeclsSize() == 1);
    CHECK(nl.moduleInstsSize() == 0);
    CHECK(nl.wiresSize(simple.module) == 4);
    CHECK(nl.addOrGetWire(simple.module, "inp1") == simple.inp1n);
    CHECK(nl.addOrGetWire(simple.module, "inp2") == simple.inp2n);
    CHECK(nl.addOrGetWire(simple.module, "clk") == simple.clkn);
    CHECK(nl.addOrGetWire(simple.module, "out") == simple.outn);
    CHECK(nl.topLevel() == simple.module);

    print(nl);
}

TEST_CASE_METHOD(SimpleFixture, "Add module inst")
{
    auto u1 = nl.addOrGetModuleInst(simple.module, simple.module, "u1");
    CHECK(nl.moduleInstsSize() == 1);
    CHECK(nl.decl(u1) == simple.module);
    CHECK(nl.name(u1) == "simple.u1");
    CHECK(nl.parent(u1) == simple.module);

    print(nl);
}

TEST_CASE("Hierarchical")
{
    Netlist nl;
    auto top = nl.addOrGetModuleDecl("top");
    nl.addOrGetInputPort(top, "in");
    nl.addOrGetOutputPort(top, "out");
    nl.addOrGetWire(top, "in");
    nl.addOrGetWire(top, "out");

    auto inner = nl.addOrGetModuleDecl("inner");
    auto innerIn = nl.addOrGetInputPort(inner, "inIn");
    auto innerOut = nl.addOrGetOutputPort(inner, "inOut");
    nl.addOrGetWire(inner, "inIn");
    nl.addOrGetWire(inner, "inOut");

    auto inst = nl.addOrGetModuleInst(top, inner, "inst");
    CHECK(nl.name(inst) == "top.inst");
    CHECK(nl.parent(inst) == nl.topLevel());

    print(nl);
}


