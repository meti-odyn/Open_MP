#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP
#include <deque>
#include <string>

class sequence {

    struct fragment {
        std::string text;
        int offset;
    };

    std::deque<fragment> fragments;
    unsigned int length_value;

    public:

    explicit sequence(const std::string& text) ;

    sequence(const std::string& first, const std::string& second, int offset = 1);

    void add_front(const std::string& text, int offset);

    void add_back(const std::string& text, int offset);

    void join(const sequence& other, int offset);

    [[nodiscard]] std::string to_string() const;

    [[nodiscard]] bool operator==(const sequence& other) const;

    [[nodiscard]] std::string front() const { return fragments.front().text; }

    [[nodiscard]] std::string back() const { return fragments.back().text; }

    [[nodiscard]] unsigned int length() const { return length_value; }

    [[nodiscard]] auto size() const { return fragments.size(); }

    [[nodiscard]] unsigned int get_skipped_count(unsigned int length) const;
};

#endif //SEQUENCE_HPP
