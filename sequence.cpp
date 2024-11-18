#include "sequence.hpp"

sequence::sequence(const std::string& text) {
    fragments = {fragment(text,0)};
    length_value = text.length();
}

sequence::sequence(const std::string& first, const std::string& second, const int offset) {
    fragments = {fragment(first,offset), fragment(second,0)};
    length_value = first.length() + second.length() - offset;
}

void sequence::add_front(const std::string &text, const int offset) {
    fragments.push_front(fragment(text,offset));
    length_value += offset;
}

void sequence::add_back(const std::string &text, const int offset) {
    fragments.back().offset = offset;
    fragments.push_back(fragment(text,0));
    length_value += offset;
}

void sequence::join(const sequence &other, const int offset) {
    fragments.back().offset = offset;
    length_value += other.length_value - offset;
    for (const auto& fragment : other.fragments) {
        fragments.push_back(fragment);
    }
}

std::string sequence::to_string() const {
    std::string result = front();
    int previous_offset = 0;
    for (const auto&[text, offset] : fragments) {
        result += text.substr(text.length() - previous_offset, previous_offset);
        previous_offset = offset;
    }
    return result;
}

bool sequence::operator==(const sequence &other) const {

    if (length() != other.length() or size() != other.size()) {
        return false;
    }

    for (auto it = fragments.begin(), other_it = other.fragments.begin();
        it != fragments.end(); ++it, ++other_it) {

        if (it->text != other_it->text) {
            return false;
        }
    }
    return true;
}
