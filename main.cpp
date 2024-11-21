#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include "inflection_map.hpp"
#include "sequence_vector.hpp"
#include <chrono>
#include <filesystem>

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

struct measurements {
    unsigned int skipped_count;
    std::chrono::microseconds duration;
    int count;
};

void process_file(const std::string& filename, unsigned int origin_sequence_length, int l = 10) {
    std::map<std::string,measurements> results;
    for(auto iteration = 0; iteration < l; ++iteration) {
        std::vector<std::string> words = read_lines(filename);
        auto start = std::chrono::high_resolution_clock::now();
        sequence_vector sequences(words, origin_sequence_length);

        for(int offset = 2; offset < 10 and sequences.size() > 1; ++offset) {
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
        auto end = std::chrono::high_resolution_clock::now();
        if (results.contains(result)) {
            results[result].count++;
            results[result].duration += std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        }
        else {
            results.emplace(result, measurements{
                                sequences.get_first_skipped_count(origin_sequence_length),
                                std::chrono::duration_cast<std::chrono::microseconds>(end - start),
                                1
                            });
        }
    }
    std::cout << "liczba znalezionych rozwiązań : " << results.size() << '\t'
    << "rozwiązania: " << std::endl;
    for(const auto &[sequence, measurement] : results) {
        double accuracy = origin_sequence_length - measurement.skipped_count - l + 1;
        accuracy /= origin_sequence_length - l + 1;
        std::cout<< "Czas(mikrosekundy): " << measurement.duration.count() / measurement.count << '\t' << "\t jakość:" << accuracy << std::endl;
    }

}

int main() {

    for (const auto& entry : std::filesystem::directory_iterator("instancje_z_bledami_negatywnymi_wynikajacymi_z_powtorzen")) {
        if (!entry.is_regular_file()) continue;
        process_file(entry.path(), 500, 10);
    }
    std::cout << std::endl;
    for (const auto& entry : std::filesystem::directory_iterator("instancje_z_bloedami_negatywnymi_losowymi")) {
        if (!entry.is_regular_file()) continue;
        process_file(entry.path(), 500, 10);
    }
    std::cout << std::endl;
    for (const auto& entry : std::filesystem::directory_iterator("pozytywne_przeklamania_na_koncach_oligo")) {
        if (!entry.is_regular_file()) continue;
        process_file(entry.path(), 500, 10);
    }
    std::cout << std::endl;
    for (const auto& entry : std::filesystem::directory_iterator("pozytywne_losowe")) {
        if (!entry.is_regular_file()) continue;
        process_file(entry.path(), 500, 10);
    }

    return 0;
}

