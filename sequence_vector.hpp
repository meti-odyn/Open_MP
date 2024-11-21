#ifndef SEQUENCE_VECTOR_HPP
#define SEQUENCE_VECTOR_HPP
#include <string>
#include <map>
#include <memory>
#include <vector>
#include "sequence.hpp"
#include "inflection_map.hpp"
#include <set>

class sequence_vector {

    std::vector<std::shared_ptr<sequence>> sequences;
    std::map<std::string, std::shared_ptr<sequence>> sequences_by_front, sequences_by_back;
    uint max_sequence_length;

    void merge(const std::shared_ptr<sequence>& merge_to, const std::shared_ptr<sequence>& merged, inflection_map& map, int offset);

    public:

    sequence_vector(std::vector<std::string>& words, uint origin_length);

    void sort();

    void merge(inflection_map& map, int offset);

    [[nodiscard]] std::vector<std::string> fronts() const;

    [[nodiscard]] std::vector<std::string> backs() const;

    std::shared_ptr<sequence> get_by_front(const std::string& key) { return sequences_by_front.at(key); }

    std::shared_ptr<sequence> get_by_back(const std::string& key) { return sequences_by_back.at(key); }

    [[nodiscard]] std::string get_first () const { return sequences.front()->to_string(); }

    [[nodiscard]] unsigned int get_first_length() const { return sequences.front()->length(); }

    [[nodiscard]] auto size() const { return sequences.size(); }

    [[nodiscard]] unsigned int get_first_skipped_count (unsigned int length) { return sequences.front()->get_skipped_count(length); }
};



#endif //SEQUENCE_VECTOR_HPP
