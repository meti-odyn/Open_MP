#ifndef INFLECTION_MAP_HPP
#define INFLECTION_MAP_HPP

#include <iostream>
#include <map>
#include <memory>
#include <vector>

class inflection_map {

    struct inflection_vectors {
        std::shared_ptr<std::vector<std::string>> prefixes, suffixes;
        std::vector<std::shared_ptr<std::vector<std::string>>> keys;

        inflection_vectors(const auto& begin, const auto& end);

        inflection_vectors();
    };

    std::map<std::string, inflection_vectors> inflection_vectors_map;

public:

    inflection_map(const std::vector<std::string>& lines, uint8_t offset);

    inflection_map(const std::vector<std::string>& potential_prefixes, const std::vector<std::string>& potential_suffixes, uint8_t offset);

    [[nodiscard]] static bool is_prefix(const std::string& prefix, const std::string& word, uint8_t offset);

    void remove(const std::string& key);

    auto operator[] (const std::string& key);

    // void clear_prefixes(const std::string& key);
    //
    // void clear_suffixes(const std::string& key);

    [[nodiscard]] std::shared_ptr<std::vector<std::string>> get_prefixes(const std::string& key) const { return inflection_vectors_map.at(key).prefixes; }

    [[nodiscard]] std::shared_ptr<std::vector<std::string>> get_suffixes(const std::string& key) const { return inflection_vectors_map.at(key).suffixes; }

    [[nodiscard]] bool contains(const std::string & line) const { return inflection_vectors_map.contains(line); }
};



#endif //INFLECTION_MAP_HPP
