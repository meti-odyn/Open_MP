#include <algorithm>
#include "sequence_vector.hpp"

#include <ranges>

sequence_vector::sequence_vector(std::vector<std::string>& words, uint origin_length) {
    max_sequence_length = origin_length;
    inflection_map map(words, 1);

    while(!words.empty()) {
        const std::string word = *words.begin();
        auto seq = std::make_shared<sequence>(word);
        //words.erase(word);
        std::erase(words, word);

        while (!map.get_prefixes(seq->front())->empty() and seq->length() < max_sequence_length) { //while (map.contains(seq->front()) and !map.get_prefixes(seq->front())->empty())
            const std::string prefix = *map.get_prefixes(seq->front())->begin();
            if(seq->size() > 1) {
                map.remove(seq->front());
            }
            //words.erase(prefix);
            std::erase(words, prefix);
            seq->add_front(prefix, 1);
        }

        while (!map.get_suffixes(seq->back())->empty() and seq->length() < max_sequence_length) { // while (map.contains(seq->back()) and !map.get_suffixes(seq->back())->empty())
            const std::string suffix = *map.get_suffixes(seq->back())->begin();
            if(seq->size() > 1) {
                map.remove(seq->back());
            }
            seq->add_back(suffix, 1);
            std::erase(words, suffix);
            //words.erase(suffix);
        }

        sequences.push_back(seq);
        sequences_by_back[seq->back()] = seq;
        sequences_by_front[seq->front()] = seq;
        if(seq->length() == max_sequence_length) {
            break;
        }
    }
}

void sequence_vector::sort() {
    std::sort(sequences.begin(),sequences.end(), [](const std::shared_ptr<sequence> a,const std::shared_ptr<sequence> b) {
        return a->length() == b->length() ? a->size() < b->size() : a->length() > b->length();
    });
}

void sequence_vector::merge(const std::shared_ptr<sequence>& merge_to, const std::shared_ptr<sequence>& merged, inflection_map& map, int offset) {
    if(merge_to->size() > 1) {
        map.remove(merge_to->back());
    }
    else map.get_suffixes(merge_to->back())->clear();

    if(merged->size() > 1) {
        map.remove(merged->front());
    }
    else map.get_prefixes(merged->front())->clear();

    sequences_by_back[merged->back()] = sequences_by_back[merge_to->back()];
    sequences_by_back.erase(merge_to->back());
    sequences_by_front.erase(merged->front());
    merge_to->join(*merged, offset);

    std::erase(sequences, merged);
}

void sequence_vector::merge(inflection_map& map, const int offset) {
    for (size_t i = 0; i < sequences.size(); ++i) {//ssize_t i =0
        //if (i == 0) {sort();}

        std::shared_ptr<sequence> seq = sequences[i];

        while (map.contains(seq->back()) and !map.get_suffixes(seq->back())->empty()) {
            const auto& suffixes = *map.get_suffixes(seq->back());
            std::string suffix = *std::max_element(suffixes.begin(), suffixes.end(),
                [this](const std::string& a, const std::string& b) {
                    return get_by_front(a)->length() > get_by_front(b)->length();
                }
            );
            merge(seq, get_by_front(suffixes.front()), map, offset);
            if(seq->length() >= max_sequence_length) {
                i = sequences.size();
            }
            //i = -1;
        }

        while (map.contains(seq->front()) and !map.get_prefixes(seq->front())->empty() and i < sequences.size()) {
            const auto& prefixes = *map.get_prefixes(seq->front());
            std::string prefix = *std::max_element(prefixes.begin(), prefixes.end(),
                [this](const std::string& a,const std::string& b) {
                    return get_by_back(a)->length() > get_by_back(b)->length();
                });
            auto other_seq = get_by_back(prefix);
            merge(other_seq, seq, map, offset);
            seq = other_seq;
            if(seq->length() >= max_sequence_length) {
                i = sequences.size();
            }
            //i = -1;
        }
    }
}

std::vector<std::string> sequence_vector::fronts() const {
    auto range = sequences_by_front | std::views::keys;
    std::vector<std::string> keys = std::vector(range.begin(), range.end());
    return keys;
}

std::vector<std::string> sequence_vector::backs() const {
    auto range = sequences_by_back | std::views::keys;
    std::vector<std::string> keys = std::vector(range.begin(), range.end());
    return keys;
}

