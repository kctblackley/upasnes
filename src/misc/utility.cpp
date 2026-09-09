#include "utility.hpp"


constexpr size_t COPIER_HEADER_SIZE = 512;

bool has_copier_header(size_t file_size) {
    if (file_size <= COPIER_HEADER_SIZE) {
        return false;
    }
    return ((file_size - COPIER_HEADER_SIZE) % 0x8000) == 0;
}

std::vector<u8> load_rom(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);

    std::vector<u8> rom{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()};

    if (has_copier_header(rom.size())) {
        std::cout << "Detected " << COPIER_HEADER_SIZE
                   << "-byte copier header, skipping it\n";
        rom.erase(rom.begin(), rom.begin() + COPIER_HEADER_SIZE);
    }

    return rom;
}