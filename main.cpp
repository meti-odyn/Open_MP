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

class sequence {

    unsigned long length_value;
    std::list<preposition> prepositions;

    sequence() : length_value(0) {}

public:

    [[nodiscard]] size_t size() {
        return prepositions.size();
    }

    [[nodiscard]] const preposition& front() const {
        return prepositions.front();
    }

    [[nodiscard]] const preposition& back() const {
        return prepositions.back();
    }

    [[nodiscard]] auto begin() const {
        return prepositions.begin();
    }

    [[nodiscard]] auto end() const {
        return prepositions.end();
    }

    [[nodiscard]] bool operator==(const sequence& other) const {
        if (this->length() != other.length()) return false;
        for (auto it = this->prepositions.begin(), other_it = other.prepositions.begin();
             it != this->prepositions.end(); ++it, ++other_it) {
            if (it->text != other_it->text) return false;
        }
        return true;
    }

    explicit sequence(const preposition& prep) : length_value(prep.text.length()) {
        prepositions.push_back(prep);
    }

    sequence(const std::string& text, const int offset) : length_value(text.length()) {
        prepositions.push_back(preposition(text, offset));
    }

    sequence(const preposition& prep1, const preposition& prep2) {
        length_value = prep1.text.length() + prep2.text.length() - prep1.offset;
        prepositions.push_back(prep1);
        prepositions.push_back(prep2);
    }

    [[nodiscard]] unsigned long length() const { return length_value; }

    void add(const sequence& other) {
        this->prepositions.insert(this->prepositions.end(), other.prepositions.begin(), other.prepositions.end());
        this->length_value += other.length();
    }

    void insert_back(const preposition& key) {
        this->prepositions.push_back(key);
        this->length_value += key.text.length() - key.offset;
    }

    void insert_front(const preposition& key) {
        this->prepositions.push_front(key);
        this->length_value += key.text.length() - key.offset;
    }

    std::string to_string() const {
        std::string text = prepositions.front().text;
        bool first = true;
        for (auto preposition : this->prepositions) {
            if (first) {
                text = preposition.text;
                first = false;
            }
            else text += preposition.text;
        }
        return text;
    }

};

class sequence_list {

    std::map<std::string, std::shared_ptr<sequence>> first_texts_in_lists;
    std::map<std::string, std::shared_ptr<sequence>> last_texts_in_lists;
    std::vector<sequence> sequences;

public:

    sequence_list() = default;

    [[nodiscard]] sequence& at(int index) {
        return sequences.at(index);
    }

    [[nodiscard]] size_t size() {
        return sequences.size();
    }

    [[nodiscard]] const sequence& front() const {
        return sequences.front();
    }

    [[nodiscard]] const sequence& back() const {
        return sequences.back();
    }

    [[nodiscard]] auto begin() const {
        return sequences.begin();
    }

    [[nodiscard]] auto end() const {
        return sequences.end();
    }

    [[nodiscard]] sequence& operator[](size_t index) {
        return sequences[index];
    }

    [[nodiscard]] const sequence& operator[](size_t index) const {
        return sequences[index];
    }

    void sort_by_size() {
        std::sort(sequences.begin(), sequences.end(), [](const sequence& a,
                                                         const sequence& b) { return a.length() > b.length(); });
    }

    void add(const sequence& value) {
        sequences.push_back(value);
        *last_texts_in_lists[value.back().text] = value;
        *first_texts_in_lists[value.front().text] = value;
    }

    const sequence& get_by_last_text(const std::string& key) {
        return *last_texts_in_lists[key];
    }

    const sequence& get_by_first_text(const std::string& key) {
        return *first_texts_in_lists[key];
    }

    bool is_sequence_last_key(const std::string& last_text) {
        return last_texts_in_lists.contains(last_text);
    }

    bool is_sequence_first_key(const std::string& first_text) {
        return first_texts_in_lists.contains(first_text);
    }

    void join_sequences(sequence& seq, const sequence& joining_sequence) {
        last_texts_in_lists.erase(seq.back().text);
        sequences.erase(std::find(sequences.begin(), sequences.end(), joining_sequence));
        // erase(sequences.begin() + otherKey);
        seq.add(joining_sequence);
    }

    void join_sequences(int key, int other_key) {
        if (key > -1 and key < sequences.size() and other_key > -1 and other_key < this->size()) {
            join_sequences(at(key), at(other_key));
        }
    }
};

class inflection_map {

    std::map<std::string, inflection_vectors> inflection;

public:

    inflection_map() = default;

    void remove(const std::string& key) {

        if (const auto it = inflection.find(key); it != inflection.end()) {
            for (auto& vector: it->second.key_containing_vectors) {
                vector->erase(std::ranges::remove(*vector, key).begin(), vector->end());
            }
            inflection.erase(it);
        }
    }

    bool has_only_one_prefix_that_has_only_one_suffix(const std::string& key) {
        std::vector<std::string>& prefixes = inflection.at(key).prefixes;
        return prefixes.size() == 1 and inflection.at(prefixes.front()).suffixes.size() == 1;
    }

    bool has_only_one_suffix_that_has_only_one_prefix(const std::string& key) {
        std::vector<std::string>& suffixes = inflection.at(key).suffixes;
        return suffixes.size() == 1 and inflection.at(suffixes.front()).prefixes.size() == 1;
    }

    auto keys() {
        return inflection | std::views::keys;
    }

    inflection_vectors& operator[](const std::string& key) {
        return inflection[key];
    }

