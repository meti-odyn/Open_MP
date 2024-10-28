#include <algorithm>
#include <iostream>
#include <omp.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ranges>
#include <set>
#include <list>
#include <fstream>
#include <vector>

struct inflection_vectors {
    std::vector<std::string> prefixes;
    std::vector<std::string> suffixes;
    std::set<std::shared_ptr<std::vector<std::string>>> key_containing_vectors;
};

struct preposition {
    std::string text;
    int offset;
};

class sequence : public std::list<preposition> {
    unsigned long length_value;
    sequence() : length_value(0) {}

public:
    explicit sequence(const preposition& prep) : length_value(prep.text.length()) {
        push_back(prep);
    }

    sequence(const std::string & text, const int offset) : length_value(text.length()) {
        push_back(preposition(text, offset));
    }

    sequence(const preposition& prep1, const preposition& prep2) {
        length_value = prep1.text.length() + prep2.text.length() - prep1.offset;
        push_back(prep1);
        push_back(prep2);
    }

    [[nodiscard]] unsigned long length () const { return length_value; }

    void add (sequence & other) {
        this->insert(this->end(), other.begin(), other.end());
        this->length_value += other.length();
    }

    void insert_back (preposition & key) {
        this->push_back(key);
        this->length_value += key.text.length() - key.offset;
    }

    void insert_front (preposition & key) {
        this->push_front(key);
        this->length_value += key.text.length() - key.offset;
    }

};

class sequence_list : public std::vector<sequence> {

    std::map<std::string, sequence&> last_texts_in_lists;
    std::map<std::string, sequence&> first_texts_in_lists;

public:
    sequence_list() = default;

    void sort_by_size () {
        std::sort(this->begin(), this->end(), [](const sequence& a,
            const sequence& b) {return a.length() > b.length();} );
    }

    void add (const sequence & value) {
        this ->push_back(value);
        last_texts_in_lists[value.back().text] = value;
        first_texts_in_lists[value.front().text] = value;
    }

    std::list<preposition> & get_by_last_text (const std::string &key) {
        return last_texts_in_lists[key];
    }

    std::list<preposition> & get_by_first_text (const std::string & key) {
        return first_texts_in_lists[key];
    }

    bool is_sequence_last_key (const std::string &last_text) {
        return last_texts_in_lists.contains(last_text);
    }

    bool is_sequence_first_key (const std::string &first_text) {
        return first_texts_in_lists.contains(first_text);
    }

    void join_sequences (sequence& seq, sequence& joining_sequence) {
        last_texts_in_lists.erase(seq.back().text);
        this->erase(std::find(this->begin(), this->end(), joining_sequence));
        //erase(sequences.begin() + otherKey);
        seq.add(joining_sequence);
    }

    void join_sequences( int key, int other_key) {
        if( key > -1 and key < this->size() and other_key > -1 and other_key < this->size() ) {
            join_sequences(at(key), at(other_key));
        }
    }
};

class inflection_map : public std::map<std::string, inflection_vectors> {

    public:

    inflection_map() = default;

    void remove (std::string &key) {

        if (const auto it = this->find(key); it != this->end()) {
            for (auto& vector : it->second.key_containing_vectors) {
                vector->erase(std::ranges::remove(*vector, key).begin(), vector->end());
            }
            this->erase(it);
        }
    }

    bool has_only_one_prefix_that_has_only_one_suffix (const std::string& key) {
        std::vector<std::string>& prefixes = this->at(key).prefixes;
        return prefixes.size() == 1 and this->at(prefixes.front()).suffixes.size() == 1;
    }

    bool has_only_one_suffix_that_has_only_one_prefix (const std::string& key) {
        std::vector<std::string>& suffixes = this->at(key).suffixes;
        return suffixes.size() == 1 and this->at(suffixes.front()).prefixes.size() == 1;
    }
};

class sequence_builder {

    public:
    explicit sequence_builder(std::vector<std::string> words) : word_list(std::move(words)) {}

private:

    std::vector<std::string> word_list;
    inflection_map groups = inflection_map();
    sequence_list sequences = sequence_list();

    static auto find_matching_inflection (bool (*compare_function) (const std::string&, const std::string&, int),
        const std::string& it, int offset, const std::vector<std::string>& spectrum) {

        auto range = spectrum | std::views::filter([&it, offset, compare_function](const std::string& subseq) {
            return compare_function(it, subseq ,offset);
        });
        return std::vector(range.begin(), range.end());
    }

    static bool is_prefix (const std::string& prefix, const std::string& word, int offset) {
        if (prefix.length() < offset or word.length() < offset) return false;
        return prefix.substr( offset, prefix.length() - offset) == word.substr(0, word.length() - offset);
    }

