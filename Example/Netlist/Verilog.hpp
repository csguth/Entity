#ifndef TOVERILOG_HPP
#define TOVERILOG_HPP

#include <iosfwd>
#include "Netlist.hpp"
#include "Views.hpp"

class Verilog final
{
public:
    Verilog(const Netlist& nl) :
        mNetlist(nl)
    {

    }

    friend std::ostream& operator<<(std::ostream& out, const Verilog& v)
    {
        using Views::name;
        using Views::preappend;
        using Views::instAndName;
        using namespace ranges;
        ranges::for_each(v.mNetlist.moduleDecls(), [&](auto decl)
        {
            auto getName = name(v.mNetlist);
            auto inputPortsNames = v.mNetlist.inputPorts(decl) | getName;
            auto outputPortsNames = v.mNetlist.outputPorts(decl) | getName;
            auto intersperseOut = [](auto range, auto sep, auto& stream)
            {
                ranges::copy(view::intersperse(range, sep), ostream_iterator<std::string>(stream));
            };
            auto moduleInterface = view::concat(inputPortsNames, outputPortsNames);
            auto inputPorts = inputPortsNames | preappend("input ", ";");
            auto outputPorts = outputPortsNames | preappend("output ", ";");
            out << "module " << v.mNetlist.name(decl) << " (";
            intersperseOut(moduleInterface, ", ", out);
            out << ")\n\n";
            intersperseOut(inputPorts, "\n", out);
            out << "\n\n";
            intersperseOut(outputPorts, "\n", out);
            out << "\n\n";            
            if (v.mNetlist.instsSize(decl))
            {
                intersperseOut(v.mNetlist.insts(decl) | instAndName(v.mNetlist), "\n", out);
                out << "\n\n";
            }
            out << "endmodule";
        });
        return out;
    }

private:
    const Netlist& mNetlist;

};

#endif // TOVERILOG_HPP
