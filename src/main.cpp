#include "snes.hpp"
#include <string>
#include <sstream>
#include <iomanip>

int main(int argc, char* argv[]) {
	std::string directory = std::string("rom/") + argv[1] + ".sfc";
	SNES snes = SNES();
	snes.load_cartridge(directory, argv[1]);
	snes.run();
}