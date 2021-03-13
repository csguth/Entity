#ifndef TOVERILOG_HPP
#define TOVERILOG_HPP

#include <ostream>
#include "Netlist.hpp"
#include "Views.hpp"

namespace Verilog
{

struct Indent final
{
    std::string newLines(std::size_t N) const
    {
        if (N == 0)
            return "";
        if (N == 1)
            return newLine();
        return newLines(N-1) + newLine();
    }
    std::string newLine() const
    {
        std::string result{"\n"};
        result.append(mIndent, ' ');
        return result;
    }
    Indent indent() const
    {
        return {mIndent + 1};
    }
    std::size_t mIndent;
};

auto formatInputPorts(const Netlist& nl)
{
    return Views::name(nl) | Views::preappend("input ", ";");
}

class Writer final
{
private:
    auto formatModuleDecl() const
    {
        using namespace ranges;
        using Views::name;
        using Views::preappend;
        using Views::instAndName;
        using Views::intersperseOut;
        return views::transform([this](auto decl) -> std::string
        {
            std::stringstream output;
            auto getName = name(mNetlist);
            auto inputPortsNames = mNetlist.inputPorts(decl) | getName;
            auto outputPortsNames = mNetlist.outputPorts(decl) | getName;

            auto moduleInterface = views::concat(inputPortsNames, outputPortsNames);
            auto inputPorts = inputPortsNames | preappend("input ", ";");
            auto outputPorts = outputPortsNames | preappend("output ", ";");
            output << mIndent.newLine() << "module " << mNetlist.name(decl) << " (";
            auto indent = mIndent.indent();
            intersperseOut(moduleInterface, ", ", output);
            output << ")" << indent.newLines(2);
            intersperseOut(inputPorts, indent.newLine(), output);
            output << indent.newLine();
            intersperseOut(outputPorts, indent.newLine(), output);
            if (mNetlist.childrenSize(decl))
            {
                output << indent.newLines(2);
                intersperseOut(mNetlist.children(decl) | instAndName(mNetlist), indent.newLine(), output);
            }
            output << mIndent.newLines(2);
            output << "endmodule";
            return output.str();
        });
    }

public:
    Writer(const Netlist& nl, std::size_t indent = 0) :
        mNetlist(nl),
        mIndent(Indent{indent})
    {

    }

    friend std::ostream& operator<<(std::ostream& out, const Writer& v)
    {
        using namespace ranges;
        using Views::intersperseOut;
        auto modulesOut = v.mNetlist.moduleDecls() | v.formatModuleDecl();
        return intersperseOut(modulesOut, v.mIndent.newLine(), out);
    }

    Writer indent(std::size_t newIndent)
    {
        return Writer(mNetlist, newIndent);
    }




    const Netlist& mNetlist;
    Indent mIndent;

};

}

#endif // TOVERILOG_HPP_HPP
