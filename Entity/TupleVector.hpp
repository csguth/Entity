#ifndef TUPLEVECTOR_HPP
#define TUPLEVECTOR_HPP

#include <vector>
#include <functional>
#include <algorithm>

namespace Entity
{

template<class T>
constexpr T integerCeilDivision(T x, T y)
{
    return (x % y == 0) ? x/y : (x/y) + 1;
}

template<uint32_t offset, class ...Types>
struct TupleVectorTraits
{
    using CurrentType = typename std::tuple_element<offset-1, std::tuple<Types...>>::type;
    static constexpr std::size_t wordsNeeded(std::size_t numElements)
    {
        const auto numWords = integerCeilDivision(numElements*sizeof(CurrentType), sizeof(uint32_t));
        return numWords + TupleVectorTraits<offset-1, Types...>::wordsNeeded(numElements);
    }
    static constexpr std::size_t firstWord(std::size_t capacity)
    {
        return TupleVectorTraits<offset-1, Types...>::lastWord(capacity);
    }
    static constexpr std::size_t lastWord(std::size_t capacity)
    {
        return TupleVectorTraits<offset, Types...>::firstWord(capacity) + integerCeilDivision(capacity*sizeof(CurrentType), sizeof(uint32_t));
    }
    static constexpr void copy(const std::vector<uint32_t>& origin, std::size_t originCapacity, std::vector<uint32_t>& destination, std::size_t destinationCapacity)
    {
        const auto numWords = integerCeilDivision(originCapacity*sizeof(CurrentType), sizeof(uint32_t));
        std::copy_backward(origin.begin() + firstWord(originCapacity), origin.begin() + lastWord(originCapacity), destination.begin() + firstWord(destinationCapacity) + numWords);
        TupleVectorTraits<offset-1, Types...>::copy(origin, originCapacity, destination, destinationCapacity);
    }
};

template<class ...Types>
struct TupleVectorTraits<0, Types...>
{
    static constexpr std::size_t wordsNeeded(std::size_t)
    {
        return 0;
    }
    static constexpr std::size_t firstWord(std::size_t)
    {
        return 0;
    }
    static constexpr std::size_t lastWord(std::size_t)
    {
        return 0;
    }
    static constexpr void copy(const std::vector<uint32_t>&, std::size_t, std::vector<uint32_t>&, std::size_t)
    {

    }
};

template <class ... Types>
class TupleVector
{
public:
    TupleVector(std::size_t size) :
        m_actualSize(size),
        m_capacity(size),
        m_data(TupleVectorTraits<sizeof...(Types), Types...>::wordsNeeded(size))
    {
    }
    TupleVector():
        m_actualSize(0),
        m_capacity(0),
        m_data(TupleVectorTraits<sizeof...(Types), Types...>::wordsNeeded(1))
    {

    }
    friend void swap(TupleVector& first, TupleVector& second)
    {
        using std::swap;
        swap(first.m_actualSize, second.m_actualSize);
        swap(first.m_capacity,   second.m_capacity);
        first.m_data.swap(second.m_data);
    }
    constexpr std::size_t size() const
    {
        return m_actualSize;
    }
    constexpr bool empty() const
    {
        return size() == 0;
    }
    constexpr std::size_t capacity() const
    {
        return m_capacity;
    }
    void reserve(std::size_t newCapacity)
    {
        m_data.resize(TupleVectorTraits<sizeof...(Types), Types...>::wordsNeeded(newCapacity));
        TupleVectorTraits<sizeof...(Types), Types...>::copy(m_data, m_capacity, m_data, newCapacity);
        m_capacity = newCapacity;
    }
    void resize(std::size_t newSize)
    {
        if(newSize <= m_capacity)
        {
            m_actualSize = newSize;
        }
        else
        {
            reserve(std::max(static_cast<std::size_t>(1), m_capacity*2));
            resize(newSize);
        }
    }
    void clear()
    {
        TupleVector<Types...> other;
        swap(*this, other);
    }
    template <uint32_t offset>
    void set(std::size_t index, typename std::tuple_element<offset, std::tuple<Types...>>::type value)
    {
        using ValueType = typename std::tuple_element<offset, std::tuple<Types...>>::type;
        auto addressOfTheFirstWord = std::addressof(m_data.data()[TupleVectorTraits<offset+1, Types...>::firstWord(m_capacity)]);
        reinterpret_cast<ValueType*>(addressOfTheFirstWord)[index] = value;
    }
    template <uint32_t offset>
    typename std::tuple_element<offset, std::tuple<Types...>>::type at(std::size_t index) const
    {
        using ValueType = typename std::tuple_element<offset, std::tuple<Types...>>::type;
        return reinterpret_cast<const ValueType*>(std::addressof(m_data.data()[TupleVectorTraits<offset+1, Types...>::firstWord(m_capacity)]))[index];
    }
    const std::vector<uint32_t>& data() const
    {
        return m_data;
    }

private:
    std::size_t           m_actualSize;
    std::size_t           m_capacity;
    std::vector<uint32_t> m_data;

};


}

#endif // TUPLEVECTOR_HPP
