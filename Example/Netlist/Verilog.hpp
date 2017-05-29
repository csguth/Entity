#ifndef TOVERILOG_HPP
#define TOVERILOG_HPP

#include <iosfwd>
#include "Netlist.hpp"
#include "Views.hpp"

namespace Verilog
{

struct Indent
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
    Indent indent()
    {
        return {mIndent + 1};
    }
    std::size_t mIndent;
};

auto formatInputPorts(const Netlist& nl)
{
    return Views::name(nl) | Views::preappend("input ", ";");
}

//auto formatInputPorts(const Netlist& netlist, ModuleDecl decl, Indent indent)
//{
//    std::stringstream ss;
//    auto f = netlist.inputPorts(decl) | Views::name(netlist) | Views::preappend("input ", ";");
//    return Views::intersperseOut(,
//                                 indent.newLine(),
//                                 ss).str();
//}



    //            auto getName = name(mNetlist);
    //            auto inputPortsNames = mNetlist.inputPorts(decl) | getName
//    netlist.moduleDecls() | Views::
//    return ranges::view::transform([](auto port)
//    {
//        return Views::preappend("input ", ";");
//    });


}
//class Verilog final
//{
//public:
//    Verilog(const Netlist& nl, std::size_t indent = 0) :
//        mNetlist(nl),
//        mIndent(indent)
//    {

//    }

//    friend std::ostream& operator<<(std::ostream& out, const Verilog& v)
//    {
//        using namespace ranges;
//        using Views::intersperseOut;
//        auto modulesOut = v.mNetlist.moduleDecls() | v.formatModuleDecl();
//        return intersperseOut(modulesOut, v.newLine(), out);
//    }

//    Verilog indent(std::size_t newIndent)
//    {
//        return Verilog(mNetlist, newIndent);
//    }

//private:
//    std::string formatModuleDecl() const
//    {
//        using namespace ranges;
//        using Views::name;
//        using Views::preappend;
//        using Views::instAndName;
//        using Views::intersperseOut;
//        return view::transform([this](auto decl)
//        {
//            std::stringstream output;
//            auto getName = name(mNetlist);
//            auto inputPortsNames = mNetlist.inputPorts(decl) | getName;
//            auto outputPortsNames = mNetlist.outputPorts(decl) | getName;

//            auto moduleInterface = view::concat(inputPortsNames, outputPortsNames);
//            auto inputPorts = inputPortsNames | preappend("input ", ";");
//            auto outputPorts = outputPortsNames | preappend("output ", ";");
//            output << "module " << mNetlist.name(decl) << " (";
//            intersperseOut(moduleInterface, ", ", output);
//            output << ")" << newLines(2);
//            intersperseOut(inputPorts, newLine(), output);
//            output << newLines(2);
//            intersperseOut(outputPorts, newLine(), output);
//            output << newLines(2);
//            if (mNetlist.instsSize(decl))
//            {
//                intersperseOut(mNetlist.insts(decl) | instAndName(mNetlist), v.newLine(), output);
//                output << newLines(2);
//            }
//            output << "endmodule";
//            return output.str();
//        });
//    }



//    const Netlist& mNetlist;
//    std::size_t mIndent;

//};

#endif // TOVERILOG_HPP
