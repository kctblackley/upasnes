#include "snes.hpp"
#include "test_harness.hpp"
#include <string>
#include <sstream>
#include <iomanip>

int main(int argc, char* argv[]) {
	// --- Single-step opcode tests --------------------------------------
	// Drop a SingleStepTests-style JSON file at tests/<name>.json and call
	// test("<name>", <opcode byte>) for the Ricoh 5A22, or test_spc700(...)
	// for the SPC-700, to check it against the current implementation.
	// e.g. tests/a1_n.json + opcode 0xA1: test("a1_n", 0xA1);

	/*std::cout << "Ricoh 5A22 Native Mode\n";
	for (uint16_t i = 0x00; i <= 0xFF; i++) {
		std::stringstream ss;
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)i << "_n";

		std::string str = ss.str();
		std::cout << str << " " << "\n";
		test(str, i);
	}*/

	/*std::cout << "Ricoh 5A22 Emulation Mode\n";
	for (uint16_t i = 0x00; i <= 0xFF; i++) {
		std::stringstream ss;
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)i << "_e";

		std::string str = ss.str();

		test(str, i);
	}*/

	/*std::cout << "SPC700\n";
	for (uint16_t i = 0x00; i <= 0xFF; i++) {
		std::stringstream ss;
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)i;

		std::string str = ss.str();

		test_spc700(str, i);
	}*/


	// --- Normal emulation ------------------------------------------------
	std::string directory = std::string("rom/") + argv[1] + ".sfc";
	SNES snes = SNES();
	snes.load_cartridge(directory);
	snes.run();
}