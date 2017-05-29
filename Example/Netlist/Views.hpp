#ifndef VIEWS_HPP
#define VIEWS_HPP

#include <range/v3/all.hpp>

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
    return ranges::view::transform(getNameWithoutParentName(nl));
}

auto prepend(std::string prefix)
{
    return ranges::view::transform([=](auto str2)
    {
        return prefix + str2;
    });
}

auto append(std::string suffix)
{
    return ranges::view::transform([=](auto str2)
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
    return ranges::view::transform([&](auto inst)
    {
        return nl.name(nl.decl(inst)) + " " + getNameWithoutParentName(nl)(inst);
    });
}

template <typename RangeType, typename SeparatorType, typename StreamType>
auto intersperseOut(RangeType range, SeparatorType sep, StreamType& stream) -> StreamType&
{
    ranges::copy(ranges::view::intersperse(range, sep), ranges::ostream_iterator<std::string>(stream));
    return stream;
}

}

#endif // VIEWS_HPP
