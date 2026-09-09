#include "mapper.hpp"

class ExHiROM : public Mapper<ExHiROM> {
	friend class Mapper<ExHiROM>;
public:
	void to_string() {
		std::cout << "ExHiROM\n";
		log_info();
	}
protected:
	std::optional<u32> rom_idx(SNESAddress address) const {
		if (address.bank >= 0xC0) {
			return ((address.bank & 0x3F) << 16) | address.offset;
		}

		if (address.bank >= 0x80 && address.offset >= 0x8000) {
			return ((address.bank & 0x3F) << 16) | address.offset;
		}

		if (address.bank >= 0x40 && address.bank <= 0x7D) {
			return 0x400000 + ((address.bank & 0x3F) << 16) + address.offset;
		}

		if (address.bank <= 0x3F && address.offset >= 0x8000) {
			return 0x400000 + ((address.bank & 0x3F) << 16) + address.offset;
		}

		return std::nullopt;
	}

	std::optional<u32> sram_idx(SNESAddress address) const {
		if (!( (address.bank >= 0x20 && address.bank <= 0x3F) || (address.bank >= 0xA0 && address.bank <= 0xBF) )) {
			return std::nullopt;
		}

		if (address.offset < 0x6000 || address.offset > 0x7FFF) {
			return std::nullopt;
		}

		return ((address.bank & 0x1F) << 13) | (address.offset & 0x1FFF);
	}
};