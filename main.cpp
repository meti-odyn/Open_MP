#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include "inflection_map.hpp"
#include "sequence_vector.hpp"

std::vector<std::string> read_lines(const std::string& filename = "10.500-100.txt") {
    std::ifstream input_file(filename);
    std::vector<std::string> lines;
    if (input_file) {
        std::string line_buffer;
        while (std::getline(input_file, line_buffer)) lines.push_back(line_buffer);
    }
    else {
        throw std::runtime_error("Cannot open " + filename + "\n");
    }
    return lines;
}

int main() {

    unsigned int origin_sequence_length = 400;
    std::vector<std::string> words = read_lines();
    sequence_vector sequences(words, origin_sequence_length);

    for(int offset = 2; offset < 10; ++offset) {
        sequences.sort();
        if(sequences.get_first_length() >= origin_sequence_length) {
            break;
        }
        inflection_map map(sequences.backs(),sequences.fronts(), offset);
        sequences.merge(map, offset);
    }

    std::string result = sequences.get_first();
    if(result.length() > origin_sequence_length) {
        result = result.substr(0, origin_sequence_length);
    }
    std::cout << result << std::endl << "result length: " << result.length() << std::endl;

    return 0;
}

