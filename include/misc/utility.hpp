#pragma once

#include <cstdint>
#include <fstream>
#include <sstream>
#include <iterator>
#include <string>
#include <vector>
#include <iostream>

using Byte = uint8_t;

// For actual ROM files (this is a useless comment why did I write this?)
std::vector<Byte> load_rom(const std::string& filename);