#ifndef VIEWS_HPP
#define VIEWS_HPP

#include <range/v3/all.hpp>
#include <boost/variant/get.hpp>

namespace Views
{

template <class Netlist>
auto getNameWithoutParentName(Netlist& nl)
{
    return [&](auto el)
    {
        auto str = nl.name(el);
        return str.substr(str.find_last_of('.') + 1);
    };
}

template <class Netlist>
auto name(Netlist& nl)
{
    return ranges::views::transform(getNameWithoutParentName(nl));
}

auto prepend(std::string prefix)
{
    return ranges::views::transform([=](auto str2)
    {
        return prefix + str2;
    });
}

auto append(std::string suffix)
{
    return ranges::views::transform([=](auto str2)
    {
        return str2 + suffix;
    });
}

auto preappend(std::string prefix, std::string suffix)
{
    return prepend(prefix) | append(suffix);
}



template <class Netlist>
auto instAndName(Netlist& nl)
{
    return ranges::views::transform([&](auto inst)
    {
        std::stringstream ss;
        ranges::copy((nl.ports(inst) |
                     ranges::views::transform([&](auto mappedPort)
                     {
                         auto port = nl.port(mappedPort);
                         const std::string portName = [&]()
                         {
                             if (port.type() == typeid(InputPort))
                             {
                                 return getNameWithoutParentName(nl)(boost::get<InputPort>(port));
                             }
                             return getNameWithoutParentName(nl)(boost::get<OutputPort>(port));
                         }();
                         const std::string wireName = [&]()
                         {
                            return getNameWithoutParentName(nl)(nl.wire(mappedPort));
                         }();
                         return "." + portName + "(" + wireName + ")";
                     }) |
                     ranges::views::intersperse(", ")),
                     ranges::ostream_iterator<std::string>(ss));
        return nl.name(nl.decl(inst)) + " " + getNameWithoutParentName(nl)(inst) + " " + ss.str() + ";";
    });
}

template <typename RangeType, typename SeparatorType, typename StreamType>
auto intersperseOut(RangeType range, SeparatorType sep, StreamType& stream) -> StreamType&
{
    ranges::copy(ranges::views::intersperse(range, sep), ranges::ostream_iterator<std::string>(stream));
    return stream;
}

}

#endif // VIEWS_HPP