    auto begin() {
        return inflection.begin();
    }

    auto end() {
        return inflection.end();
    }
};

class sequence_builder {

public:

    explicit sequence_builder(std::vector<std::string> words) : word_list(std::move(words)) {}

private:

    std::vector<std::string> word_list;
    inflection_map groups = inflection_map();
    sequence_list sequences = sequence_list();

    static auto find_matching_inflection(bool (* compare_function)(const std::string&, const std::string&, int),
                                         const std::string& it, int offset, const std::vector<std::string>& spectrum) {

        auto range = spectrum | std::views::filter([&it, offset, compare_function](const std::string& subseq) {
            return compare_function(it, subseq, offset);
        });
        return std::vector(range.begin(), range.end());
    }

    static bool is_prefix(const std::string& prefix, const std::string& word, int offset) {
        if (prefix.length() < offset or word.length() < offset) return false;
        return prefix.substr(offset, prefix.length() - offset) == word.substr(0, word.length() - offset);
    }

    static bool is_suffix(const std::string& suffix, const std::string& word, int offset) {
        return is_prefix(word, suffix, offset);
    }


public:

    std::vector<std::string> build_sequences_naive(const int offset = 1) {

        std::set<std::string> to_omit;
        std::set<std::string> sequences_ends;

        for (const auto& word: groups.keys()) {

            if (to_omit.contains(word)) {
                continue;
            }
            sequence subsequence = sequence(word, offset);

            while (!groups[subsequence.front().text].prefixes.empty()) {
                std::string prefix = groups[subsequence.front().text].prefixes.front();
                to_omit.emplace(subsequence.front().text);
                subsequence.insert_front(preposition(prefix, offset));
            }

            while (!groups[subsequence.back().text].suffixes.empty()) {
                std::string suffix = groups[subsequence.back().text].suffixes.front();
                to_omit.emplace(subsequence.back().text);
                subsequence.insert_back(preposition(suffix, offset));
            }
            sequences.add(subsequence);
            sequences_ends.emplace(subsequence.front().text);
            sequences_ends.emplace(subsequence.back().text);
        }
        return {sequences_ends.begin(), sequences_ends.end()};
    }

    void find_sequences(int offset = 1) {

        for (auto& [word, vectors]: groups) {
            if (groups.has_only_one_prefix_that_has_only_one_suffix(word)) {
                sequence subsequence = sequence(
                        preposition(vectors.prefixes.front(), offset),
                        preposition(word, offset)
                );

                #pragma omp parallel sections
                {
                    #pragma omp section
                    while (groups.has_only_one_prefix_that_has_only_one_suffix(subsequence.front().text)) {
                        std::string prefix = groups[subsequence.front().text].prefixes.front();
                        groups.remove(subsequence.front().text);
                        subsequence.insert_front(preposition(prefix, offset));
                    }

                    #pragma omp section
                    while (groups.has_only_one_suffix_that_has_only_one_prefix(subsequence.back().text)) {
                        std::string suffix = groups[subsequence.back().text].suffixes.front();
                        groups.remove(subsequence.back().text);
                        subsequence.insert_back(preposition(suffix, offset));
                    }
                }
                sequences.add(subsequence);
            }
        }
    }

    void join_sequences(std::vector<sequence>& subsequences, int offset = 1) {
        auto& sequence = subsequences.front();

        for (auto subsequence = subsequences.cbegin() + 1; subsequence != subsequences.cend(); ++subsequence) {
            std::string first = subsequence->front().text;
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
        for (const std::string& it: spectrum) {

            groups[it] = inflection_vectors{
                    find_matching_inflection(is_prefix, it, offset, spectrum),
                    std::vector<std::string>(),
                    std::set<std::shared_ptr<std::vector<std::string>>>()
            };

        }

        #pragma omp parallel for
        for (const std::string& word: spectrum) {
            auto shared_prefix = std::make_shared<std::vector<std::string>>(groups[word].prefixes);

            for (const auto& prefix: groups[word].prefixes) {
                // czy te zapisy są równoległe??? # omp paralell
                groups[prefix].key_containing_vectors.emplace(shared_prefix);
                groups[prefix].suffixes.push_back(word);
                groups[word].key_containing_vectors.emplace(
                        std::make_shared<std::vector<std::string>>(groups[prefix].suffixes));
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
                std::find(suffixes.begin(), suffixes.end(), sequences[otherKey].front().text) != suffixes.end() and
                max_length > sequences[currentKey].length() - offset + sequences[otherKey].length()) {

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

    sequence run(const int max_offsset, const long max_length) {
        fill_groups();
        std::vector<std::string> sequences_extremes = build_sequences_naive();
        for (int offset = 2; offset < max_offsset and sequences.size() > 2; offset++) {
            fill_groups(sequences_extremes, offset);
            sequences.sort_by_size();
            for (int index = 0; index < sequences.size(); index++) {
                glueSubsequences(offset, index, sequences_extremes, max_length);
            }
        }
        sequences.sort_by_size();
        return sequences.front();
    }
};

int main() {
    std::vector<std::string> lines;
    std::ifstream file;
    
    //file.open()
    if (file.open("10.500-100.txt")) {
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
    }
    else {
        std::cerr << "Unable to open file." << std::endl;
        return 1;
    }
    sequence_builder builder(lines);
    sequence result = builder.run(8, 500);
    std::cout << result.length() << std::endl;

    return 0;
}