    static bool is_suffix (const std::string& suffix, const std::string& word, int offset) {
        return is_prefix(word, suffix, offset);
    }


public:
    std::vector<std::string> build_sequences_naive(const int offset = 1) {

        std::set<std::string> to_omit;
        std::set<std::string> sequences_ends;

        for (const auto &word: groups | std::views::keys) {

            if(to_omit.contains(word)) {
                continue;
            }
            sequence subsequence = sequence(word, offset);

            while (!groups[subsequence.front().text].prefixes.empty()) {
                std::string prefix = groups[subsequence.front().text].prefixes.front();
                to_omit.emplace(subsequence.front().text);
                subsequence.emplace_front(prefix, offset);
            }

            while (!groups[subsequence.back().text].suffixes.empty()) {
                std::string suffix = groups[subsequence.back().text].suffixes.front();
                to_omit.emplace(subsequence.back().text);
                subsequence.emplace_back(suffix, offset);
            }
            sequences.add(subsequence);
            sequences_ends.emplace(subsequence.front().text);
            sequences_ends.emplace(subsequence.back().text);
        }
        return {sequences_ends.begin(), sequences_ends.end()};
    }

    void find_sequences(int offset = 1) {

        for (auto & [word, vectors] : groups) {
            if(groups.has_only_one_prefix_that_has_only_one_suffix(word)) {
                sequence subsequence = sequence(
                    preposition(vectors.prefixes.front(), offset),
                    preposition(word, offset)
                );

                # pragma omp parallel sections
                {
                    #pragma omp section
                    while (groups.has_only_one_prefix_that_has_only_one_suffix(subsequence.front().text)) {
                        std::string prefix = groups[subsequence.front().text].prefixes.front();
                        groups.remove(subsequence.front().text);
                        subsequence.emplace_front(prefix, offset);
                    }

                    #pragma omp section
                    while (groups.has_only_one_suffix_that_has_only_one_prefix(subsequence.back().text)) {
                        std::string suffix = groups[subsequence.back().text].suffixes.front();
                        groups.remove(subsequence.back().text);
                        subsequence.emplace_back(suffix, offset);
                    }
                }
                sequences.add(subsequence);
            }
        }
    }

    void join_sequences(const std::vector<sequence &> & subsequences, int offset = 1) {
        auto & sequence = subsequences.front();

        for (auto subsequence = subsequences.begin() + 1; subsequence != subsequences.end(); subsequence++) {
            std::string first = (*subsequence).front().text;
            std::string last = sequence.back().text;
            groups.remove(last);
            groups.remove(first);
            //TODO
            //potential vulnerability:
            sequences.join_sequences(sequence, *subsequence);
        }

    }


    void fill_groups(const std::vector<std::string>& spectrum, const int offset = 1) {

        #pragma omp parallel for
        for (const std::string& it : spectrum) {

            groups[it] = inflection_vectors{
                find_matching_inflection(is_prefix, it, offset, spectrum),
                std::vector<std::string>(),
                std::set<std::shared_ptr<std::vector<std::string>>>()
            };

        }

        #pragma omp parallel for
        for (const std::string& word : spectrum) {
            auto shared_prefix = std::make_shared<std::vector<std::string>>(groups[word].prefixes);

            for (const auto& prefix : groups[word].prefixes) {
                // czy te zapisy są równoległe??? # omp paralell
                groups[prefix].key_containing_vectors.emplace(shared_prefix);
                groups[prefix].suffixes.push_back(word);
                groups[word].key_containing_vectors.emplace(std::make_shared<std::vector<std::string>>(groups[prefix].suffixes));
            }
        }

    }

    void fill_groups(const int offset = 1) {
        fill_groups(word_list, offset);
    }

    void glueSubsequences(int offset, int key, std::vector<std::string>& sequences_extremes, const long max_length) {
        int currentKey = key;

        for (int otherKey = 0; otherKey < sequences.size() and sequences.size() > 1; otherKey++) {
            auto suffixes = groups[sequences[currentKey].back().text].suffixes;

            if (currentKey != otherKey and
                std::find(suffixes.begin(), suffixes.end(), otherKey) != suffixes.end() and
                max_length > sequences[currentKey].length() - offset + sequences[otherKey].length() ) {

                std::erase(sequences_extremes, sequences[otherKey].front().text);
                std::erase(sequences_extremes, sequences[currentKey].back().text);
                sequences.join_sequences(currentKey, otherKey);

                if (otherKey < currentKey) {
                    currentKey--;
                }
                otherKey = -1;
                }
        }

    }

    void run (const int max_offsset, const long max_length) {
        fill_groups();
        std::vector<std::string> sequences_extremes = build_sequences_naive();
        for(int offset = 2 ; offset < max_offsset and sequences_extremes.size() > 2; offset++) {
            fill_groups(sequences_extremes, offset);
            sequences.sort_by_size();
            for (int index = 0; index < sequences.size(); index++) {
                glueSubsequences(offset, index, sequences_extremes, max_length);
            }
        }
        sequences.sort_by_size();
    }
};

int main() {
    std::vector<std::string> lines;
    std::string line;
    std::ifstream file("10.500-100.txt");

    if (file.is_open()) {
        while (getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
    }
    sequence_builder builder (lines);
    builder.run(6,500);


    return 0;
}
