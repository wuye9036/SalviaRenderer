#pragma once

#include <functional>
#include <vector>

#include <boost/functional/hash/hash.hpp>

namespace eflib
{
    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v)
    {
        using std::hash;

        hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <typename T>
    size_t do_hash(T const& v)
    {
        using std::hash;
        return hash<T>{}(v);
    }

    template <class It>
    inline std::size_t hash_range(It first, It last)
    {
        std::size_t seed = 0;

        for (; first != last; ++first)
        {
            hash_combine(seed, *first);
        }

        return seed;
    }

    template <class It>
    inline void hash_range(std::size_t& seed, It first, It last)
    {
        for (; first != last; ++first)
        {
            hash_combine(seed, *first);
        }
    }

    namespace hash_detail
    {
        template <std::size_t I, typename T>
        inline typename std::enable_if_t<(I == std::tuple_size<T>::value)>
            hash_combine_tuple(std::size_t&, T const&)
        {
        }

        template <std::size_t I, typename T>
        inline typename std::enable_if_t<(I < std::tuple_size<T>::value)>
            hash_combine_tuple(std::size_t& seed, T const& v)
        {
            hash_combine(seed, std::get<I>(v));
            hash_combine_tuple<I + 1>(seed, v);
        }

        template <typename T>
        inline std::size_t hash_tuple(T const& v)
        {
            std::size_t seed = 0;
            hash_combine_tuple<0>(seed, v);
            return seed;
        }
    }
}

namespace std
{
    template <typename T> struct hash<std::vector<T>>
    {
        size_t operator()(std::vector<T> const& v) const noexcept
        {
            return eflib::hash_range(v.cbegin(), v.cend());
        }
    };

    template <typename T1, typename T2>
    struct hash<std::pair<T1, T2>>
    {
        size_t operator()(std::pair<T1, T2> const& v) const noexcept
        {
            size_t seed = 0;
            eflib::hash_combine(seed, v.first);
            eflib::hash_combine(seed, v.second);
            return seed;
        }
    };

    template <typename... T>
    inline std::size_t hash_value(std::tuple<T...> const& v)
    {
        return std::hash_detail::hash_tuple(v);
    }

    template <typename ...Ts>
    struct hash<std::tuple<Ts...>>
    {
        size_t operator()(std::tuple<Ts...> const& v) const noexcept
        {
            return eflib::hash_detail::hash_tuple(v);
        }
    };
}
