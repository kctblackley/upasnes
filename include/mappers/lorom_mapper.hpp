#include "mapper.hpp"

#define MEGABYTE 1048576

class LoROM : public Mapper<LoROM> {
	friend class Mapper<LoROM>;
public:
	void to_string() {
		std::cout << "LoROM\n";
		log_info();
	}

protected:
	std::optional<Address> rom_idx(SNESAddress address) const {
		if (address.bank >= 0x40 && rom.size() <= 2 * MEGABYTE && has_superfx) {
	        return ((address.bank - 0x40) << 16) | address.offset;
	    }
		if (address.offset < 0x8000) {
			return std::nullopt;
		}
		return ((address.bank & 0x7F) << 15) | (address.offset & 0x7FFF);
	}

	std::optional<Address> sram_idx(SNESAddress address) const {
		bool sram_bank =
		    (address.bank >= 0x70 && address.bank <= 0x7D) ||
		    (address.bank >= 0xF0 && address.bank <= 0xFF);

		if (sram_bank) {
		    return ((address.bank & 0x0F) << 15) | address.offset;
		}

		bool sram_mirror_bank = (address.bank <= 0x3F) || (address.bank >= 0x80 && address.bank <= 0xBF);

		if (sram_mirror_bank && address.offset >= 0x6000 && address.offset < 0x8000) {
		    return ((address.bank & 0x0F) << 15) | (address.offset - 0x6000);
		}

		return std::nullopt;
	}
};