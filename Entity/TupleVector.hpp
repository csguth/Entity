#ifndef TUPLEVECTOR_HPP
#define TUPLEVECTOR_HPP

#include <vector>
#include <functional>

namespace Entity
{

template<uint32_t offset, class ...Types>
struct TupleVectorTraits
{
    static constexpr std::size_t beginByte(const std::vector<std::tuple<Types...>>& data)
    {
        auto capacity    = data.capacity();
        auto beginOfPrev = TupleVectorTraits<offset-1, Types...>::beginByte(data);
        return beginOfPrev + capacity * sizeof(typename std::tuple_element<offset-1, std::tuple<Types...>>::type);
    }
    static constexpr std::size_t endByte(const std::vector<std::tuple<Types...>>& data)
    {
        return TupleVectorTraits<offset, Types...>::beginByte(data) + data.size() * sizeof(typename std::tuple_element<offset, std::tuple<Types...>>::type);
    }
    static constexpr void copy(const std::vector<std::tuple<Types...>>& orig, std::vector<std::tuple<Types...>>& dest)
    {
        auto origData = reinterpret_cast<const uint8_t*>(orig.data());
        auto destData = reinterpret_cast<uint8_t*>(dest.data());
        std::copy(origData + beginByte(orig), origData + endByte(orig), destData + beginByte(dest));
        TupleVectorTraits<offset-1, Types...>::copy(orig, dest);
    }
};

template<class ...Types>
struct TupleVectorTraits<0, Types...>
{
    static constexpr std::size_t beginByte(const std::vector<std::tuple<Types...>>&)
    {
        return 0;
    }
    static constexpr std::size_t endByte(const std::vector<std::tuple<Types...>>& data)
    {
        return data.size() * sizeof(typename std::tuple_element<0, std::tuple<Types...>>::type);
    }
    static constexpr void forEach(std::function<void(std::size_t)> callable)
    {
        callable(0);
    }
    static constexpr void copy(const std::vector<std::tuple<Types...>>& orig, std::vector<std::tuple<Types...>>& dest)
    {
        auto origData = reinterpret_cast<const uint8_t*>(orig.data());
        auto destData = reinterpret_cast<uint8_t*>(dest.data());
        std::copy(origData + beginByte(orig), origData + endByte(orig), destData + beginByte(dest));
    }
};

template <class ... Types>
class TupleVector
{
public:
    TupleVector():
        m_actualSize(0)
    {

    }
    TupleVector(std::size_t initialSize):
        m_data(initialSize),
        m_actualSize(initialSize)
    {

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
        return m_data.size();
    }
    void reserve(std::size_t newCapacity)
    {
        if(newCapacity > capacity())
        {
            TupleVector<Types...> temp(newCapacity);
            TupleVectorTraits<sizeof...(Types)-1, Types...>::copy(m_data, temp.m_data);
            m_data.swap(temp.m_data);
        }
    }
    void resize(std::size_t newSize)
    {
        reserve(newSize);
        m_actualSize = newSize;
    }
    void clear()
    {
        TupleVector<Types...> temp;
        std::swap(*this, temp);
    }
    template <uint32_t offset, class ValueType>
    void set(std::size_t index, ValueType value)
    {
        const auto asBytes = reinterpret_cast<uint8_t*>(m_data.data());
        reinterpret_cast<ValueType*>(std::addressof(asBytes[beginByte<offset>()]))[index] = value;
    }
    template <uint32_t offset>
    typename std::tuple_element<offset, std::tuple<Types...>>::type at(std::size_t index) const
    {
        using ReturnType = typename std::tuple_element<offset, std::tuple<Types...>>::type;
        const auto asBytes = reinterpret_cast<const uint8_t*>(m_data.data());
        return reinterpret_cast<const ReturnType*>(std::addressof(asBytes[beginByte<offset>()]))[index];
    }
    template <uint32_t offset>
    constexpr std::size_t endByte() const
    {
        return TupleVectorTraits<offset, Types...>::endByte(m_data);
    }
    template <uint32_t offset>
    constexpr std::size_t beginByte() const
    {
        return TupleVectorTraits<offset, Types...>::beginByte(m_data);
    }

private:
    std::vector<std::tuple<Types...>> m_data;
    std::size_t                       m_actualSize;

};


}

#endif // TUPLEVECTOR_HPP
