#include <ranges>
#include <stdexcept>
#include "inflection_map.hpp"
#include <omp.h>
#include <map>

inflection_map::inflection_vectors::inflection_vectors(const auto& begin, const auto& end)
    : prefixes(std::make_shared<std::vector<std::string>>(begin, end)),
      suffixes(std::make_shared<std::vector<std::string>>()),
      keys() {}

inflection_map::inflection_vectors::inflection_vectors ()
    : prefixes(std::make_shared<std::vector<std::string>>()),
      suffixes(std::make_shared<std::vector<std::string>>()),
      keys() {}

inflection_map::inflection_map(const std::vector<std::string>& lines, uint8_t offset) {

    #pragma omp parallel for
    for (auto it = lines.begin(); it != lines.end(); ++it ) {
        const std::string &text = *it;
        auto range = lines | std::views::filter([&](const std::string &line) {
            return is_prefix(text, line, offset);
        });
        #pragma omp critical
        {
            inflection_vectors_map.emplace(text, inflection_vectors(range.begin(), range.end()));
        }
    }


    // #pragma omp parallel {
    #pragma omp parallel for
        for (const std::string& text: lines) {
            const auto prefixes_ptr = inflection_vectors_map.at(text).prefixes;
            for (const auto& prefix : *prefixes_ptr) {
                #pragma omp critical
                {
                    inflection_vectors_map.at(prefix).keys.push_back(prefixes_ptr);
                    inflection_vectors_map.at(prefix).suffixes->push_back(text);
                    inflection_vectors_map.at(text).keys.push_back(inflection_vectors_map.at(prefix).suffixes);
                }
            }
        }
    //}
}

inflection_map::inflection_map(const std::vector<std::string> &potential_prefixes, const std::vector<std::string> &potential_suffixes, uint8_t offset) {
    //potential_suffix to front - początek seq - więc on może mieć tylko prefixy i być tylko suffixem
    #pragma omp parallel for
    for (const std::string& text : potential_suffixes) {
        auto range = potential_prefixes | std::views::filter([&](const std::string& word) {
            return is_prefix(text, word, offset);
        });

        #pragma omp critical
        {
            inflection_vectors_map.emplace(text, inflection_vectors(range.begin(), range.end()));
        }
    }

#pragma omp parallel for
    for (auto it = potential_prefixes.begin(); it != potential_prefixes.end(); ++it) { //const std::string& text : potential_prefixes

        #pragma omp critical
        {
            inflection_vectors_map.emplace(*it, inflection_vectors());
        }
    }
    #pragma omp parallel for
    for (const std::string& front : potential_suffixes) {
        const auto prefixes_ptr = inflection_vectors_map.at(front).prefixes;
        for (const auto& back : *prefixes_ptr) {
            #pragma omp critical
            {
                inflection_vectors_map.at(back).keys.push_back(prefixes_ptr);
                inflection_vectors_map.at(back).suffixes->push_back(front);
                inflection_vectors_map.at(front).keys.push_back(inflection_vectors_map.at(back).suffixes);
            }
        }
    }
}


bool inflection_map::is_prefix(const std::string &prefix, const std::string &word, uint8_t offset) {
    if (prefix.length() <= offset or word.length() <= offset) return false;
    return prefix.substr(0, prefix.length() - offset) == word.substr(offset, word.length() - offset);
}


void inflection_map::remove(const std::string& key) {
    #pragma omp for
    for (auto& vector : inflection_vectors_map.at(key).keys) {
        //vector->erase(key);
        std::erase(*vector, key);
    }
    inflection_vectors_map.erase(key);
}

auto inflection_map::operator[](const std::string &key) {
    return inflection_vectors_map.at(key);
}
