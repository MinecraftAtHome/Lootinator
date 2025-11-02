#ifndef LOOTINATOR_UTILITY_ENUM_BIMAP_H
#define LOOTINATOR_UTILITY_ENUM_BIMAP_H

#include <unordered_map>
#include <string>

namespace util {
    template<typename T>
    class EnumToStringBimap {
        std::unordered_map<T, std::string> enum_to_string;
        std::unordered_map<std::string, T> string_to_enum;

    public:
        EnumToStringBimap(std::initializer_list<std::pair<T, std::string>> il);

        bool contains_string(const std::string& str) const;
        bool contains_enum(const T value) const; 
        T lookup_string(const std::string& str) const;
        std::string lookup_enum(const T value) const; 
    };
    
    template <typename T>
    inline EnumToStringBimap<T>::EnumToStringBimap(std::initializer_list<std::pair<T, std::string>> il)
    {
        for (const auto& elem : il) {
            enum_to_string.emplace(elem.first, elem.second);
            string_to_enum.emplace(elem.second, elem.first);
        }
    }

    template <typename T>
    inline bool EnumToStringBimap<T>::contains_string(const std::string &str) const
    {
        return string_to_enum.find(str) != string_to_enum.end();
    }

    template <typename T>
    inline bool EnumToStringBimap<T>::contains_enum(const T value) const
    {
        return enum_to_string.find(value) != enum_to_string.end();
    }

    template <typename T>
    inline T EnumToStringBimap<T>::lookup_string(const std::string &str) const
    {
        return string_to_enum.at(str);
    }

    template <typename T>
    inline std::string EnumToStringBimap<T>::lookup_enum(const T value) const
    {
        return enum_to_string.at(value);
    }
}

#endif