#pragma once

#include <vector>

template<class Key, class Value>
class VectorMap
{
    std::vector<std::pair<Key, Value>> mData;
    using iterator = typename std::vector<std::pair<Key, Value>>::const_iterator;

    static bool kvcompare(const std::pair<Key, Value> & a, const std::pair<Key, Value> & b)
    {
        return a.first < b.first;
    }

public:
    void reserve(size_t size)
    {
        mData.reserve(size);
    }

    void add(const Key & key, const Value & value)
    {
        mData.push_back(std::make_pair(key, value));
    }

    void sort()
    {
        std::sort(mData.begin(), mData.end(), kvcompare);
    }

    void clear()
    {
        mData.clear();
    }

    iterator begin() const
    {
        return mData.cbegin();
    }

    iterator end() const
    {
        return mData.cend();
    }

    iterator find(const Key & key) const
    {
        auto search = std::make_pair(key, Value());
        auto first = std::lower_bound<>(mData.cbegin(), mData.cend(), search, kvcompare);
        if(first != mData.cend() && !kvcompare(search, *first))
            return first;
        else
            return end();
    }
};
