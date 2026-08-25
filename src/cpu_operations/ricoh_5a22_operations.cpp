#include "ricoh_5a22_operations.hpp"
#include "ricoh_5a22_addressing_modes.hpp"
#include "ricoh_5a22.hpp"

static Address get_pcpb(Word pc, Byte pb) {
	return (pb << 16) | pc;
}

namespace ReadTo {
	struct OpCode  {};
	struct Pointer {};
	struct Address {};
	struct Operand {};
	struct Bank    {};

	struct PC {};
	struct PB {};
	struct Discard {};

	struct OpCodeHigh  {};
	struct PointerHigh {};
	struct AddressHigh {};
	struct OperandHigh {};
	struct BankHigh    {};

	struct OperandLow  {};
	struct AddressLow  {};

	struct PCPB      {};
	struct PointerDB {};
}

namespace ReadFrom {
	struct OpCode    {};
	struct Pointer   {};
	struct Address   {};
	struct AddressDL {};
	struct AddressPlusOneWrap16 {};
	struct Operand   {};
	struct PC {};

	struct Vector {};
	struct VectorPlusOne {};

	struct RegisterA {};

	struct AddressPlusOne   {};
	struct AddressPlusOneDL {};

	struct AddressPlusTwo   {};
	struct AddressPlusTwoDL {};

	struct PointerPlusOne {};
	
	struct XBank     {};
	struct PointerBank    {};
	struct PointerPlusOneBankCarry {};
	struct PointerPlusOneBankNoCarry {};
	struct PointerDB    {};
	struct PointerPlusOneDBCarry {};
	struct AddressDB               {};
	struct AddressPlusOneDBCarry   {};
	struct AddressBank             {};
	struct AddressPlusOneBankCarry {};
	struct PCPB      {};

	struct Stack0 {};
	struct Stack0Emulation {};
	struct Stack1 {};
	struct Stack1Emulation {};
	struct Stack2 {};
	struct Stack2Emulation {};
	struct Stack3 {};
	struct Stack3Emulation {};
}

namespace CopyMode {
	struct All  {};
	struct High {};
	struct Low  {};
}

namespace Branching {
	struct None          {};
	struct FlagNative    {};
	struct FlagEmulation {};
}

namespace SetMode {
	// Native
	struct None {};
	struct DX   {};
	struct APS  {};
	struct AOD  {};
	struct AOXD {};
	struct AOYD {};
	struct DBOL {};
	struct OOPC {};
	struct OX   {};
	struct OY   {};
	struct OZ   {};

	// Emulation
	struct DXEmulation   {};
	struct APSEmulation  {};

	struct AOXDEmulation {};
	struct AOYDEmulation {};
}

namespace Mode {
	constexpr bool PlusOne = true;
	constexpr bool PCIncrement = true;
	constexpr bool IfSkipped = true;
	constexpr bool IsImmediate = true;

	struct Native    {};
	struct Emulation {};

	struct Set   {};
	struct Reset {};

	struct RegisterA {};
	struct RegisterX {};
	struct RegisterY {};
	struct Operand   {};

	struct Increase {};
	struct Decrease {};

	struct MFlag {};
	struct XFlag {};
}

namespace BranchMode {
	struct None    {};
	struct N_Zero  {};
	struct N_One   {};
	struct V_Zero  {};
	struct V_One   {};
	struct Always  {};
	struct C_Zero  {};
	struct C_One   {};
	struct Z_Zero  {};
	struct Z_One   {};
}

namespace WriteValue {
	struct OperandLow  {};
	struct OperandHigh {};
	struct RegisterALow {};
	struct RegisterAHigh {};
	struct None        {};
	struct ALow {};
	struct AHigh {};
	struct XLow {};
	struct XHigh {};
	struct YLow {};
	struct YHigh {};
	struct DLow {};
	struct DHigh {};
	struct PCLow {};
	struct PCHigh {};

	struct Zero {};

	struct DB {};
	struct PB {};

	struct P {};	
}

namespace WriteTo {
	struct PointerDB        {};
	struct PointerPlusOneDB {};
	struct PointerPlusOneDBCarry {};
	struct YDB  {};

	struct Address {};
	struct AddressPlusOne {};

	struct PointerBank {};
	struct PointerPlusOneBankCarry {};

	struct AddressBank {};
	struct AddressPlusOneBankCarry {};

	struct AddressDB {};
	struct AddressPlusOneDBCarry {};
	struct None {};

	struct Stack0 {};
	struct Stack0Emulation {};
	struct StackMinus1 {};
	struct StackMinus1Emulation {};
	struct StackMinus2 {};
	struct StackMinus2Emulation {};	
	struct StackMinus3 {};
	struct StackMinus3Emulation {};	
}

// Operations which will require additional features when added
namespace Ricoh5A22SpecialFunctions {
	static void STOP(Ricoh5A22& cpu, bool skipped) {
		// STOP BEHAVIOUR TO OCCUR HERE
		// Stops processor until hardware reset
		return;
	}

	static void WAIT(Ricoh5A22& cpu, bool skipped) {
		cpu.waiting = true;
		cpu.poll_interrupts();
		return;
	}
}

namespace Ricoh5A22Functions {
	static void NOP(Ricoh5A22& cpu, bool skipped) {
		return;
	}

	static void DecrementPC(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.PC -= 1;
	}

	static void WaitPC(Ricoh5A22& cpu, bool skipped) {
		if (!cpu.waiting) {
			cpu.regs.PC++;
			cpu.waiting = true;
		}
	}

	template <typename Set = SetMode::None, bool IfSkipped = false, typename Branch = BranchMode::None, bool NoIncrement = false>
	static void IncrementPC(Ricoh5A22& cpu, bool skipped) {
		
		if constexpr (IfSkipped) {
			if (skipped && !NoIncrement) { cpu.regs.PC++; }
		} else {
			if constexpr (!NoIncrement) {
				cpu.regs.PC++;
			}
		}

		if constexpr (std::is_same_v<Branch, BranchMode::N_One>) {
			cpu.Branching = (cpu.get_flag_N());
		}
		if constexpr (std::is_same_v<Branch, BranchMode::N_Zero>) {
			cpu.Branching = !(cpu.get_flag_N());
		}
		if constexpr (std::is_same_v<Branch, BranchMode::V_One>) {
			cpu.Branching = (cpu.get_flag_V());
		}
		if constexpr (std::is_same_v<Branch, BranchMode::V_Zero>) {
			cpu.Branching = !(cpu.get_flag_V());
		}
		if constexpr (std::is_same_v<Branch, BranchMode::C_One>) {
			cpu.Branching = (cpu.get_flag_C());
		}
		if constexpr (std::is_same_v<Branch, BranchMode::C_Zero>) {
			cpu.Branching = !(cpu.get_flag_C());
		}
		if constexpr (std::is_same_v<Branch, BranchMode::Z_One>) {
			cpu.Branching = (cpu.get_flag_Z());
		}
		if constexpr (std::is_same_v<Branch, BranchMode::Z_Zero>) {
			cpu.Branching = !(cpu.get_flag_Z());
		}
		if constexpr (std::is_same_v<Branch, BranchMode::Always>) {
			cpu.Branching = true;
		}

		if constexpr (std::is_same_v<Set, SetMode::DX>) {
			cpu.BufferPointer = cpu.BufferPointer + cpu.regs.X + cpu.regs.D;
			cpu.BufferAddress = cpu.BufferPointer;
		}
		if constexpr (std::is_same_v<Set, SetMode::DXEmulation>) {
			cpu.BufferPointer = cpu.BufferPointer + cpu.regs.X + cpu.regs.D;
			cpu.BufferAddress = (get_hi(cpu.regs.D) << 8) | get_lo(cpu.BufferPointer);
		}
		if constexpr (std::is_same_v<Set, SetMode::APS>) {
			cpu.BufferAddress = cpu.BufferPointer + cpu.regs.S;
		}
		if constexpr (std::is_same_v<Set, SetMode::APSEmulation>) {
			cpu.BufferAddress = cpu.BufferPointer + (cpu.regs.S | 0x0100);
		}
		if constexpr (std::is_same_v<Set, SetMode::AOD>) {
			cpu.BufferAddress = cpu.BufferOperand + cpu.regs.D;
		}
		if constexpr (std::is_same_v<Set, SetMode::AOXD>) {
			cpu.BufferAddress = cpu.BufferOperand + cpu.regs.X + cpu.regs.D;
		}
		if constexpr (std::is_same_v<Set, SetMode::AOXDEmulation>) {
			if (get_lo(cpu.regs.D) == 0) {
				cpu.BufferAddress = (cpu.regs.D & 0xFF00) | (uint8_t)(uint8_t(cpu.BufferOperand) + uint8_t(cpu.regs.X) + uint8_t(cpu.regs.D));
			} else {
				cpu.BufferAddress = cpu.BufferOperand + get_lo(cpu.regs.X) + cpu.regs.D;
			}
		}
		if constexpr (std::is_same_v<Set, SetMode::AOYD>) {
			cpu.BufferAddress = cpu.BufferOperand + cpu.regs.Y + cpu.regs.D;
		}
		if constexpr (std::is_same_v<Set, SetMode::AOYDEmulation>) {
			if (get_lo(cpu.regs.D) == 0) {
				cpu.BufferAddress = (cpu.regs.D & 0xFF00) | (uint8_t)(uint8_t(cpu.BufferOperand) + uint8_t(cpu.regs.Y) + uint8_t(cpu.regs.D));
			} else {
				cpu.BufferAddress = cpu.BufferOperand + get_lo(cpu.regs.Y) + cpu.regs.D;
			}
		}
		if constexpr (std::is_same_v<Set, SetMode::DBOL>) {
			cpu.regs.DB = get_lo(cpu.BufferOperand);
		}
		if constexpr (std::is_same_v<Set, SetMode::OOPC>) {
			cpu.BufferOperand = cpu.BufferOperand + cpu.regs.PC;
		}
		if constexpr (std::is_same_v<Set, SetMode::OX>) {
			cpu.BufferOperand = cpu.regs.X;
		}
		if constexpr (std::is_same_v<Set, SetMode::OY>) {
			cpu.BufferOperand = cpu.regs.Y;
		}
		if constexpr (std::is_same_v<Set, SetMode::OZ>) {
			cpu.BufferOperand = 0;
		}

		
	}

	template <typename From, typename To, bool PlusOne = false, typename BranchingRoutine = Branching::None>
	static void Read(Ricoh5A22& cpu, bool skipped) {
		
		Word register_offset = 0;
		Byte register_bank = 0;

		if constexpr (std::is_same_v<From, ReadFrom::PCPB>) {
			register_offset = cpu.regs.PC;
			register_bank = cpu.regs.PB;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Address>) {
			register_offset = cpu.BufferAddress;
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Pointer>) {
			register_offset = cpu.BufferPointer;
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::PointerPlusOne>) {
			register_offset = cpu.BufferPointer + 1;
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Vector>) {
			register_offset = cpu.Vector;
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::VectorPlusOne>) {
			register_offset = cpu.Vector + 1;
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::AddressDL>) {
			if (get_lo(cpu.regs.D) != 0) {
				register_offset = cpu.BufferAddress;
			} else {
				register_offset = (cpu.BufferAddress & 0xFF00) | (uint8_t)(get_lo(cpu.BufferAddress));
			}
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::AddressPlusOneDL>) {
			if (get_lo(cpu.regs.D) != 0) {
				register_offset = cpu.BufferAddress + 1;
			} else {
				register_offset = (cpu.BufferAddress & 0xFF00) | (uint8_t)(get_lo(cpu.BufferAddress + 1));
			}
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::AddressPlusOneWrap16>) {
			register_offset = (cpu.BufferAddress & 0xFF00) |
                      ((cpu.BufferAddress + 1) & 0x00FF);
    		register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::AddressPlusTwoDL>) {
			if (get_lo(cpu.regs.D) != 0) {
				register_offset = cpu.BufferAddress + 2;
			} else {
				register_offset = (cpu.BufferAddress & 0xFF00) | (uint8_t)(get_lo(cpu.BufferAddress + 2));
			}
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::AddressPlusTwo>) {
			register_offset = cpu.BufferAddress + 2;
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::PointerDB>) {
			register_offset = cpu.BufferPointer;
			register_bank = cpu.regs.DB;
		}
		if constexpr (std::is_same_v<From, ReadFrom::PointerBank>) {
			register_offset = cpu.BufferPointer;
			register_bank = cpu.BufferBank;
		}
		if constexpr (std::is_same_v<From, ReadFrom::XBank>) {
			register_offset = cpu.regs.X;
			register_bank = cpu.BufferBank;
		}
		if constexpr (std::is_same_v<From, ReadFrom::PointerPlusOneBankCarry>) {
			register_offset = cpu.BufferPointer + 1;
			register_bank = cpu.BufferBank;

			if (cpu.BufferPointer == 0xFFFF) {
				register_bank++;
			}
		}
		if constexpr (std::is_same_v<From, ReadFrom::PointerPlusOneBankNoCarry>) {
			register_offset = cpu.BufferPointer + 1;
			register_bank = cpu.BufferBank;
		}
		if constexpr (std::is_same_v<From, ReadFrom::PointerDB>) {
			register_offset = cpu.BufferPointer;
			register_bank = cpu.regs.DB;
		}
		if constexpr (std::is_same_v<From, ReadFrom::PointerPlusOneDBCarry>) {
			register_offset = cpu.BufferPointer + 1;
			register_bank = cpu.regs.DB;

			if (cpu.BufferPointer == 0xFFFF) {
				register_bank++;
			}
		}
		if constexpr (std::is_same_v<From, ReadFrom::AddressDB>) {
			register_offset = cpu.BufferAddress;
			register_bank = cpu.regs.DB;
		}
		if constexpr (std::is_same_v<From, ReadFrom::AddressPlusOneDBCarry>) {
			register_offset = cpu.BufferAddress + 1;
			register_bank = cpu.regs.DB;

			if (cpu.BufferAddress == 0xFFFF) {
				register_bank++;
			}
		}
		if constexpr (std::is_same_v<From, ReadFrom::AddressBank>) {
			register_offset = cpu.BufferAddress;
			register_bank = cpu.BufferBank;
		}
		if constexpr (std::is_same_v<From, ReadFrom::AddressPlusOneBankCarry>) {
			register_offset = cpu.BufferAddress + 1;
			register_bank = cpu.BufferBank;

			if (cpu.BufferAddress == 0xFFFF) {
				register_bank++;
			}
		}
		if constexpr (std::is_same_v<From, ReadFrom::Stack0>) {
			register_offset = cpu.regs.S;
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Stack1>) {
			register_offset = cpu.regs.S + 1;
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Stack1Emulation>) {
			register_offset = 0x0100 | ((get_lo(cpu.regs.S) + 1) & 0xFF);
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Stack2Emulation>) {
			register_offset = 0x0100 | ((get_lo(cpu.regs.S) + 2) & 0xFF);
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Stack3Emulation>) {
			register_offset = 0x0100 | ((get_lo(cpu.regs.S) + 3) & 0xFF);
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Stack2>) {
			register_offset = cpu.regs.S + 2;
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Stack3>) {
			register_offset = cpu.regs.S + 3;
			register_bank = 0;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Stack0Emulation>) {
			register_offset = 0x0100 | (uint8_t)(get_lo(cpu.regs.S));
			register_bank = 0;
		}


		Word* read_to = nullptr;

		if constexpr (std::is_same_v<To, ReadTo::OpCode> || std::is_same_v<To, ReadTo::OpCodeHigh>) {
			read_to = &cpu.BufferOpCode;
		}
		if constexpr (std::is_same_v<To, ReadTo::Pointer> || std::is_same_v<To, ReadTo::PointerHigh>) {
			read_to = &cpu.BufferPointer;
		}
		if constexpr (std::is_same_v<To, ReadTo::Address> || std::is_same_v<To, ReadTo::AddressHigh> || std::is_same_v<To, ReadTo::AddressLow>) {
			read_to = &cpu.BufferAddress;
		}
		if constexpr (std::is_same_v<To, ReadTo::Operand> || std::is_same_v<To, ReadTo::OperandHigh> || std::is_same_v<To, ReadTo::OperandLow>) {
			read_to = &cpu.BufferOperand;
		}
		if constexpr (std::is_same_v<To, ReadTo::Bank> || std::is_same_v<To, ReadTo::BankHigh>) {
			read_to = &cpu.BufferBank;
		}
		if constexpr (std::is_same_v<To, ReadTo::Discard>) {
			read_to = &cpu.DiscardBuffer;
		}

		if constexpr (PlusOne) {
			register_offset = register_offset + 1;
		}


		Byte value_read = cpu.read(get_pcpb(register_offset, register_bank));
		Word to_read = value_read;
		if constexpr (PlusOne) {
			to_read = (value_read << 8) | get_lo(*read_to);
		} else {
			to_read = value_read;
		}

		if constexpr (std::is_same_v<To, ReadTo::PB>) {
			cpu.regs.PB = to_read;
		} else if constexpr(std::is_same_v<To, ReadTo::OpCodeHigh> || std::is_same_v<To, ReadTo::PointerHigh> || std::is_same_v<To, ReadTo::AddressHigh> || std::is_same_v<To, ReadTo::OperandHigh>) {
			*read_to = (to_read << 8) | get_lo(*read_to);
		} else if constexpr(std::is_same_v<To, ReadTo::OperandLow> || std::is_same_v<To, ReadTo::AddressLow>) {
			*read_to = (get_hi(*read_to) << 8) | (uint8_t)(to_read);
		} else {
			*read_to = to_read;
		}

		if constexpr(std::is_same_v<To, ReadTo::Bank>) {
			cpu.BufferMVDest = to_read;
		}

		if constexpr (std::is_same_v<BranchingRoutine, Branching::FlagEmulation>) {
			/*cpu.BufferAddress = (int16_t)((int8_t)(get_lo(cpu.BufferOperand))) + cpu.regs.PC + 1;
			cpu.BoundaryCrossed = (get_hi(cpu.BufferAddress) != get_hi(cpu.regs.PC + 1));
			if (!cpu.Branching || !cpu.BoundaryCrossed) {
				cpu.poll_interrupts();
			}*/
			int16_t offset = static_cast<int8_t>(cpu.BufferOperand & 0xFF);

		    cpu.BufferAddress =
		        static_cast<uint16_t>(cpu.regs.PC + 1 + offset);

		    cpu.BoundaryCrossed =
		        ((cpu.regs.PC + 1) & 0xFF00) !=
		        (cpu.BufferAddress & 0xFF00);

		    if (!cpu.Branching || !cpu.BoundaryCrossed) {
			    if (cpu.interrupt_pending()) {
			        cpu.regs.PC += 1;
			    }
			    cpu.poll_interrupts();
			}
		}
		if constexpr (std::is_same_v<BranchingRoutine, Branching::FlagNative>) {
		    int16_t offset = static_cast<int8_t>(cpu.BufferOperand & 0xFF);
		    cpu.BufferAddress = static_cast<uint16_t>(cpu.regs.PC + 1 + offset);

		    if (!cpu.Branching) {
		        if (cpu.interrupt_pending()) {
		            cpu.regs.PC += 1;      // only advance to the true instruction boundary
		        }                          // when we're actually about to latch it
		        cpu.poll_interrupts();
		    }
		}

		
	}

	static void Next(Ricoh5A22& cpu, bool skipped) {
	    Word register_offset = cpu.regs.PC;
	    Byte register_bank = cpu.regs.PB;

	    Byte value_read = cpu.read(get_pcpb(register_offset, register_bank));

	    cpu.BufferOpCode = value_read;

	    cpu.log_ricoh();
	}

	template <typename From, typename To, typename Mode = CopyMode::All, bool PCIncrement = false>
	static void Copy(Ricoh5A22& cpu, bool skipped) {
		
		Word* to = nullptr;
		Word* from = nullptr;

		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}

		if constexpr (std::is_same_v<From, ReadFrom::OpCode>) {
			from = &cpu.BufferOpCode;
		}
		if constexpr (std::is_same_v<From, ReadFrom::RegisterA>) {
			from = &cpu.regs.A;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Pointer>) {
			from = &cpu.BufferPointer;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Address>) {
			from = &cpu.BufferAddress;
		}
		if constexpr (std::is_same_v<From, ReadFrom::Operand>) {
			from = &cpu.BufferOperand;
		}
		if constexpr (std::is_same_v<From, ReadFrom::PC>) {
			from = &cpu.regs.PC;
		}

		if constexpr (std::is_same_v<To, ReadTo::OpCode>) {
			to = &cpu.BufferOpCode;
		}
		if constexpr (std::is_same_v<To, ReadTo::Pointer>) {
			to = &cpu.BufferPointer;
		}
		if constexpr (std::is_same_v<To, ReadTo::Address>) {
			to = &cpu.BufferAddress;
		}
		if constexpr (std::is_same_v<To, ReadTo::Operand>) {
			to = &cpu.BufferOperand;
		}
		if constexpr (std::is_same_v<To, ReadTo::PC>) {
			to = &cpu.regs.PC;
		}

		if constexpr (std::is_same_v<Mode, CopyMode::All>) {
			*to = *from;
		}
		if constexpr (std::is_same_v<Mode, CopyMode::High>) {
			*to = get_lo(*to);
			*to = (get_hi(*from) << 8) | *to;
		}
		if constexpr (std::is_same_v<Mode, CopyMode::Low>) {
			*to = get_hi(*to);
			*to = ( (*to) << 8) | get_lo(*from);
		}
		
	}


	template <typename CPUMode, bool PCIncrement = false>
	static void LDA(Ricoh5A22& cpu, bool skipped) {
		
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				cpu.regs.A = (get_hi(cpu.regs.A) << 8) | get_lo(cpu.BufferOperand);
				cpu.set_flag_N(get_lo(cpu.regs.A) & 0x80);
				cpu.set_flag_Z(get_lo(cpu.regs.A));
			} else {
				cpu.regs.A = cpu.BufferOperand;
				cpu.set_flag_N(get_hi(cpu.regs.A) & 0x80);
				cpu.set_flag_Z(cpu.regs.A);
			}
		} else {
			cpu.regs.A = (get_hi(cpu.regs.A) << 8) | get_lo(cpu.BufferOperand);
			cpu.set_flag_N(get_lo(cpu.regs.A) & 0x80);
			cpu.set_flag_Z(get_lo(cpu.regs.A));
		}
		
	}

	template <typename CPUMode, bool PCIncrement = false>
	static void LDX(Ricoh5A22& cpu, bool skipped) {
		
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_X()) {
				cpu.regs.X = (get_hi(cpu.regs.X) << 8) | get_lo(cpu.BufferOperand);
				cpu.set_flag_N(get_lo(cpu.regs.X) & 0x80);
				cpu.set_flag_Z(get_lo(cpu.regs.X));
			} else {
				cpu.regs.X = cpu.BufferOperand;
				cpu.set_flag_N(get_hi(cpu.regs.X) & 0x80);
				cpu.set_flag_Z(cpu.regs.X);
			}
		} else {
			cpu.regs.X = (get_hi(cpu.regs.X) << 8) | get_lo(cpu.BufferOperand);
			cpu.set_flag_N(get_lo(cpu.regs.X) & 0x80);
			cpu.set_flag_Z(get_lo(cpu.regs.X));
		}
		
	}

	static void BRL(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.PC += cpu.BufferOperand;
	}

	static void SEC(Ricoh5A22& cpu, bool skipped) {
		cpu.set_flag_C();
	}

	static void SEI(Ricoh5A22& cpu, bool skipped) {
		cpu.set_flag_I();
	}

	static void SED(Ricoh5A22& cpu, bool skipped) {
		cpu.set_flag_D();
	}

	static void CLC(Ricoh5A22& cpu, bool skipped) {
		cpu.clear_flag_C();
	}

	static void CLD(Ricoh5A22& cpu, bool skipped) {
		cpu.clear_flag_D();
	}

	static void CLI(Ricoh5A22& cpu, bool skipped) {
		cpu.clear_flag_I();
	}

	static void CLV(Ricoh5A22& cpu, bool skipped) {
		cpu.clear_flag_V();
	}

	static void XCE(Ricoh5A22& cpu, bool skipped) {
		bool carry = cpu.get_flag_C();
		bool emulation = cpu.regs.emulation_mode;

		if (emulation) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}

		cpu.regs.emulation_mode = carry;

		if (cpu.regs.emulation_mode) {
			cpu.set_flag_M();
			cpu.set_flag_X();
			cpu.regs.S = cpu.regs.S | 0xFF00;
			cpu.regs.X = cpu.regs.X & 0x00FF;
			cpu.regs.Y = cpu.regs.Y & 0x00FF;
		}
	}

	template <typename CPUMode, bool PCIncrement = false>
	static void LDY(Ricoh5A22& cpu, bool skipped) {
		
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_X()) {
				cpu.regs.Y = (get_hi(cpu.regs.Y) << 8) | get_lo(cpu.BufferOperand);
				cpu.set_flag_N(get_lo(cpu.regs.Y) & 0x80);
				cpu.set_flag_Z(get_lo(cpu.regs.Y));
			} else {
				cpu.regs.Y = cpu.BufferOperand;
				cpu.set_flag_N(get_hi(cpu.regs.Y) & 0x80);
				cpu.set_flag_Z(cpu.regs.Y);
			}
		} else {
			cpu.regs.Y = (get_hi(cpu.regs.Y) << 8) | get_lo(cpu.BufferOperand);
			cpu.set_flag_N(get_lo(cpu.regs.Y) & 0x80);
			cpu.set_flag_Z(get_lo(cpu.regs.Y));
		}
		
	}

	template <typename CPUMode, bool PCIncrement = false>
	static void ORA(Ricoh5A22& cpu, bool skipped) {
		
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.A = cpu.regs.A | cpu.BufferOperand;
			if (cpu.get_flag_M()) {
				cpu.set_flag_N(get_lo(cpu.regs.A) & 0x80);
				cpu.set_flag_Z(get_lo(cpu.regs.A));
			} else {
				cpu.set_flag_N(get_hi(cpu.regs.A) & 0x80);
				cpu.set_flag_Z(cpu.regs.A);
			}
		} else {
			cpu.regs.A = cpu.regs.A | cpu.BufferOperand;
			cpu.set_flag_N(get_lo(cpu.regs.A) & 0x80);
			cpu.set_flag_Z(get_lo(cpu.regs.A));
		}
		
	}

	template <typename CPUMode, bool PCIncrement = false>
	static void CMP(Ricoh5A22& cpu, bool skipped) {
		
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				cpu.clear_flag_C();
				cpu.clear_flag_Z();
				cpu.clear_flag_N();
				if (get_lo(cpu.regs.A) >= get_lo(cpu.BufferOperand)) {
					cpu.set_flag_C();
				}
				if (get_lo(cpu.regs.A) == get_lo(cpu.BufferOperand)) {
					cpu.set_flag_Z();
				}
				if (((get_lo(cpu.regs.A) - get_lo(cpu.BufferOperand)) & 0x80) != 0) {
					cpu.set_flag_N();
				}
			} else {
				cpu.clear_flag_C();
				cpu.clear_flag_Z();
				cpu.clear_flag_N();
				if (cpu.regs.A >= cpu.BufferOperand) {
					cpu.set_flag_C();
				}
				if (cpu.regs.A == cpu.BufferOperand) {
					cpu.set_flag_Z();
				}
				if (((cpu.regs.A - cpu.BufferOperand) & 0x8000) != 0) {
					cpu.set_flag_N();
				}
			}
		} else {
			cpu.clear_flag_C();
			cpu.clear_flag_Z();
			cpu.clear_flag_N();
			if (get_lo(cpu.regs.A) >= get_lo(cpu.BufferOperand)) {
				cpu.set_flag_C();
			}
			if (get_lo(cpu.regs.A) == get_lo(cpu.BufferOperand)) {
				cpu.set_flag_Z();
			}
			if (((get_lo(cpu.regs.A) - get_lo(cpu.BufferOperand)) & 0x80) != 0) {
				cpu.set_flag_N();
			}	
		}
		
	}


	template <typename CPUMode, bool PCIncrement = false>
	static void AND(Ricoh5A22& cpu, bool skipped) {
		
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				cpu.regs.A = (get_hi(cpu.regs.A) << 8) | (get_lo(cpu.regs.A) & get_lo(cpu.BufferOperand));
				cpu.set_flag_N(get_lo(cpu.regs.A) & 0x80);
				cpu.set_flag_Z(get_lo(cpu.regs.A));
			} else {
				cpu.regs.A = cpu.regs.A & cpu.BufferOperand;
				cpu.set_flag_N(get_hi(cpu.regs.A) & 0x80);
				cpu.set_flag_Z(cpu.regs.A);
			}
		} else {
			cpu.regs.A = (get_hi(cpu.regs.A) << 8) | (get_lo(cpu.regs.A) & get_lo(cpu.BufferOperand));
			cpu.set_flag_N(get_lo(cpu.regs.A) & 0x80);
			cpu.set_flag_Z(get_lo(cpu.regs.A));
		}
		
	}

	template <typename CPUMode, bool PCIncrement = false>
	static void EOR(Ricoh5A22& cpu, bool skipped) {
		
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.A = cpu.regs.A ^ cpu.BufferOperand;
			if (cpu.get_flag_M()) {
				cpu.set_flag_N(get_lo(cpu.regs.A) & 0x80);
				cpu.set_flag_Z(get_lo(cpu.regs.A));
			} else {
				cpu.set_flag_N(get_hi(cpu.regs.A) & 0x80);
				cpu.set_flag_Z(cpu.regs.A);
			}
		} else {
			cpu.regs.A = cpu.regs.A ^ cpu.BufferOperand;
			cpu.set_flag_N(get_lo(cpu.regs.A) & 0x80);
			cpu.set_flag_Z(get_lo(cpu.regs.A));
		}

		
	}

	static void adc_m_flag(Ricoh5A22& cpu) {
		Byte value = cpu.BufferOperand & 0xFF;
		if (!cpu.get_flag_D()) {
			uint16_t result = get_lo(cpu.regs.A) + value + cpu.get_flag_C();
			if ((~(get_lo(cpu.regs.A) ^ value) & (get_lo(cpu.regs.A) ^ get_lo(result)) & 0x80) != 0) {
				cpu.set_flag_V();
			} else {
				cpu.clear_flag_V();
			}
			if (result > 0xFF) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}
			result = get_lo(result);
			cpu.regs.A = (get_hi(cpu.regs.A) << 8) | result;
			cpu.set_flag_Z(result);
			cpu.set_flag_N(result & 0x80);
		} else {
			uint16_t lo = (get_lo(cpu.regs.A) & 0x0F) + (value & 0x0F) + cpu.get_flag_C();
			if (lo > 9) {
				lo += 6;
			}
			uint16_t carry_to_hi = (lo > 0x0F) ? 1 : 0;
			uint16_t hi_sum = (get_lo(cpu.regs.A) >> 4) + (value >> 4) + carry_to_hi;
			if ((~((get_lo(cpu.regs.A) >> 4) ^ (value >> 4)) & ((get_lo(cpu.regs.A) >> 4) ^ hi_sum) & 0x08) != 0) {
				cpu.set_flag_V();
			} else {
				cpu.clear_flag_V();
			}
			if (hi_sum > 9) {
				hi_sum += 6;
			}
			if (hi_sum > 0x0f) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}
			uint16_t result = ((hi_sum & 0x0f) << 4) | (lo & 0x0f);
			cpu.regs.A = (get_hi(cpu.regs.A) << 8) | result;
			cpu.set_flag_Z(result);
			cpu.set_flag_N(result & 0x80);
		}
	}

	static void adc_no_m_flag(Ricoh5A22& cpu) {
		if (!cpu.get_flag_D()) {
			uint32_t result = cpu.regs.A + cpu.BufferOperand + cpu.get_flag_C();
			if ((~(cpu.regs.A ^ cpu.BufferOperand) & (cpu.regs.A ^ (uint16_t)(result)) & 0x8000) != 0) {
				cpu.set_flag_V();
			} else {
				cpu.clear_flag_V();
			}
			if (result > 0xFFFF) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}
			result = (uint16_t)(result);
			cpu.regs.A = (uint16_t)result;
			cpu.set_flag_Z(result);
			if ((result & 0x8000) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
		} else {
			uint16_t result = 0;
			uint16_t carry = cpu.get_flag_C();
			for (int i = 0; i < 16; i += 4) {
				uint16_t digit_sum = (uint16_t)( (cpu.regs.A >> i) & 0x0F) + (uint16_t)((cpu.BufferOperand >> i) & 0x0F) + carry;
				if (i == 12) {
					if ((~((cpu.regs.A >> 12) ^ (cpu.BufferOperand >> 12)) & ((cpu.regs.A >> 12) ^ digit_sum) & 0x08) != 0) {
						cpu.set_flag_V();
					} else {
						cpu.clear_flag_V();
					}
				}
				if (digit_sum > 9) {
					digit_sum += 6;
				}
				carry = (digit_sum > 0x0F) ? 1 : 0;
				result = result | (uint16_t)((digit_sum & 0x0F) << i);
			}
			if (carry != 0) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}
			cpu.regs.A = result;
			cpu.set_flag_Z(result);
			if ((result & 0x8000) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
		}
	}

	template <typename CPUMode, bool PCIncrement = false>
	static void ADC(Ricoh5A22& cpu, bool skipped) {
		
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				Ricoh5A22Functions::adc_m_flag(cpu);
			} else {
				Ricoh5A22Functions::adc_no_m_flag(cpu);
			}
		} else {
			Ricoh5A22Functions::adc_m_flag(cpu);
		}
		
	}

	static void sbc_m_flag(Ricoh5A22& cpu) {
		if (!cpu.get_flag_D()) {
			uint16_t result = get_lo(cpu.regs.A) - cpu.BufferOperand - (1 - cpu.get_flag_C());

			if (((get_lo(cpu.regs.A) ^ cpu.BufferOperand) & (get_lo(cpu.regs.A) ^ get_lo(result)) & 0x80) != 0) {
				cpu.set_flag_V();
			} else {
				cpu.clear_flag_V();
			}

			if (result <= 0xFF) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}

			result = get_lo(result);
			cpu.regs.A = (get_hi(cpu.regs.A) << 8) | result;
			cpu.set_flag_Z(result);
			cpu.set_flag_N(result & 0x80);
		} else {
			int16_t lo = (get_lo(cpu.regs.A) & 0x0F) - (cpu.BufferOperand & 0x0F) - (1 - cpu.get_flag_C());

			if (lo < 0) {
				lo -= 6;
			}

			int16_t borrow_to_hi = (lo < 0) ? 1 : 0;

			int16_t hi_sum = (get_lo(cpu.regs.A) >> 4) - (cpu.BufferOperand >> 4) - borrow_to_hi;

			if ((((get_lo(cpu.regs.A) >> 4) ^ (cpu.BufferOperand >> 4)) & ((get_lo(cpu.regs.A) >> 4) ^ hi_sum) & 0x08) != 0) {
				cpu.set_flag_V();
			} else {
				cpu.clear_flag_V();
			}

			if (hi_sum < 0) {
				hi_sum -= 6;
			}

			if (hi_sum >= 0) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}

			uint16_t result = ((hi_sum & 0x0F) << 4) | (lo & 0x0F);
			cpu.regs.A = (get_hi(cpu.regs.A) << 8) | result;
			cpu.set_flag_Z(result);
			cpu.set_flag_N(result & 0x80);
		}
	}

	static void sbc_no_m_flag(Ricoh5A22& cpu) {
		if (!cpu.get_flag_D()) {
			uint32_t result = (uint32_t)cpu.regs.A - (uint32_t)cpu.BufferOperand - (1 - cpu.get_flag_C());

			if (((cpu.regs.A ^ cpu.BufferOperand) & (cpu.regs.A ^ (uint16_t)result) & 0x8000) != 0) {
				cpu.set_flag_V();
			} else {
				cpu.clear_flag_V();
			}

			if (result <= 0xFFFF) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}

			result = (uint16_t)result;
			cpu.regs.A = (uint16_t)result;
			cpu.set_flag_Z(result);

			if ((result & 0x8000) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
		} else {
			uint16_t result = 0;
			uint16_t borrow = 1 - cpu.get_flag_C();

			for (int i = 0; i < 16; i += 4) {
				int16_t digit_sum = (int16_t)((cpu.regs.A >> i) & 0x0F) - (int16_t)((cpu.BufferOperand >> i) & 0x0F) - borrow;

				if (i == 12) {
					if ((((cpu.regs.A >> 12) ^ (cpu.BufferOperand >> 12)) & ((cpu.regs.A >> 12) ^ digit_sum) & 0x08) != 0) {
						cpu.set_flag_V();
					} else {
						cpu.clear_flag_V();
					}
				}

				if (digit_sum < 0) {
					digit_sum -= 6;
				}

				borrow = (digit_sum < 0) ? 1 : 0;

				result = result | ((uint16_t)((digit_sum & 0x0F) << i));
			}

			if (borrow == 0) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}

			cpu.regs.A = result;
			cpu.set_flag_Z(result);

			if ((result & 0x8000) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
		}
	}

	template <typename CPUMode, bool PCIncrement = false>
	static void SBC(Ricoh5A22& cpu, bool skipped) {
		
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				Ricoh5A22Functions::sbc_m_flag(cpu);
			} else {
				Ricoh5A22Functions::sbc_no_m_flag(cpu);
			}
		} else {
			Ricoh5A22Functions::sbc_m_flag(cpu);
		}
		
	}

	template<bool PCIncrement = false>
	static void DirectIndirectYIndex(Ricoh5A22& cpu, bool skipped) {
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}

		cpu.BufferOrig = cpu.BufferPointer;
		uint32_t tmp = (uint32_t)(cpu.BufferPointer + cpu.regs.Y);
		cpu.BufferPointer = (uint16_t)(tmp);
		cpu.BufferBank = cpu.regs.DB + (tmp >> 16);
	}

	template<bool PCIncrement = false>
	static void AbsoluteYIndex(Ricoh5A22& cpu, bool skipped) {
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}

		cpu.BufferOrig = cpu.BufferPointer;
		uint32_t tmp = (uint32_t)(cpu.BufferPointer + cpu.regs.Y);
		cpu.BufferPointer = (uint16_t)(tmp);
		cpu.BufferBank = cpu.regs.DB + (tmp >> 16);
	}

	template<bool PCIncrement = false>
	static void AbsoluteXIndex(Ricoh5A22& cpu, bool skipped) {
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}

		cpu.BufferOrig = cpu.BufferPointer;
		uint32_t tmp = (uint32_t)(cpu.BufferPointer + cpu.regs.X);
		cpu.BufferPointer = (uint16_t)(tmp);
		cpu.BufferBank = cpu.regs.DB + (tmp >> 16);
	}

	template<bool PCIncrement = false>
	static void AbsoluteXRMWIndex(Ricoh5A22& cpu, bool skipped) {
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}

		uint32_t tmp = (uint32_t)(cpu.BufferPointer + cpu.regs.X);
		cpu.BufferPointer = (uint16_t)(tmp);
		cpu.BufferBank = cpu.regs.DB + (tmp >> 16);
	}

	template<bool PCIncrement = false>
	static void AbsoluteLongXIndex(Ricoh5A22& cpu, bool skipped) {
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}

		uint32_t tmp = (uint32_t)(cpu.BufferPointer + cpu.regs.X);
		cpu.BufferPointer = (uint16_t)(tmp);
		cpu.BufferBank += (tmp >> 16);
	}

	template <bool PCIncrement = false>
	static void DirectIndirectIndexedLongYIndex(Ricoh5A22& cpu, bool skipped) {
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}

		uint32_t tmp = (uint32_t)(cpu.BufferPointer + cpu.regs.Y);
		cpu.BufferPointer = (uint16_t)(tmp);
		cpu.BufferBank += (tmp >> 16);
	}

	template<bool PCIncrement = false>
	static void StackRelativeIndirectIndexed(Ricoh5A22& cpu, bool skipped) {
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}

		uint32_t tmp = (uint32_t)(cpu.BufferPointer + cpu.regs.Y);
		cpu.BufferPointer = (uint16_t)tmp;
		cpu.BufferBank = cpu.regs.DB + (tmp >> 16);
	}

	static void PollInterrupts(Ricoh5A22& cpu, bool skipped) {
		cpu.poll_interrupts();
	}

	template<typename CPUMode>
	static void SetVector(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.Vector = 0xFFE4;
		} else {
			cpu.Vector = 0xFFF4;
		}
	}

	template<typename CPUMode>
	static void SetVectorBRK(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.Vector = 0xFFE6;
		} else {
			cpu.Vector = 0xFFFE;
		}
	}

	template<typename Value, typename To>
	static void Write(Ricoh5A22& cpu, bool skipped) {
		Byte value;
		if constexpr (std::is_same_v<Value, WriteValue::OperandLow>) {
			value = (uint8_t)(get_lo(cpu.BufferOperand));
		}
		if constexpr (std::is_same_v<Value, WriteValue::P>) {
			value = cpu.regs.P;
		}
		if constexpr (std::is_same_v<Value, WriteValue::OperandHigh>) {
			value = (uint8_t)(get_hi(cpu.BufferOperand));
		}
		if constexpr (std::is_same_v<Value, WriteValue::PCLow>) {
			value = (uint8_t)(get_lo(cpu.regs.PC));
		}
		if constexpr (std::is_same_v<Value, WriteValue::PCHigh>) {
			value = (uint8_t)(get_hi(cpu.regs.PC));
		}
		if constexpr (std::is_same_v<Value, WriteValue::ALow>) {
			value = (uint8_t)(get_lo(cpu.regs.A));
		}
		if constexpr (std::is_same_v<Value, WriteValue::AHigh>) {
			value = (uint8_t)(get_hi(cpu.regs.A));
		}
		if constexpr (std::is_same_v<Value, WriteValue::XLow>) {
			value = (uint8_t)(get_lo(cpu.regs.X));
		}
		if constexpr (std::is_same_v<Value, WriteValue::XHigh>) {
			value = (uint8_t)(get_hi(cpu.regs.X));
		}
		if constexpr (std::is_same_v<Value, WriteValue::YLow>) {
			value = (uint8_t)(get_lo(cpu.regs.Y));
		}
		if constexpr (std::is_same_v<Value, WriteValue::YHigh>) {
			value = (uint8_t)(get_hi(cpu.regs.Y));
		}
		if constexpr (std::is_same_v<Value, WriteValue::DLow>) {
			value = (uint8_t)(get_lo(cpu.regs.D));
		}
		if constexpr (std::is_same_v<Value, WriteValue::DHigh>) {
			value = (uint8_t)(get_hi(cpu.regs.D));
		}
		if constexpr (std::is_same_v<Value, WriteValue::RegisterALow>) {
			value = (uint8_t)(get_lo(cpu.regs.A));
		}
		if constexpr (std::is_same_v<Value, WriteValue::RegisterAHigh>) {
			value = (uint8_t)(get_hi(cpu.regs.A));
		}
		if constexpr (std::is_same_v<Value, WriteValue::DB>) {
			value = cpu.regs.DB;
		}
		if constexpr (std::is_same_v<Value, WriteValue::PB>) {
			value = cpu.regs.PB;
		}
		if constexpr (std::is_same_v<Value, WriteValue::Zero>) {
			value = 0;
		}

		Address address;
		if constexpr (std::is_same_v<To, WriteTo::YDB>) {
			address = (cpu.regs.DB << 16) | ((uint16_t)(cpu.regs.Y));
		}
		if constexpr (std::is_same_v<To, WriteTo::PointerDB>) {
			address = (cpu.regs.DB << 16) | ((uint16_t)(cpu.BufferPointer));
		}
		if constexpr (std::is_same_v<To, WriteTo::PointerPlusOneDB>) {
			address = (cpu.regs.DB << 16) | ((uint16_t)(cpu.BufferPointer + 1));
		}
		if constexpr (std::is_same_v<To, WriteTo::PointerPlusOneDBCarry>) {
			uint16_t register_offset = cpu.BufferPointer + 1;
			uint8_t register_bank = cpu.regs.DB;

			if (cpu.BufferPointer == 0xFFFF) {
				register_bank++;
			}

			address = (register_bank << 16) | register_offset;
		}
		if constexpr (std::is_same_v<To, WriteTo::Address>) {
			address = cpu.BufferAddress;
		}
		if constexpr (std::is_same_v<To, WriteTo::AddressPlusOne>) {
			uint16_t offset = (uint16_t)(cpu.BufferAddress) + 1;
			address = offset;
		}

		if constexpr (std::is_same_v<To, WriteTo::PointerBank>) {
			address = (cpu.BufferBank << 16) | (uint16_t)(cpu.BufferPointer);
		}

		if constexpr (std::is_same_v<To, WriteTo::AddressDB>) {
			address = (cpu.regs.DB << 16) | (uint16_t)(cpu.BufferAddress);
		}

		if constexpr (std::is_same_v<To, WriteTo::AddressBank>) {
			address = (cpu.BufferBank << 16) | (uint16_t)(cpu.BufferAddress);
		}

		if constexpr (std::is_same_v<To, WriteTo::PointerPlusOneBankCarry>) {
			uint16_t register_offset = cpu.BufferPointer + 1;
			uint8_t register_bank = cpu.BufferBank;

			if (cpu.BufferPointer == 0xFFFF) {
				register_bank++;
			}

			address = (register_bank << 16) | register_offset;
		}

		if constexpr (std::is_same_v<To, WriteTo::AddressPlusOneDBCarry>) {
			uint16_t register_offset = cpu.BufferAddress + 1;
			uint8_t register_bank = cpu.regs.DB;

			if (cpu.BufferAddress == 0xFFFF) {
				register_bank++;
			}

			address = (register_bank << 16) | register_offset;
		}

		if constexpr (std::is_same_v<To, WriteTo::AddressPlusOneBankCarry>) {
			uint16_t register_offset = cpu.BufferAddress + 1;
			uint8_t register_bank = cpu.BufferBank;

			if (cpu.BufferAddress == 0xFFFF) {
				register_bank++;
			}

			address = (register_bank << 16) | register_offset;
		}

		if constexpr (std::is_same_v<To, WriteTo::Stack0>) {
			address = cpu.regs.S;
		}
		if constexpr (std::is_same_v<To, WriteTo::Stack0Emulation>) {
			address = 0x0100 | get_lo(cpu.regs.S);
		}
		if constexpr (std::is_same_v<To, WriteTo::StackMinus1>) {
			address = (Word)(cpu.regs.S - 1);
		}
		if constexpr (std::is_same_v<To, WriteTo::StackMinus1Emulation>) {
			address = (0x0100 | (uint8_t)(get_lo(cpu.regs.S) - 1));
		}
		if constexpr (std::is_same_v<To, WriteTo::StackMinus2>) {
			address = (Word)(cpu.regs.S - 2);
		}
		if constexpr (std::is_same_v<To, WriteTo::StackMinus2Emulation>) {
			address = (0x0100 | (uint8_t)(get_lo(cpu.regs.S) - 2));
		}
		if constexpr (std::is_same_v<To, WriteTo::StackMinus3>) {
			address = (Word)(cpu.regs.S - 3);
		}
		if constexpr (std::is_same_v<To, WriteTo::StackMinus3Emulation>) {
			address = (0x0100 | (uint8_t)(get_lo(cpu.regs.S) - 3));
		}

		if constexpr (SST_TEST) {
			cpu.test_poke(address, value);
		} else {
			cpu.write(address, value);
		}
	}

	static void MVP(Ricoh5A22& cpu, bool skipped) {
	    cpu.regs.A -= 1;
	    
	    if (cpu.get_flag_X()) {
	        Byte low = get_lo(cpu.regs.X);
	        low -= 1;
	        cpu.regs.X = (uint8_t)low;
	        
	        low = get_lo(cpu.regs.Y);
	        low -= 1;
	        cpu.regs.Y = (uint8_t)low;
	    } else {
	        cpu.regs.X -= 1;
	        cpu.regs.Y -= 1;
	    }

	    if (cpu.regs.A != 0xFFFF) {
	        cpu.regs.PC -= 3;
	    }
	}

	static void MVN(Ricoh5A22& cpu, bool skipped) {
	    cpu.regs.A -= 1;
	    
	    if (cpu.get_flag_X()) {
	        Byte low = get_lo(cpu.regs.X);
	        low += 1;
	        cpu.regs.X = (uint8_t)low;
	        
	        low = get_lo(cpu.regs.Y);
	        low += 1;
	        cpu.regs.Y = (uint8_t)low;
	    } else {
	        cpu.regs.X += 1;
	        cpu.regs.Y += 1;
	    }

	    if (cpu.regs.A != 0xFFFF) {
	        cpu.regs.PC -= 3;
	    }
	}

	template <typename CPUMode, typename OpMode>
	static void TB(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				if ( (get_lo(cpu.regs.A) & get_lo(cpu.BufferOperand)) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				if ( (cpu.regs.A & cpu.BufferOperand) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			if ( (get_lo(cpu.regs.A) & get_lo(cpu.BufferOperand)) == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
		if constexpr (std::is_same_v<OpMode, Mode::Set>) {
			cpu.BufferOperand = cpu.BufferOperand | cpu.regs.A;
		} else {
			cpu.BufferOperand = cpu.BufferOperand & ~cpu.regs.A;
		}
	}

	template <typename CPUMode, typename SetMode = Mode::Operand>
	static void ASL(Ricoh5A22& cpu, bool skipped) {
		uint16_t* shifting = nullptr;
		if constexpr (std::is_same_v<SetMode, Mode::RegisterA>) {
			shifting = &cpu.regs.A;
		} else {
			shifting = &cpu.BufferOperand;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				if ( (get_lo(*shifting) & 0x80) != 0) {
					cpu.set_flag_C();
				} else {
					cpu.clear_flag_C();
				}
				uint8_t low = get_lo(*shifting);
				low = low << 1;
				*shifting = (get_hi(*shifting) << 8) | low;
				if ( (low & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (low == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				if ( (get_hi(*shifting) & 0x80) != 0) {
					cpu.set_flag_C();
				} else {
					cpu.clear_flag_C();
				}
				*shifting = (*shifting) << 1;
				if ( (get_hi(*shifting) & 0x80) != 0) {
					cpu.set_flag_N();
				}  else {
					cpu.clear_flag_N();
				}
				if (*shifting == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			if ( (get_lo(*shifting) & 0x80) != 0) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}
			uint8_t low = get_lo(*shifting);
			low = low << 1;
			*shifting = (get_hi(*shifting) << 8) | low;
			if ( (low & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (low == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode, typename SetMode = Mode::Operand>
	static void ROL(Ricoh5A22& cpu, bool skipped) {
		uint16_t* rotating = nullptr;
		if constexpr (std::is_same_v<SetMode, Mode::RegisterA>) {
			rotating = &cpu.regs.A;
		} else {
			rotating = &cpu.BufferOperand;
		}

		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				bool carry = cpu.get_flag_C();
				if ((get_lo(*rotating) & 0x80) != 0) {
					cpu.set_flag_C();
				} else {
					cpu.clear_flag_C();
				}
				uint8_t low = get_lo(*rotating);
				low = (low << 1);
				if (carry) {
					low = low | 0b1;
				}
				*rotating = (get_hi(*rotating) << 8) | low;
				if ((low & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (low == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				bool carry = cpu.get_flag_C();
				if ((get_hi(*rotating) & 0x80) != 0) {
					cpu.set_flag_C();
				} else {
					cpu.clear_flag_C();
				}
				*rotating = (*rotating << 1);
				if (carry) {
					*rotating = *rotating | 0b1;
				}
				if ((get_hi(*rotating) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (*rotating == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			bool carry = cpu.get_flag_C();
			if ((get_lo(*rotating) & 0x80) != 0) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}
			uint8_t low = get_lo(*rotating);
			low = (low << 1);
			if (carry) {
				low = low | 0b1;
			}
			*rotating = (get_hi(*rotating) << 8) | low;
			if ((low & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (low == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode, typename SetMode = Mode::Operand>
	static void LSR(Ricoh5A22& cpu, bool skipped) {
		uint16_t* shifting = nullptr;
		if constexpr (std::is_same_v<SetMode, Mode::RegisterA>) {
			shifting = &cpu.regs.A;
		} else {
			shifting = &cpu.BufferOperand;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				if ( (get_lo(*shifting) & 0x01) != 0) {
					cpu.set_flag_C();
				} else {
					cpu.clear_flag_C();
				}
				uint8_t low = get_lo(*shifting);
				low = low >> 1;
				*shifting = (get_hi(*shifting) << 8) | low;
				if ( (low & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (low == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				if ( (get_lo(*shifting) & 0x01) != 0) {
					cpu.set_flag_C();
				} else {
					cpu.clear_flag_C();
				}
				*shifting = (*shifting) >> 1;
				if ( (get_hi(*shifting) & 0x80) != 0) {
					cpu.set_flag_N();
				}  else {
					cpu.clear_flag_N();
				}
				if (*shifting == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			if ( (get_lo(*shifting) & 0x01) != 0) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}
			uint8_t low = get_lo(*shifting);
			low = low >> 1;
			*shifting = (get_hi(*shifting) << 8) | low;
			if ( (low & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (low == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode, typename SetMode = Mode::Operand>
	static void ROR(Ricoh5A22& cpu, bool skipped) {
		uint16_t* rotating = nullptr;
		if constexpr (std::is_same_v<SetMode, Mode::RegisterA>) {
			rotating = &cpu.regs.A;
		} else {
			rotating = &cpu.BufferOperand;
		}

		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				bool carry = cpu.get_flag_C();
				if ((get_lo(*rotating) & 0x01) != 0) {
					cpu.set_flag_C();
				} else {
					cpu.clear_flag_C();
				}
				uint8_t low = get_lo(*rotating);
				low = (low >> 1);
				if (carry) {
					low = low | 0x80;
				}
				*rotating = (get_hi(*rotating) << 8) | low;
				if ((low & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (low == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				bool carry = cpu.get_flag_C();
				if ((*rotating & 0x01) != 0) {
					cpu.set_flag_C();
				} else {
					cpu.clear_flag_C();
				}
				*rotating = (*rotating >> 1);
				if (carry) {
					*rotating = *rotating | 0x8000;
				}
				if ((get_hi(*rotating) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (*rotating == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			bool carry = cpu.get_flag_C();
			if ((get_lo(*rotating) & 0x01) != 0) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}
			uint8_t low = get_lo(*rotating);
			low = (low >> 1);
			if (carry) {
				low = low | 0x80;
			}
			*rotating = (get_hi(*rotating) << 8) | low;
			if ((low & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (low == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode>
	static void TSX(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_X()) {
				cpu.regs.X = (get_hi(cpu.regs.X) << 8) | (uint8_t)(get_lo(cpu.regs.S));
				if ((get_lo(cpu.regs.X) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (get_lo(cpu.regs.X) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				cpu.regs.X = cpu.regs.S;
				if ((get_hi(cpu.regs.X) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (cpu.regs.X == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			cpu.regs.X = (get_hi(cpu.regs.X) << 8) | (uint8_t)(get_lo(cpu.regs.S));
			if ((get_lo(cpu.regs.X) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (get_lo(cpu.regs.X) == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode>
	static void TXY(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_X()) {
				cpu.regs.Y = (get_hi(cpu.regs.Y) << 8) | (uint8_t)(get_lo(cpu.regs.X));
				if ((get_lo(cpu.regs.Y) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (get_lo(cpu.regs.Y) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				cpu.regs.Y = cpu.regs.X;
				if ((get_hi(cpu.regs.Y) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (cpu.regs.Y == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			cpu.regs.Y = (get_hi(cpu.regs.Y) << 8) | (uint8_t)(get_lo(cpu.regs.X));
			if ((get_lo(cpu.regs.Y) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (get_lo(cpu.regs.Y) == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode>
	static void TYX(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_X()) {
				cpu.regs.X = (get_hi(cpu.regs.X) << 8) | (uint8_t)(get_lo(cpu.regs.Y));
				if ((get_lo(cpu.regs.X) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (get_lo(cpu.regs.X) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				cpu.regs.X = cpu.regs.Y;
				if ((get_hi(cpu.regs.X) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (cpu.regs.X == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			cpu.regs.X = (get_hi(cpu.regs.X) << 8) | (uint8_t)(get_lo(cpu.regs.Y));
			if ((get_lo(cpu.regs.X) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (get_lo(cpu.regs.X) == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode>
	static void TAX(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_X()) {
				cpu.regs.X = (get_hi(cpu.regs.X) << 8) | (uint8_t)(get_lo(cpu.regs.A));
				if ((get_lo(cpu.regs.X) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (get_lo(cpu.regs.X) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				cpu.regs.X = cpu.regs.A;
				if ((get_hi(cpu.regs.X) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (cpu.regs.X == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			cpu.regs.X = (get_hi(cpu.regs.X) << 8) | (uint8_t)(get_lo(cpu.regs.A));
			if ((get_lo(cpu.regs.X) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (get_lo(cpu.regs.X) == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode>
	static void TAY(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_X()) {
				cpu.regs.Y = (get_hi(cpu.regs.Y) << 8) | (uint8_t)(get_lo(cpu.regs.A));
				if ((get_lo(cpu.regs.Y) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (get_lo(cpu.regs.Y) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				cpu.regs.Y = cpu.regs.A;
				if ((get_hi(cpu.regs.Y) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (cpu.regs.Y == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			cpu.regs.Y = (get_hi(cpu.regs.Y) << 8) | (uint8_t)(get_lo(cpu.regs.A));
			if ((get_lo(cpu.regs.Y) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (get_lo(cpu.regs.Y) == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode>
	static void TXA(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				cpu.regs.A = (get_hi(cpu.regs.A) << 8) | (uint8_t)(get_lo(cpu.regs.X));
				if ((get_lo(cpu.regs.A) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (get_lo(cpu.regs.A) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				cpu.regs.A = cpu.regs.X;
				if ((get_hi(cpu.regs.A) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (cpu.regs.A == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			cpu.regs.A = (get_hi(cpu.regs.A) << 8) | (uint8_t)(get_lo(cpu.regs.X));
			if ((get_lo(cpu.regs.A) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (get_lo(cpu.regs.A) == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode>
	static void TYA(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				cpu.regs.A = (get_hi(cpu.regs.A) << 8) | (uint8_t)(get_lo(cpu.regs.Y));
				if ((get_lo(cpu.regs.A) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (get_lo(cpu.regs.A) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				cpu.regs.A = cpu.regs.Y;
				if ((get_hi(cpu.regs.A) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (cpu.regs.A == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			cpu.regs.A = (get_hi(cpu.regs.A) << 8) | (uint8_t)(get_lo(cpu.regs.Y));
			if ((get_lo(cpu.regs.A) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (get_lo(cpu.regs.A) == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode>
	static void TCD(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.D = cpu.regs.A;
		if ((get_hi(cpu.regs.D) & 0x80) != 0) {
			cpu.set_flag_N();
		} else {
			cpu.clear_flag_N();
		}
		if (cpu.regs.D == 0) {
			cpu.set_flag_Z();
		} else {
			cpu.clear_flag_Z();
		}
	}

	template <typename CPUMode>
	static void TCS(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.S = cpu.regs.A;
		} else {
			cpu.regs.S = (0b1 << 8) | (uint8_t)(get_lo(cpu.regs.A));
		}
	}

	template <typename CPUMode>
	static void TDC(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.A = cpu.regs.D;
		if ((get_hi(cpu.regs.A) & 0x80) != 0) {
			cpu.set_flag_N();
		} else {
			cpu.clear_flag_N();
		}
		if (cpu.regs.A == 0) {
			cpu.set_flag_Z();
		} else {
			cpu.clear_flag_Z();
		}
	}

	template <typename CPUMode>
	static void TSC(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.A = cpu.regs.S;
			if ((get_hi(cpu.regs.A) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (cpu.regs.A == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		} else {
			cpu.regs.A = get_lo(cpu.regs.S) | 0x0100;
			if ((get_hi(cpu.regs.A) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (cpu.regs.A == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode>
	static void TXS(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.S = cpu.regs.X;
		} else {
			cpu.regs.S = (0b1 << 8) | (uint8_t)(get_lo(cpu.regs.X));
		}
	}

	static void XBA(Ricoh5A22& cpu, bool skipped) {
		uint8_t lo = get_lo(cpu.regs.A);
		uint8_t hi = get_hi(cpu.regs.A);
		cpu.regs.A = (lo << 8) | (uint8_t)(hi);
		if ((get_lo(cpu.regs.A) & 0x80) != 0) {
			cpu.set_flag_N();
		} else {
			cpu.clear_flag_N();
		}
		if (get_lo(cpu.regs.A) == 0) {
			cpu.set_flag_Z();
		} else {
			cpu.clear_flag_Z();
		}
	}

	template <typename CPUMode, bool IsImmediate = false>
	static void BIT(Ricoh5A22& cpu, bool skipped) {
		if constexpr (IsImmediate) {
			cpu.regs.PC++;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_M()) {
				if ((get_lo(cpu.regs.A) & get_lo(cpu.BufferOperand)) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
				if constexpr (!IsImmediate) {
					if ((get_lo(cpu.BufferOperand) & 0x80) != 0) {
						cpu.set_flag_N();
					} else {
						cpu.clear_flag_N();
					}
					if ((get_lo(cpu.BufferOperand) & 0x40) != 0) {
						cpu.set_flag_V();
					} else {
						cpu.clear_flag_V();
					}
				}
			} else {
				if ((cpu.regs.A & cpu.BufferOperand) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
				if constexpr (!IsImmediate) {
					if ((get_hi(cpu.BufferOperand) & 0x80) != 0) {
						cpu.set_flag_N();
					} else {
						cpu.clear_flag_N();
					}
					if ((get_hi(cpu.BufferOperand) & 0x40) != 0) {
						cpu.set_flag_V();
					} else {
						cpu.clear_flag_V();
					}
				}
			}
		} else {
			if ((get_lo(cpu.regs.A) & get_lo(cpu.BufferOperand)) == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
			if constexpr (!IsImmediate) {
				if ((get_lo(cpu.BufferOperand) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if ((get_lo(cpu.BufferOperand) & 0x40) != 0) {
					cpu.set_flag_V();
				} else {
					cpu.clear_flag_V();
				}
			}
		}
	}

	static void JMPOp(Ricoh5A22& cpu, bool skipped) {
	    cpu.BufferPointer = (cpu.BufferPointer + cpu.regs.X) & 0xFFFF;
	    cpu.BufferBank = cpu.regs.PB;
	}

	static void JMLDCRead(Ricoh5A22& cpu, bool skipped) {
		Word address = cpu.BufferPointer + 2; 
	    Byte value_read = cpu.read(get_pcpb(address, 0));
	    cpu.BufferBank = value_read;
	}

	static void PCOperandPBBank(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.PC = cpu.BufferOperand;
		cpu.regs.PB = cpu.BufferBank;
	}

	static void PCAddressPBBank(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.PC = cpu.BufferAddress;
		cpu.regs.PB = cpu.BufferBank;
	}

	template <typename CPUMode, typename Direction, typename Changing, typename Flag>
	static void INDE(Ricoh5A22& cpu, bool skipped) {
		int8_t adding = 1;
		if constexpr (std::is_same_v<Direction, Mode::Decrease>) {
			adding = -1;
		}
		Word* changing = nullptr;
		if constexpr (std::is_same_v<Changing, Mode::RegisterA>) {
			changing = &cpu.regs.A;
		}
		if constexpr (std::is_same_v<Changing, Mode::RegisterX>) {
			changing = &cpu.regs.X;
		}
		if constexpr (std::is_same_v<Changing, Mode::RegisterY>) {
			changing = &cpu.regs.Y;
		}
		if constexpr (std::is_same_v<Changing, Mode::Operand>) {
			changing = &cpu.BufferOperand;
		}

		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			bool flag;
			if constexpr (std::is_same_v<Flag, Mode::MFlag>) {
				flag = cpu.get_flag_M();
			} else {
				flag = cpu.get_flag_X();
			}
			if (flag) {
				Byte low = get_lo(*changing);
				low += adding;
				*changing = (get_hi(*changing) << 8) | (uint8_t)(low);
				if ((get_lo(*changing) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (get_lo(*changing) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				*changing = *changing + adding;
				if ((get_hi(*changing) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (*changing == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			Byte low = get_lo(*changing);
			low += adding;
			*changing = (get_hi(*changing) << 8) | (uint8_t)(low);
			if ((get_lo(*changing) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (get_lo(*changing) == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode>
	static void REP(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.P = cpu.regs.P & ~get_lo(cpu.BufferOperand);
			if (cpu.get_flag_X()) {
				cpu.regs.X = cpu.regs.X & 0x00FF;
				cpu.regs.Y = cpu.regs.Y & 0x00FF;
			}
		} else {
			cpu.regs.P = cpu.regs.P & ~(get_lo(cpu.BufferOperand) & ~0x30);
		}
	}

	template <typename CPUMode>
	static void SEP(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.P = cpu.regs.P | get_lo(cpu.BufferOperand);
			if (cpu.get_flag_X()) {
				cpu.regs.X = cpu.regs.X & 0x00FF;
				cpu.regs.Y = cpu.regs.Y & 0x00FF;
			}
		} else {
			cpu.regs.P = cpu.regs.P | (get_lo(cpu.BufferOperand) & ~0x30);
		}
	}

	template <typename CPUMode, typename Register, bool PCIncrement = false>
	static void CopyRegister(Ricoh5A22& cpu, bool skipped) {
		Word* reg = nullptr;
		if constexpr (std::is_same_v<Register, Mode::RegisterX>) {
			reg = &cpu.regs.X;
		} else {
			reg = &cpu.regs.Y;
		}
		if constexpr (PCIncrement) {
			cpu.regs.PC++;
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (cpu.get_flag_X()) {
				if (get_lo(*reg) >= get_lo(cpu.BufferOperand)) {
					cpu.set_flag_C();
				} else {
					cpu.clear_flag_C();
				}
				if (get_lo(*reg) == get_lo(cpu.BufferOperand)) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
				if (((get_lo(*reg) - get_lo(cpu.BufferOperand)) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
			} else {
				if (*reg >= cpu.BufferOperand) {
					cpu.set_flag_C();
				} else {
					cpu.clear_flag_C();
				}
				if (*reg == cpu.BufferOperand) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
				if (((*reg - cpu.BufferOperand) & 0x8000) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
			}
		} else {
			if (get_lo(*reg) >= get_lo(cpu.BufferOperand)) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}
			if (get_lo(*reg) == get_lo(cpu.BufferOperand)) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
			if (((get_lo(*reg) - get_lo(cpu.BufferOperand)) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
		}
	}

	static void DecrementS(Ricoh5A22& cpu, bool skipped) {
		if (!skipped) {
			cpu.regs.S -= 1;
		}
	}

	static void DecrementSLow(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.S = (0b1 << 8) | (uint8_t)(get_lo(cpu.regs.S) - 1);
	}

	static void DecrementS2(Ricoh5A22& cpu, bool skipped) {
		if (!skipped) {
			cpu.regs.S -= 2;
		}
	}

	static void DecrementS2Low(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.S = (0b1 << 8) | (uint8_t)(get_lo(cpu.regs.S) - 2);
	}

	static void DecrementS2PCAddress(Ricoh5A22& cpu, bool skipped) {
		if (!skipped) {
			cpu.regs.S -= 2;
			cpu.regs.PC = cpu.BufferAddress;
		}
	}

	static void DecrementS2LowPCAddress(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.S = (0b1 << 8) | (uint8_t)(get_lo(cpu.regs.S) - 2);
		cpu.regs.PC = cpu.BufferAddress;
	}

	static void DecrementS3(Ricoh5A22& cpu, bool skipped) {
		if (!skipped) {
			cpu.regs.S -= 3;
		}
	}

	static void DecrementS3Low(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.S = (0b1 << 8) | (uint8_t)(get_lo(cpu.regs.S) - 3);
	}

	static void DecrementS4(Ricoh5A22& cpu, bool skipped) {
		if (!skipped) {
			cpu.regs.S -= 4;
		}
	}

	static void DecrementS4Low(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.S = (0b1 << 8) | (uint8_t)(get_lo(cpu.regs.S) - 4);
	}

	static void IncrementSNativeAndReadBank(Ricoh5A22& cpu, bool skipped) {
		Word address = cpu.regs.S + 1;
		cpu.BufferBank = cpu.read(get_pcpb(address, 0));
		cpu.regs.S = address;
	}

	static void IncrementS(Ricoh5A22& cpu, bool skipped) {
		if (!skipped) {
			cpu.regs.S += 1;
		}
	}

	static void IncrementSLow(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.S = (0b1 << 8) | (uint8_t)(get_lo(cpu.regs.S) + 1);
	}

	static void IncrementS2(Ricoh5A22& cpu, bool skipped) {
		if (!skipped) {
			cpu.regs.S += 2;
		}
	}

	static void IncrementS2Low(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.S = (0b1 << 8) | (uint8_t)(get_lo(cpu.regs.S) + 2);
	}

	static void IncrementS2PCAddress(Ricoh5A22& cpu, bool skipped) {
		if (!skipped) {
			cpu.regs.S += 2;
			cpu.regs.PC = cpu.BufferAddress;
		}
	}

	static void IncrementS2LowPCAddress(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.S = (0b1 << 8) | (uint8_t)(get_lo(cpu.regs.S) + 2);
		cpu.regs.PC = cpu.BufferAddress;
	}

	template <typename CPUMode>
	static void PHP(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.BufferOperand = (get_hi(cpu.BufferOperand) << 8) | (uint8_t)(cpu.regs.P);
		} else {
			uint8_t lo = cpu.regs.P | 0x30;
			cpu.BufferOperand = (get_hi(cpu.BufferOperand) << 8) | (uint8_t)(lo);
		}
	}

	template <typename CPUMode>
	static void STXIndex(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>)  {
			cpu.BufferAddress = cpu.BufferOperand + cpu.regs.Y + cpu.regs.D;
		} else {
			if (get_lo(cpu.regs.D) == 0) {
				cpu.BufferAddress = (get_hi(cpu.BufferAddress) << 8) | (uint8_t)(cpu.BufferOperand + get_lo(cpu.regs.Y) + get_lo(cpu.regs.D));
				cpu.BufferAddress = (get_hi(cpu.regs.D) << 8) | get_lo(cpu.BufferAddress);
			} else {
				cpu.BufferAddress = cpu.BufferOperand + cpu.regs.Y + cpu.regs.D;
			}
		}
	}

	template <typename CPUMode>
	static void STYIndex(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>)  {
			cpu.BufferAddress = cpu.BufferOperand + cpu.regs.X + cpu.regs.D;
		} else {
			if (get_lo(cpu.regs.D) == 0) {
				cpu.BufferAddress = (get_hi(cpu.BufferAddress) << 8) | (uint8_t)(cpu.BufferOperand + get_lo(cpu.regs.X) + get_lo(cpu.regs.D));
				cpu.BufferAddress = (get_hi(cpu.regs.D) << 8) | get_lo(cpu.BufferAddress);
			} else {
				cpu.BufferAddress = cpu.BufferOperand + cpu.regs.X + cpu.regs.D;
			}
		}
	}

	static void JSRIndex(Ricoh5A22& cpu, bool skipped) {
		cpu.BufferPointer += cpu.regs.X;
		cpu.BufferBank = cpu.regs.PB;
	}

	static void PCAddress(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.PC = cpu.BufferAddress;
	}

	template <typename CPUMode>
	static void JSL(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.S -= 3;
			cpu.regs.PC = cpu.BufferAddress;
			cpu.regs.PB = cpu.BufferBank;
		} else {
			cpu.regs.S = (0b1 << 8) | (get_lo(cpu.regs.S) - 3);
			cpu.regs.PC = cpu.BufferAddress;
			cpu.regs.PB = cpu.BufferBank;
		}
	}

	template <typename CPUMode, typename Register, typename Flag>
	static void PL(Ricoh5A22& cpu, bool skipped) {
		Word* reg = nullptr;
		if constexpr (std::is_same_v<Register, Mode::RegisterA>) {
			reg = &cpu.regs.A;
		} else if constexpr (std::is_same_v<Register, Mode::RegisterX>) {
			reg = &cpu.regs.X;
		} else {
			reg = &cpu.regs.Y;
		}
		bool flag = false;
		if constexpr (std::is_same_v<Flag, Mode::MFlag>) {
			flag = cpu.get_flag_M();
		} else {
			flag = cpu.get_flag_X();
		}
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			if (flag) {
				*reg = (get_hi(*reg) << 8) | get_lo(cpu.BufferOperand);
				if ((get_lo(*reg) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (get_lo(*reg) == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			} else {
				cpu.regs.S += 1;
				*reg = cpu.BufferOperand;
				if ((get_hi(*reg) & 0x80) != 0) {
					cpu.set_flag_N();
				} else {
					cpu.clear_flag_N();
				}
				if (*reg == 0) {
					cpu.set_flag_Z();
				} else {
					cpu.clear_flag_Z();
				}
			}
		} else {
			*reg = (get_hi(*reg) << 8) | get_lo(cpu.BufferOperand);
			if ((get_lo(*reg) & 0x80) != 0) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (get_lo(*reg) == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <typename CPUMode>
	static void PLP(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.P = get_lo(cpu.BufferOperand);
			if (cpu.get_flag_X()) {
				cpu.regs.X = cpu.regs.X & 0x00FF;
				cpu.regs.Y = cpu.regs.Y & 0x00FF;
			}
		} else {
			cpu.regs.P = get_lo(cpu.BufferOperand);
			cpu.set_flag_X();
			cpu.set_flag_M();
			cpu.regs.X = cpu.regs.X & 0x00FF;
			cpu.regs.Y = cpu.regs.Y & 0x00FF;	
		}
	}

	template <typename CPUMode>
	static void PLB(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.DB = cpu.BufferBank;
		if ((cpu.regs.DB & 0x80) != 0) {
			cpu.set_flag_N();
		} else {
			cpu.clear_flag_N();
		}
		if (cpu.regs.DB == 0) {
			cpu.set_flag_Z();
		} else {
			cpu.clear_flag_Z();
		}
	}

	template <typename CPUMode>
	static void PLD(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.S += 2;
		} else {
			cpu.regs.S = (0b1 << 8) | (get_lo(cpu.regs.S) + 2); 
		}
		cpu.regs.D = cpu.BufferOperand;
		if ((get_hi(cpu.regs.D) & 0x80) != 0) {
			cpu.set_flag_N();
		} else {
			cpu.clear_flag_N();
		}
		if (cpu.regs.D == 0) {
			cpu.set_flag_Z();
		} else {
			cpu.clear_flag_Z();
		}
	}

	template <typename CPUMode>
	static void RTS(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.S += 1;
		} else {
			cpu.regs.S = (0b1 << 8) | (get_lo(cpu.regs.S) + 1);
		}
		cpu.regs.PC = cpu.BufferOperand + 1;
	}

	template <typename CPUMode>
	static void POperand(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.P = cpu.BufferOperand;
		} else {
			cpu.regs.P = ((get_lo(cpu.BufferOperand) & ~0x30) | (cpu.regs.P & 0x30));
		}
	}

	template <typename CPUMode>
	static void RTI(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.S += 3;
			cpu.regs.PC = cpu.BufferOperand;
			if (cpu.get_flag_X()) {
				cpu.regs.X = get_lo(cpu.regs.X);
				cpu.regs.Y = get_lo(cpu.regs.Y);
			}
		} else {
			cpu.regs.S = (0b1 << 8) | (get_lo(cpu.regs.S) + 2);
			cpu.regs.PC = cpu.BufferOperand;
		}
	}

	template <typename CPUMode>
	static void RTL(Ricoh5A22& cpu, bool skipped) {
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			cpu.regs.S += 3;
		} else {
			cpu.regs.S = (0b1 << 8) | (get_lo(cpu.regs.S) + 3);
		}
		cpu.regs.PC = (cpu.BufferOperand + 1);
	}

	static void SetIUnsetD(Ricoh5A22& cpu, bool skipped) {
		cpu.set_flag_I();
		cpu.clear_flag_D();
	}

	static void COP(Ricoh5A22& cpu, bool skipped) {
		cpu.regs.PC = cpu.BufferAddress;
		cpu.regs.PB = 0;
	}

	static void PushStatusWithBreakFlag(Ricoh5A22& cpu, bool skipped) {
		Byte value = cpu.regs.P | 0x10;
		Address address = 0x0100 | (uint8_t)(get_lo(cpu.regs.S) - 2);

		if constexpr (SST_TEST) {
			cpu.test_poke(address, value);
		} else {
			cpu.write(address, value);
		}
	}

	template<typename CPUMode>
	static void SetVectorNMI(Ricoh5A22& cpu, bool skipped) {
	    if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
	        cpu.Vector = 0xFFEA;
	    } else {
	        cpu.Vector = 0xFFFA;
	    }
	}

	template<typename CPUMode>
	static void SetVectorIRQ(Ricoh5A22& cpu, bool skipped) {
	    if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
	        cpu.Vector = 0xFFEE;
	    } else {
	        cpu.Vector = 0xFFFE;   // same vector as SetVectorBRK<Emulation> — see note below
	    }
	}

	static void PushStatusClearBreakFlag(Ricoh5A22& cpu, bool skipped) {
	    Byte value = cpu.regs.P & ~0x10;
	    Address address = 0x0100 | (uint8_t)(get_lo(cpu.regs.S) - 2);
	    if constexpr (SST_TEST) {
	        cpu.test_poke(address, value);
	    } else {
	        cpu.write(address, value);
	    }
	}
}

namespace Ricoh5A22Predicates {

	static bool Never(Ricoh5A22& cpu) {
		PREDICATE_CHECK_ROUTINE
		return false;
	}

	static bool Even(Ricoh5A22& cpu) {
		PREDICATE_CHECK_ROUTINE
		return ( (cpu.regs.PC & 0b1) == 0);
	}

	static bool Odd(Ricoh5A22& cpu) {
		PREDICATE_CHECK_ROUTINE
		return ( (cpu.regs.PC & 0b1) == 1);
	}

	static bool DLZero(Ricoh5A22& cpu) {
		PREDICATE_CHECK_ROUTINE
		return get_lo(cpu.regs.D) == 0;
	}

	static bool MFlagSet(Ricoh5A22& cpu) {
		PREDICATE_CHECK_ROUTINE
		return cpu.get_flag_M();
	}

	static bool XFlagSet(Ricoh5A22& cpu) {
		PREDICATE_CHECK_ROUTINE
		return cpu.get_flag_X();
	}

	template <typename CPUMode = Mode::Native>
	static bool ReadingCondition(Ricoh5A22& cpu) {
		PREDICATE_CHECK_ROUTINE
		if constexpr (std::is_same_v<CPUMode, Mode::Native>) {
			return (get_hi(cpu.BufferOrig) == get_hi(cpu.BufferPointer) && cpu.get_flag_X() == true);
		} else {
			return (get_hi(cpu.BufferOrig) == get_hi(cpu.BufferPointer));
		}
	}

	static bool NoBranching(Ricoh5A22& cpu) {
		PREDICATE_CHECK_ROUTINE
		return !cpu.Branching;
	}

	static bool NoBoundaryCrossed(Ricoh5A22& cpu) {
		PREDICATE_CHECK_ROUTINE
		return !cpu.BoundaryCrossed;
	}

	static bool NoBranchingOrNoBoundaryCrossed(Ricoh5A22& cpu) {
		PREDICATE_CHECK_ROUTINE
		return !cpu.Branching || !cpu.BoundaryCrossed;
	}
}

// Interrupt routines
// NMI
Instruction<Ricoh5A22> n_nmi = {
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::PB, WriteTo::Stack0>),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::StackMinus1>),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus2>),
    MakeHandler(Ricoh5A22Functions::SetVectorNMI<Mode::Native>),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::P, WriteTo::StackMinus3>),
    MakeHandler(Ricoh5A22Functions::DecrementS4),
    MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Vector, ReadTo::AddressLow>),
    MakeHandler(Ricoh5A22Functions::SetIUnsetD),
    MakeHandler(Ricoh5A22Functions::Read<ReadFrom::VectorPlusOne, ReadTo::AddressHigh>),
    MakeHandler(Ricoh5A22Functions::COP),
    NEXT_OPCODE
};

Instruction<Ricoh5A22> e_nmi = {
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::Stack0Emulation>),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1Emulation>),
    MakeHandler(Ricoh5A22Functions::SetVectorNMI<Mode::Emulation>),
    MakeHandler(Ricoh5A22Functions::PushStatusClearBreakFlag),
    MakeHandler(Ricoh5A22Functions::DecrementS3Low),
    MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Vector, ReadTo::AddressLow>),
    MakeHandler(Ricoh5A22Functions::SetIUnsetD),
    MakeHandler(Ricoh5A22Functions::Read<ReadFrom::VectorPlusOne, ReadTo::AddressHigh>),
    MakeHandler(Ricoh5A22Functions::COP),
    NEXT_OPCODE
};

// IRQ
Instruction<Ricoh5A22> n_irq = {
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::PB, WriteTo::Stack0>),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::StackMinus1>),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus2>),
    MakeHandler(Ricoh5A22Functions::SetVectorIRQ<Mode::Native>),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::P, WriteTo::StackMinus3>),
    MakeHandler(Ricoh5A22Functions::DecrementS4),
    MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Vector, ReadTo::AddressLow>),
    MakeHandler(Ricoh5A22Functions::SetIUnsetD),
    MakeHandler(Ricoh5A22Functions::Read<ReadFrom::VectorPlusOne, ReadTo::AddressHigh>),
    MakeHandler(Ricoh5A22Functions::COP),
    NEXT_OPCODE
};
Instruction<Ricoh5A22> e_irq = {
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::Stack0Emulation>),
    MakeHandler(Ricoh5A22Functions::NOP),
    MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1Emulation>),
    MakeHandler(Ricoh5A22Functions::SetVectorIRQ<Mode::Emulation>),
    MakeHandler(Ricoh5A22Functions::PushStatusClearBreakFlag),
    MakeHandler(Ricoh5A22Functions::DecrementS3Low),
    MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Vector, ReadTo::AddressLow>),
    MakeHandler(Ricoh5A22Functions::SetIUnsetD),
    MakeHandler(Ricoh5A22Functions::Read<ReadFrom::VectorPlusOne, ReadTo::AddressHigh>),
    MakeHandler(Ricoh5A22Functions::COP),
    NEXT_OPCODE
};

// Main Instruction<Ricoh5A22>s

// BRK (00)
Instruction<Ricoh5A22> n_00 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PB, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus2>),
	MakeHandler(Ricoh5A22Functions::SetVectorBRK<Mode::Native>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::P, WriteTo::StackMinus3>),
	MakeHandler(Ricoh5A22Functions::DecrementS4),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Vector, ReadTo::AddressLow>),
	MakeHandler(Ricoh5A22Functions::SetIUnsetD),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::VectorPlusOne, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::COP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_00 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::Stack0Emulation>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1Emulation>),
	MakeHandler(Ricoh5A22Functions::SetVectorBRK<Mode::Emulation>),
	MakeHandler(Ricoh5A22Functions::PushStatusWithBreakFlag),
	MakeHandler(Ricoh5A22Functions::DecrementS3Low),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Vector, ReadTo::AddressLow>),
	MakeHandler(Ricoh5A22Functions::SetIUnsetD),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::VectorPlusOne, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::COP),
	NEXT_OPCODE
};

// ORA (01)
Instruction<Ricoh5A22> n_01 = {
	NATIVE_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_01 = {
	EMULATION_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// COP (02)
Instruction<Ricoh5A22> n_02 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PB, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus2>),
	MakeHandler(Ricoh5A22Functions::SetVector<Mode::Native>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::P, WriteTo::StackMinus3>),
	MakeHandler(Ricoh5A22Functions::DecrementS4),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Vector, ReadTo::AddressLow>),
	MakeHandler(Ricoh5A22Functions::SetIUnsetD),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::VectorPlusOne, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::COP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_02 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::Stack0Emulation>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1Emulation>),
	MakeHandler(Ricoh5A22Functions::SetVector<Mode::Emulation>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::P, WriteTo::StackMinus2Emulation>),
	MakeHandler(Ricoh5A22Functions::DecrementS3Low),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Vector, ReadTo::AddressLow>),
	MakeHandler(Ricoh5A22Functions::SetIUnsetD),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::VectorPlusOne, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::COP),
	NEXT_OPCODE
};

// ORA (03)
Instruction<Ricoh5A22> n_03 = {
	NATIVE_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_03 = {
	EMULATION_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// TSB (04)
Instruction<Ricoh5A22> n_04 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::TB<Mode::Native, Mode::Set>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_04 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::TB<Mode::Emulation, Mode::Set>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// ORA (05)
Instruction<Ricoh5A22> n_05 = {
	NATIVE_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_05 = {
	EMULATION_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// ASL (06)
Instruction<Ricoh5A22> n_06 = {
	NATIVE_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ASL<Mode::Native>),
	NATIVE_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_06 = {
	EMULATION_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ASL<Mode::Emulation>),
	EMULATION_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// ORA (07)
Instruction<Ricoh5A22> n_07 = {
	NATIVE_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_07 = {
	EMULATION_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// PHP (08)
Instruction<Ricoh5A22> n_08 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::PHP<Mode::Native>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::DecrementS),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_08 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::PHP<Mode::Emulation>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Stack0Emulation>),
	MakeHandler(Ricoh5A22Functions::DecrementSLow),
	NEXT_OPCODE
};

// ORA (09)
Instruction<Ricoh5A22> n_09 = {
	NATIVE_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native, Mode::PCIncrement>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_09 = {
	EMULATION_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation, Mode::PCIncrement>),
	NEXT_OPCODE
};

// ASL (0A)
Instruction<Ricoh5A22> n_0a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::ASL<Mode::Native, Mode::RegisterA>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_0a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::ASL<Mode::Emulation, Mode::RegisterA>),
	NEXT_OPCODE
};

// PHD (0B)
Instruction<Ricoh5A22> n_0b = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::DHigh, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::DLow, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::DecrementS2),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_0b = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::DHigh, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::DLow, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::DecrementS2Low),
	NEXT_OPCODE
};

// TSB (0C)
Instruction<Ricoh5A22> n_0c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressDB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusOneDBCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::TB<Mode::Native, Mode::Set>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOneDBCarry>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_0c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressDB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::TB<Mode::Emulation, Mode::Set>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// ORA (0D)
Instruction<Ricoh5A22> n_0d = {
	NATIVE_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_0d = {
	EMULATION_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// ASL (0E)
Instruction<Ricoh5A22> n_0e = {
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ASL<Mode::Native>),
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_0e = {
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ASL<Mode::Emulation>),
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// ORA (0F)
Instruction<Ricoh5A22> n_0f = {
	NATIVE_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_0f = {
	EMULATION_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// BPL (10)
Instruction<Ricoh5A22> n_10 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::N_Zero>),
	NATIVE_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_10 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::N_Zero>),
	EMULATION_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};

// ORA (11)
Instruction<Ricoh5A22> n_11 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_11 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// ORA (12)
Instruction<Ricoh5A22> n_12 = {
	NATIVE_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_12 = {
	EMULATION_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// ORA (13)
Instruction<Ricoh5A22> n_13 = {
	NATIVE_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_13 = {
	EMULATION_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// TRB (14)
Instruction<Ricoh5A22> n_14 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::TB<Mode::Native, Mode::Reset>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_14 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::TB<Mode::Emulation, Mode::Reset>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// ORA (15)
Instruction<Ricoh5A22> n_15 = {
	NATIVE_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_15 = {
	EMULATION_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// ASL (16)
Instruction<Ricoh5A22> n_16 = {
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ASL<Mode::Native>),
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_16 = {
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ASL<Mode::Emulation>),
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// ORA (17)
Instruction<Ricoh5A22> n_17 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_17 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// CLC (18)
Instruction<Ricoh5A22> n_18 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::CLC),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_18 = n_18;

// ORA (19)
Instruction<Ricoh5A22> n_19 = {
	NATIVE_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_19 = {
	EMULATION_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// INC (1A)
Instruction<Ricoh5A22> n_1a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Increase, Mode::RegisterA, Mode::MFlag>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_1a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Increase, Mode::RegisterA, Mode::MFlag>),
	NEXT_OPCODE
};

// TCS (1B)
Instruction<Ricoh5A22> n_1b = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TCS<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_1b = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TCS<Mode::Emulation>),
	NEXT_OPCODE
};


// TRB (1C)
Instruction<Ricoh5A22> n_1c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressDB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusOneDBCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::TB<Mode::Native, Mode::Reset>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOneDBCarry>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_1c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressDB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::TB<Mode::Emulation, Mode::Reset>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// ORA (1D)
Instruction<Ricoh5A22> n_1d = {
	NATIVE_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_1d = {
	EMULATION_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// ASL (1E)
Instruction<Ricoh5A22> n_1e = {
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ASL<Mode::Native>),
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_1e = {
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ASL<Mode::Emulation>),
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// ORA (1F)
Instruction<Ricoh5A22> n_1f = {
	NATIVE_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_1f = {
	EMULATION_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::ORA<Mode::Emulation>),
	NEXT_OPCODE
};

// JSR (20)
Instruction<Ricoh5A22> n_20 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::DecrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::DecrementS2PCAddress),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_20 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::DecrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::Stack0Emulation>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1Emulation>),
	MakeHandler(Ricoh5A22Functions::DecrementS2LowPCAddress),
	NEXT_OPCODE
};

// AND (21)
Instruction<Ricoh5A22> n_21 = {
	NATIVE_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_21 = {
	EMULATION_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// JSL (22)
Instruction<Ricoh5A22> n_22 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PB, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus2>),
	MakeHandler(Ricoh5A22Functions::JSL<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_22 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PB, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus2>),
	MakeHandler(Ricoh5A22Functions::JSL<Mode::Emulation>),
	NEXT_OPCODE
};

// AND (23)
Instruction<Ricoh5A22> n_23 = {
	NATIVE_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_23 = {
	EMULATION_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// BIT (24)
Instruction<Ricoh5A22> n_24 = {
	NATIVE_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::BIT<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_24 = {
	EMULATION_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::BIT<Mode::Emulation>),
	NEXT_OPCODE
};

// AND (25)
Instruction<Ricoh5A22> n_25 = {
	NATIVE_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_25 = {
	EMULATION_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// ROL (26)
Instruction<Ricoh5A22> n_26 = {
	NATIVE_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROL<Mode::Native>),
	NATIVE_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_26 = {
	EMULATION_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROL<Mode::Emulation>),
	EMULATION_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// AND (27)
Instruction<Ricoh5A22> n_27 = {
	NATIVE_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_27 = {
	EMULATION_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// PLP (28)
Instruction<Ricoh5A22> n_28 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementS),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::PLP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_28 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementSLow),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0Emulation, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::PLP<Mode::Emulation>),
	NEXT_OPCODE
};

// AND (29)
Instruction<Ricoh5A22> n_29 = {
	NATIVE_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native, Mode::PCIncrement>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_29 = {
	EMULATION_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation, Mode::PCIncrement>),
	NEXT_OPCODE
};

// ROL (2A)
Instruction<Ricoh5A22> n_2a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::ROL<Mode::Native, Mode::RegisterA>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_2a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::ROL<Mode::Emulation, Mode::RegisterA>),
	NEXT_OPCODE
};

// PLD (2B)
Instruction<Ricoh5A22> n_2b = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack1, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack2, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::PLD<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_2b = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack1, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack2, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::PLD<Mode::Emulation>),
	NEXT_OPCODE
};

// BIT (2C)
Instruction<Ricoh5A22> n_2c = {
	NATIVE_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::BIT<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_2c = {
	EMULATION_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::BIT<Mode::Emulation>),
	NEXT_OPCODE
};

// AND (2D)
Instruction<Ricoh5A22> n_2d = {
	NATIVE_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_2d = {
	EMULATION_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// ROL (2E)
Instruction<Ricoh5A22> n_2e = {
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROL<Mode::Native>),
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_2e = {
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROL<Mode::Emulation>),
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// AND (2F)
Instruction<Ricoh5A22> n_2f = {
	NATIVE_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_2f = {
	EMULATION_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// BMI (30)
Instruction<Ricoh5A22> n_30 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::N_One>),
	NATIVE_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_30 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::N_One>),
	EMULATION_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};

// AND (31)
Instruction<Ricoh5A22> n_31 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_31 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// AND (32)
Instruction<Ricoh5A22> n_32 = {
	NATIVE_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_32 = {
	EMULATION_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// AND (33)
Instruction<Ricoh5A22> n_33 = {
	NATIVE_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_33 = {
	EMULATION_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// BIT (34)
Instruction<Ricoh5A22> n_34 = {
	NATIVE_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::BIT<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_34 = {
	EMULATION_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::BIT<Mode::Emulation>),
	NEXT_OPCODE
};

// AND (35)
Instruction<Ricoh5A22> n_35 = {
	NATIVE_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_35 = {
	EMULATION_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// ROL (36)
Instruction<Ricoh5A22> n_36 = {
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROL<Mode::Native>),
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_36 = {
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROL<Mode::Emulation>),
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// AND (37)
Instruction<Ricoh5A22> n_37 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_37 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// SEC (38)
Instruction<Ricoh5A22> n_38 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::SEC),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_38 = n_38;

// AND (39)
Instruction<Ricoh5A22> n_39 = {
	NATIVE_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_39 = {
	EMULATION_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// DEC (3A)
Instruction<Ricoh5A22> n_3a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Decrease, Mode::RegisterA, Mode::MFlag>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_3a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Decrease, Mode::RegisterA, Mode::MFlag>),
	NEXT_OPCODE
};

// TSC (3B)
Instruction<Ricoh5A22> n_3b = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TSC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_3b = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TSC<Mode::Emulation>),
	NEXT_OPCODE
};

// BIT (3C)
Instruction<Ricoh5A22> n_3c = {
	NATIVE_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::BIT<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_3c = {
	EMULATION_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::BIT<Mode::Emulation>),
	NEXT_OPCODE
};

// AND (3D)
Instruction<Ricoh5A22> n_3d = {
	NATIVE_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_3d = {
	EMULATION_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// ROL (3E)
Instruction<Ricoh5A22> n_3e = {
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROL<Mode::Native>),
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_3e = {
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROL<Mode::Emulation>),
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// AND (3F)
Instruction<Ricoh5A22> n_3f = {
	NATIVE_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_3f = {
	EMULATION_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::AND<Mode::Emulation>),
	NEXT_OPCODE
};

// RTI (40)
Instruction<Ricoh5A22> n_40 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementS),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::POperand<Mode::Native>),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack1, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack2, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack3, ReadTo::PB>),
	MakeHandler(Ricoh5A22Functions::RTI<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_40 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementSLow),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0Emulation, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::POperand<Mode::Emulation>),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack1Emulation, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack2Emulation, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::RTI<Mode::Emulation>),
	NEXT_OPCODE
};

// EOR (41)
Instruction<Ricoh5A22> n_41 = {
	NATIVE_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_41 = {
	EMULATION_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// WDM (42)
Instruction<Ricoh5A22> n_42 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_42 = n_42;

// EOR (43)
Instruction<Ricoh5A22> n_43 = {
	NATIVE_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_43 = {
	EMULATION_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// MVP (44)
Instruction<Ricoh5A22> n_44 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::DBOL>),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::XBank, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::YDB>),
	MakeHandler(Ricoh5A22Functions::MVP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_44 = n_44;


// EOR (45)
Instruction<Ricoh5A22> n_45 = {
	NATIVE_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_45 = {
	EMULATION_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// LSR (46)
Instruction<Ricoh5A22> n_46 = {
	NATIVE_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::LSR<Mode::Native>),
	NATIVE_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_46 = {
	EMULATION_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::LSR<Mode::Emulation>),
	EMULATION_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// EOR (47)
Instruction<Ricoh5A22> n_47 = {
	NATIVE_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_47 = {
	EMULATION_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// PHA (48)
Instruction<Ricoh5A22> n_48 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::AHigh, WriteTo::Stack0>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::DecrementS),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::ALow, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::DecrementS),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_48 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::ALow, WriteTo::Stack0Emulation>),
	MakeHandler(Ricoh5A22Functions::DecrementSLow),
	NEXT_OPCODE
};

// EOR (49)
Instruction<Ricoh5A22> n_49 = {
	NATIVE_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native, Mode::PCIncrement>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_49 = {
	EMULATION_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation, Mode::PCIncrement>),
	NEXT_OPCODE
};

// LSR (4A)
Instruction<Ricoh5A22> n_4a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::LSR<Mode::Native, Mode::RegisterA>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_4a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::LSR<Mode::Emulation, Mode::RegisterA>),
	NEXT_OPCODE
};

// PHK (4B)
Instruction<Ricoh5A22> n_4b = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PB, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::DecrementS),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_4b = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PB, WriteTo::Stack0Emulation>),
	MakeHandler(Ricoh5A22Functions::DecrementSLow),
	NEXT_OPCODE
};

// JMP (4C)
Instruction<Ricoh5A22> n_4c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::Address, ReadTo::PC>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_4c = n_4c;

// EOR (4D)
Instruction<Ricoh5A22> n_4d = {
	NATIVE_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_4d = {
	EMULATION_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// LSR (4E)
Instruction<Ricoh5A22> n_4e = {
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::LSR<Mode::Native>),
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_4e = {
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::LSR<Mode::Emulation>),
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// EOR (4F)
Instruction<Ricoh5A22> n_4f = {
	NATIVE_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_4f = {
	EMULATION_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// BVC (50)
Instruction<Ricoh5A22> n_50 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::V_Zero>),
	NATIVE_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_50 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::V_Zero>),
	EMULATION_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};

// EOR (51)
Instruction<Ricoh5A22> n_51 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_51 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// EOR (52)
Instruction<Ricoh5A22> n_52 = {
	NATIVE_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_52 = {
	EMULATION_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// EOR (53)
Instruction<Ricoh5A22> n_53 = {
	NATIVE_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_53 = {
	EMULATION_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// MVN (54)
Instruction<Ricoh5A22> n_54 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::DBOL>),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::XBank, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::YDB>),
	MakeHandler(Ricoh5A22Functions::MVN),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_54 = n_54;

// EOR (55)
Instruction<Ricoh5A22> n_55 = {
	NATIVE_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_55 = {
	EMULATION_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// LSR (56)
Instruction<Ricoh5A22> n_56 = {
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::LSR<Mode::Native>),
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_56 = {
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::LSR<Mode::Emulation>),
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// EOR (57)
Instruction<Ricoh5A22> n_57 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_57 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// CLI (58)
Instruction<Ricoh5A22> n_58 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::CLI),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_58 = n_58;

// EOR (59)
Instruction<Ricoh5A22> n_59 = {
	NATIVE_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_59 = {
	EMULATION_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// PHY (5A)
Instruction<Ricoh5A22> n_5a = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::YHigh, WriteTo::Stack0>, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::DecrementS),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::YLow, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::DecrementS),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_5a = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::YLow, WriteTo::Stack0Emulation>),
	MakeHandler(Ricoh5A22Functions::DecrementSLow),
	NEXT_OPCODE
};

// TCD (5B)
Instruction<Ricoh5A22> n_5b = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TCD<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_5b = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TCD<Mode::Emulation>),
	NEXT_OPCODE
};

// JML (5C)
Instruction<Ricoh5A22> n_5c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>),
	MakeHandler(Ricoh5A22Functions::PCAddressPBBank),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_5c = n_5c;

// EOR (5D)
Instruction<Ricoh5A22> n_5d = {
	NATIVE_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_5d = {
	EMULATION_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// LSR (5E)
Instruction<Ricoh5A22> n_5e = {
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::LSR<Mode::Native>),
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_5e = {
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::LSR<Mode::Emulation>),
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// EOR (5F)
Instruction<Ricoh5A22> n_5f = {
	NATIVE_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_5f = {
	EMULATION_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::EOR<Mode::Emulation>),
	NEXT_OPCODE
};

// RTS (60)
Instruction<Ricoh5A22> n_60 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementS),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack1, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::RTS<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_60 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementSLow),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0Emulation, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack1Emulation, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::RTS<Mode::Emulation>),
	NEXT_OPCODE
};

// ADC (61)
Instruction<Ricoh5A22> n_61 = {
	NATIVE_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_61 = {
	EMULATION_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// PER (62)
Instruction<Ricoh5A22> n_62 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::OOPC>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::DecrementS2),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_62 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::OOPC>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::DecrementS2Low),
	NEXT_OPCODE
};

// ADC (63)
Instruction<Ricoh5A22> n_63 = {
	NATIVE_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_63 = {
	EMULATION_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// STZ (64)
Instruction<Ricoh5A22> n_64 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::Zero, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::Zero, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_64 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::Zero, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE	
};

// ADC (65)
Instruction<Ricoh5A22> n_65 = {
	NATIVE_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_65 = {
	EMULATION_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// ROR (66)
Instruction<Ricoh5A22> n_66 = {
	NATIVE_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROR<Mode::Native>),
	NATIVE_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_66 = {
	EMULATION_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROR<Mode::Emulation>),
	EMULATION_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// ADC (67)
Instruction<Ricoh5A22> n_67 = {
	NATIVE_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_67 = {
	EMULATION_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// PLA (68)
Instruction<Ricoh5A22> n_68 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementS),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack1, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::PL<Mode::Native, Mode::RegisterA, Mode::MFlag>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_68 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementSLow),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::PL<Mode::Emulation, Mode::RegisterA, Mode::MFlag>),
	NEXT_OPCODE
};

// ADC (69)
Instruction<Ricoh5A22> n_69 = {
	NATIVE_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native, Mode::PCIncrement>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_69 = {
	EMULATION_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation, Mode::PCIncrement>),
	NEXT_OPCODE
};

// ROR (6A)
Instruction<Ricoh5A22> n_6a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::ROR<Mode::Native, Mode::RegisterA>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_6a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::ROR<Mode::Emulation, Mode::RegisterA>),
	NEXT_OPCODE
};

// RTL (6B)
Instruction<Ricoh5A22> n_6b = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack1, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack2, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack3, ReadTo::PB>),
	MakeHandler(Ricoh5A22Functions::RTL<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_6b = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack1, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack2, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack3, ReadTo::PB>),
	MakeHandler(Ricoh5A22Functions::RTL<Mode::Emulation>),
	NEXT_OPCODE
};

// JMP (6C)
Instruction<Ricoh5A22> n_6c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Pointer, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOne, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::Address, ReadTo::PC>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_6c = n_6c;

// ADC (6D)
Instruction<Ricoh5A22> n_6d = {
	NATIVE_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_6d = {
	EMULATION_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// ROR (6E)
Instruction<Ricoh5A22> n_6e = {
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROR<Mode::Native>),
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_6e = {
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROR<Mode::Emulation>),
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// ADC (6F)
Instruction<Ricoh5A22> n_6f = {
	NATIVE_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_6f = {
	EMULATION_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// BVS (70)
Instruction<Ricoh5A22> n_70 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::V_One>),
	NATIVE_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_70 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::V_One>),
	EMULATION_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};

// ADC (71)
Instruction<Ricoh5A22> n_71 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_71 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// ADC (72)
Instruction<Ricoh5A22> n_72 = {
	NATIVE_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_72 = {
	EMULATION_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// ADC (73)
Instruction<Ricoh5A22> n_73 = {
	NATIVE_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_73 = {
	EMULATION_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// STZ (74)
Instruction<Ricoh5A22> n_74 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::STYIndex<Mode::Native>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::Zero, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::Zero, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_74 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::STYIndex<Mode::Emulation>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::Zero, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// ADC (75)
Instruction<Ricoh5A22> n_75 = {
	NATIVE_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_75 = {
	EMULATION_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// ROR (76)
Instruction<Ricoh5A22> n_76 = {
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROR<Mode::Native>),
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_76 = {
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROR<Mode::Emulation>),
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// ADC (77)
Instruction<Ricoh5A22> n_77 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_77 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// SEI (78)
Instruction<Ricoh5A22> n_78 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::SEI),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_78 = n_78;


// ADC (79)
Instruction<Ricoh5A22> n_79 = {
	NATIVE_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_79 = {
	EMULATION_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// PLY (7A)
Instruction<Ricoh5A22> n_7a = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementS),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack1, ReadTo::OperandHigh>, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::PL<Mode::Native, Mode::RegisterY, Mode::XFlag>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_7a = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementSLow),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::PL<Mode::Emulation, Mode::RegisterY, Mode::XFlag>),
	NEXT_OPCODE
};

// TDC (7B)
Instruction<Ricoh5A22> n_7b = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TDC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_7b = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TDC<Mode::Emulation>),
	NEXT_OPCODE
};

// JMP (7C)
Instruction<Ricoh5A22> n_7c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>),
	MakeHandler(Ricoh5A22Functions::JMPOp),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::AddressLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankNoCarry, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::Address, ReadTo::PC>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_7c = n_7c;

// ADC (7D)
Instruction<Ricoh5A22> n_7d = {
	NATIVE_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_7d = {
	EMULATION_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};

// ROR (7E)
Instruction<Ricoh5A22> n_7e = {
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROR<Mode::Native>),
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_7e = {
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::ROR<Mode::Emulation>),
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// ADC (7F)
Instruction<Ricoh5A22> n_7f = {
	NATIVE_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_7f = {
	EMULATION_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::ADC<Mode::Emulation>),
	NEXT_OPCODE
};


// BRA (80)
Instruction<Ricoh5A22> n_80 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::Always>),
	NATIVE_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_80 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::Always>),
	EMULATION_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};

// STA (81)
Instruction<Ricoh5A22> n_81 = {
	NATIVE_DIRECT_INDEXED_INDIRECT_D_X_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_81 = {
	EMULATION_DIRECT_INDEXED_INDIRECT_D_X_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// BRL (82)
Instruction<Ricoh5A22> n_82 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::BRL),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_82 = n_82;

// STA (83)
Instruction<Ricoh5A22> n_83 = {
	NATIVE_STACK_RELATIVE_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_83 = {
	EMULATION_STACK_RELATIVE_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STY (84)
Instruction<Ricoh5A22> n_84 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::YLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::YHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_84 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::YLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE	
};

// STA (85)
Instruction<Ricoh5A22> n_85 = {
	NATIVE_DIRECT_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_85 = {
	EMULATION_DIRECT_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STX (86)
Instruction<Ricoh5A22> n_86 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::XLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::XHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_86 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::XLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE	
};

// STA (87)
Instruction<Ricoh5A22> n_87 = {
	NATIVE_DIRECT_INDIRECT_LONG_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_87 = {
	EMULATION_DIRECT_INDIRECT_LONG_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// DEY (88)
Instruction<Ricoh5A22> n_88 = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Decrease, Mode::RegisterY, Mode::XFlag>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_88 = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Decrease, Mode::RegisterY, Mode::XFlag>),
	NEXT_OPCODE
};

// TXA (8A)
Instruction<Ricoh5A22> n_8a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TXA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_8a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TXA<Mode::Emulation>),
	NEXT_OPCODE
};

// PHB (8B)
Instruction<Ricoh5A22> n_8b = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::DB, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::DecrementS),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_8b = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::DB, WriteTo::Stack0Emulation>),
	MakeHandler(Ricoh5A22Functions::DecrementSLow),
	NEXT_OPCODE
};

// STY (8C)
Instruction<Ricoh5A22> n_8c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::OY>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOneDBCarry>, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_8c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::OY>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STA (8D)
Instruction<Ricoh5A22> n_8d = {
	NATIVE_ABSOLUTE_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_8d = {
	EMULATION_ABSOLUTE_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STX (8E)
Instruction<Ricoh5A22> n_8e = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::OX>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOneDBCarry>, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_8e = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::OX>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STA (8F)
Instruction<Ricoh5A22> n_8f = {
	NATIVE_ABSOLUTE_LONG_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_8f = {
	EMULATION_ABSOLUTE_LONG_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// BIT (89)
Instruction<Ricoh5A22> n_89 = {
	NATIVE_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::BIT<Mode::Native, Mode::IsImmediate>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_89 = {
	EMULATION_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::BIT<Mode::Emulation, Mode::IsImmediate>),
	NEXT_OPCODE
};

// BCC (90)
Instruction<Ricoh5A22> n_90 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::C_Zero>),
	NATIVE_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_90 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::C_Zero>),
	EMULATION_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};

// STA (91)
Instruction<Ricoh5A22> n_91 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_D_Y_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_91 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_D_Y_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STA (92)
Instruction<Ricoh5A22> n_92 = {
	NATIVE_DIRECT_INDIRECT_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_92 = {
	EMULATION_DIRECT_INDIRECT_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STA (93)
Instruction<Ricoh5A22> n_93 = {
	NATIVE_STACK_RELATIVE_INDIRECT_INDEXED_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_93 = {
	EMULATION_STACK_RELATIVE_INDIRECT_INDEXED_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STY (94)
Instruction<Ricoh5A22> n_94 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::STYIndex<Mode::Native>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::YLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::YHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_94 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::STYIndex<Mode::Emulation>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::YLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STA (95)
Instruction<Ricoh5A22> n_95 = {
	NATIVE_DIRECT_X_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_95 = {
	EMULATION_DIRECT_X_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STX (96)
Instruction<Ricoh5A22> n_96 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::STXIndex<Mode::Native>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::XLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::XHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_96 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::STXIndex<Mode::Emulation>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::XLow, WriteTo::Address>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STA (97)
Instruction<Ricoh5A22> n_97 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_LONG_D_Y_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_97 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_LONG_D_Y_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// TYA (98)
Instruction<Ricoh5A22> n_98 = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TYA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_98 = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TYA<Mode::Emulation>),
	NEXT_OPCODE
};


// STA (99)
Instruction<Ricoh5A22> n_99 = {
	NATIVE_ABSOLUTE_Y_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_99 = {
	EMULATION_ABSOLUTE_Y_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// TXS (9A)
Instruction<Ricoh5A22> n_9a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TXS<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_9a = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TXS<Mode::Emulation>),
	NEXT_OPCODE
};

// TXY (9B)
Instruction<Ricoh5A22> n_9b = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TXY<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_9b = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TXY<Mode::Emulation>),
	NEXT_OPCODE
};

// STZ (9C)
Instruction<Ricoh5A22> n_9c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::OZ>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOneDBCarry>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_9c = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::OZ>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STA (9D)
Instruction<Ricoh5A22> n_9d = {
	NATIVE_ABSOLUTE_X_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_9d = {
	EMULATION_ABSOLUTE_X_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// Thank you to Layle from the EmuDev Discord server for noticing a major typo in the implementation of opcode 9E.
// This fixed Earthbound, Chrono Trigger, Tales of Phantasia, and FFVI (and other games relying on this instruction)

// STZ (9E)
Instruction<Ricoh5A22> n_9e = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>),
	MakeHandler(Ricoh5A22Functions::AbsoluteXIndex<Mode::PCIncrement>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::OZ, false, BranchMode::None, true>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::PointerPlusOneBankCarry>, Ricoh5A22Predicates::MFlagSet),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_9e = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>),
	MakeHandler(Ricoh5A22Functions::AbsoluteXIndex<Mode::PCIncrement>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::OZ, false, BranchMode::None, true>),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>),
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// STA (9F)
Instruction<Ricoh5A22> n_9f = {
	NATIVE_ABSOLUTE_LONG_X_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_9f = {
	EMULATION_ABSOLUTE_LONG_X_WRITE
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};

// LDY (A0)
Instruction<Ricoh5A22> n_a0 = {
	NATIVE_IMMEDIATE_X
	MakeHandler(Ricoh5A22Functions::LDY<Mode::Native, Mode::PCIncrement>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_a0 = {
	EMULATION_IMMEDIATE_X
	MakeHandler(Ricoh5A22Functions::LDY<Mode::Emulation, Mode::PCIncrement>),
	NEXT_OPCODE
};

// LDA (A1)
Instruction<Ricoh5A22> n_a1 = {
	NATIVE_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_a1 = {
	EMULATION_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// LDX (A2)
Instruction<Ricoh5A22> n_a2 = {
	NATIVE_IMMEDIATE_X
	MakeHandler(Ricoh5A22Functions::LDX<Mode::Native, Mode::PCIncrement>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_a2 = {
	EMULATION_IMMEDIATE_X
	MakeHandler(Ricoh5A22Functions::LDX<Mode::Emulation, Mode::PCIncrement>),
	NEXT_OPCODE
};

// LDA (A3)
Instruction<Ricoh5A22> n_a3 = {
	NATIVE_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_a3 = {
	EMULATION_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// LDY (A4)
Instruction<Ricoh5A22> n_a4 = {
	NATIVE_DIRECT_READ_X
	MakeHandler(Ricoh5A22Functions::LDY<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_a4 = {
	EMULATION_DIRECT_READ_X
	MakeHandler(Ricoh5A22Functions::LDY<Mode::Emulation>),
	NEXT_OPCODE
};


// LDA (A5)
Instruction<Ricoh5A22> n_a5 = {
	NATIVE_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_a5 = {
	EMULATION_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// LDX (A6)
Instruction<Ricoh5A22> n_a6 = {
	NATIVE_DIRECT_READ_X
	MakeHandler(Ricoh5A22Functions::LDX<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_a6 = {
	EMULATION_DIRECT_READ_X
	MakeHandler(Ricoh5A22Functions::LDX<Mode::Emulation>),
	NEXT_OPCODE
};

// LDA (A7)
Instruction<Ricoh5A22> n_a7 = {
	NATIVE_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_a7 = {
	EMULATION_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};

// TAY (A8)
Instruction<Ricoh5A22> n_a8 = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TAY<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_a8 = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TAY<Mode::Emulation>),
	NEXT_OPCODE
};

// LDA (A9)
Instruction<Ricoh5A22> n_a9 = {
	NATIVE_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native, Mode::PCIncrement>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_a9 = {
	EMULATION_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation, Mode::PCIncrement>),
	NEXT_OPCODE
};

// TAX (AA)
Instruction<Ricoh5A22> n_aa = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TAX<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_aa = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TAX<Mode::Emulation>),
	NEXT_OPCODE
};

// PLB (AB)
Instruction<Ricoh5A22> n_ab = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementS),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::Bank>),
	MakeHandler(Ricoh5A22Functions::PLB<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ab = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementSNativeAndReadBank),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::PLB<Mode::Emulation>),
	NEXT_OPCODE
};

// LDY (AC)
Instruction<Ricoh5A22> n_ac = {
	NATIVE_ABSOLUTE_READ_X
	MakeHandler(Ricoh5A22Functions::LDY<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ac = {
	EMULATION_ABSOLUTE_READ_X
	MakeHandler(Ricoh5A22Functions::LDY<Mode::Emulation>),
	NEXT_OPCODE
};

// LDA (AD)
Instruction<Ricoh5A22> n_ad = {
	NATIVE_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ad = {
	EMULATION_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// LDX (AE)
Instruction<Ricoh5A22> n_ae = {
	NATIVE_ABSOLUTE_READ_X
	MakeHandler(Ricoh5A22Functions::LDX<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ae = {
	EMULATION_ABSOLUTE_READ_X
	MakeHandler(Ricoh5A22Functions::LDX<Mode::Emulation>),
	NEXT_OPCODE
};

// LDA (AF)
Instruction<Ricoh5A22> n_af = {
	NATIVE_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_af = {
	EMULATION_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// BCS (B0)
Instruction<Ricoh5A22> n_b0 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::C_One>),
	NATIVE_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_b0 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::C_One>),
	EMULATION_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};

// LDA (B1)
Instruction<Ricoh5A22> n_b1 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_b1 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};

// LDA (B2)
Instruction<Ricoh5A22> n_b2 = {
	NATIVE_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_b2 = {
	EMULATION_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// LDA (B3)
Instruction<Ricoh5A22> n_b3 = {
	NATIVE_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_b3 = {
	EMULATION_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// LDY (B4)
Instruction<Ricoh5A22> n_b4 = {
	NATIVE_DIRECT_X_READ_X
	MakeHandler(Ricoh5A22Functions::LDY<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_b4 = {
	EMULATION_DIRECT_X_READ_X
	MakeHandler(Ricoh5A22Functions::LDY<Mode::Emulation>),
	NEXT_OPCODE
};

// LDA (B5)
Instruction<Ricoh5A22> n_b5 = {
	NATIVE_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_b5 = {
	EMULATION_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// LDX (B6)
Instruction<Ricoh5A22> n_b6 = {
	NATIVE_DIRECT_Y_READ
	MakeHandler(Ricoh5A22Functions::LDX<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_b6 = {
	EMULATION_DIRECT_Y_READ
	MakeHandler(Ricoh5A22Functions::LDX<Mode::Emulation>),
	NEXT_OPCODE
};

// LDA (B7)
Instruction<Ricoh5A22> n_b7 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_b7 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// CLV (B8)
Instruction<Ricoh5A22> n_b8 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::CLV),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_b8 = n_b8;


// LDA (B9)
Instruction<Ricoh5A22> n_b9 = {
	NATIVE_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_b9 = {
	EMULATION_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// TSX (BA)
Instruction<Ricoh5A22> n_ba = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TSX<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ba = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TSX<Mode::Emulation>),
	NEXT_OPCODE
};

// TYX (BB)
Instruction<Ricoh5A22> n_bb = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TYX<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_bb = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::TYX<Mode::Emulation>),
	NEXT_OPCODE
};

// LDY (BC)
Instruction<Ricoh5A22> n_bc = {
	NATIVE_ABSOLUTE_X_READ_X
	MakeHandler(Ricoh5A22Functions::LDY<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_bc = {
	EMULATION_ABSOLUTE_X_READ_X
	MakeHandler(Ricoh5A22Functions::LDY<Mode::Emulation>),
	NEXT_OPCODE
};

// LDA (BD)
Instruction<Ricoh5A22> n_bd = {
	NATIVE_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_bd = {
	EMULATION_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// LDX (BE)
Instruction<Ricoh5A22> n_be = {
	NATIVE_ABSOLUTE_Y_READ_X
	MakeHandler(Ricoh5A22Functions::LDX<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_be = {
	EMULATION_ABSOLUTE_Y_READ_X
	MakeHandler(Ricoh5A22Functions::LDX<Mode::Emulation>),
	NEXT_OPCODE
};

// LDA (BF)
Instruction<Ricoh5A22> n_bf = {
	NATIVE_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_bf = {
	EMULATION_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::LDA<Mode::Emulation>),
	NEXT_OPCODE
};

// CPY (C0)
Instruction<Ricoh5A22> n_c0 = {
	NATIVE_IMMEDIATE_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Native, Mode::RegisterY, Mode::PCIncrement>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_c0 = {
	EMULATION_IMMEDIATE_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Emulation, Mode::RegisterY, Mode::PCIncrement>),
	NEXT_OPCODE
};

// CMP (C1)
Instruction<Ricoh5A22> n_c1 = {
	NATIVE_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_c1 = {
	EMULATION_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// REP (C2)
Instruction<Ricoh5A22> n_c2 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::REP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_c2 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::REP<Mode::Emulation>),
	NEXT_OPCODE
};

// CMP (C3)
Instruction<Ricoh5A22> n_c3 = {
	NATIVE_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_c3 = {
	EMULATION_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// CPY (C4)
Instruction<Ricoh5A22> n_c4 = {
	NATIVE_DIRECT_READ_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Native, Mode::RegisterY>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_c4 = {
	EMULATION_DIRECT_READ_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Emulation, Mode::RegisterY>),
	NEXT_OPCODE
};

// CMP (C5)
Instruction<Ricoh5A22> n_c5 = {
	NATIVE_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_c5 = {
	EMULATION_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// DEC (C6)
Instruction<Ricoh5A22> n_c6 = {
	NATIVE_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Decrease, Mode::Operand, Mode::MFlag>),
	NATIVE_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_c6 = {
	EMULATION_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Decrease, Mode::Operand, Mode::MFlag>),
	EMULATION_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// CMP (C7)
Instruction<Ricoh5A22> n_c7 = {
	NATIVE_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_c7 = {
	EMULATION_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// INY (C8)
Instruction<Ricoh5A22> n_c8 = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Increase, Mode::RegisterY, Mode::XFlag>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_c8 = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Increase, Mode::RegisterY, Mode::XFlag>),
	NEXT_OPCODE
};

// CMP (C9)
Instruction<Ricoh5A22> n_c9 = {
	NATIVE_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native, Mode::PCIncrement>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_c9 = {
	EMULATION_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation, Mode::PCIncrement>),
	NEXT_OPCODE
};

// DEX (CA)
Instruction<Ricoh5A22> n_ca = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Decrease, Mode::RegisterX, Mode::XFlag>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ca = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Decrease, Mode::RegisterX, Mode::XFlag>),
	NEXT_OPCODE
};

// WAI (CB)
Instruction<Ricoh5A22> n_cb = {
	MakeHandler(Ricoh5A22Functions::WaitPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22SpecialFunctions::WAIT)
};
Instruction<Ricoh5A22> e_cb = n_cb;

// CPY (CC)
Instruction<Ricoh5A22> n_cc = {
	NATIVE_ABSOLUTE_READ_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Native, Mode::RegisterY>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_cc = {
	EMULATION_ABSOLUTE_READ_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Emulation, Mode::RegisterY>),
	NEXT_OPCODE
};

// CMP (CD)
Instruction<Ricoh5A22> n_cd = {
	NATIVE_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_cd = {
	EMULATION_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// DEC (CE)
Instruction<Ricoh5A22> n_ce = {
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Decrease, Mode::Operand, Mode::MFlag>),
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ce = {
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Decrease, Mode::Operand, Mode::MFlag>),
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// CMP (CF)
Instruction<Ricoh5A22> n_cf = {
	NATIVE_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_cf = {
	EMULATION_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// BNE (D0)
Instruction<Ricoh5A22> n_d0 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::Z_Zero>),
	NATIVE_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_d0 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::Z_Zero>),
	EMULATION_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};

// CMP (D1)
Instruction<Ricoh5A22> n_d1 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_d1 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// CMP (D2)
Instruction<Ricoh5A22> n_d2 = {
	NATIVE_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_d2 = {
	EMULATION_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// CMP (D3)
Instruction<Ricoh5A22> n_d3 = {
	NATIVE_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_d3 = {
	EMULATION_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// PEI (D4)
Instruction<Ricoh5A22> n_d4 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::DecrementS2),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_d4 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero),
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::DecrementS2Low),
	NEXT_OPCODE	
};

// CMP (D5)
Instruction<Ricoh5A22> n_d5 = {
	NATIVE_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_d5 = {
	EMULATION_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// DEC (D6)
Instruction<Ricoh5A22> n_d6 = {
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Decrease, Mode::Operand, Mode::MFlag>),
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_d6 = {
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Decrease, Mode::Operand, Mode::MFlag>),
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// CMP (D7)
Instruction<Ricoh5A22> n_d7 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_d7 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// CLD (D8)
Instruction<Ricoh5A22> n_d8 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::CLD),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_d8 = n_d8;


// CMP (D9)
Instruction<Ricoh5A22> n_d9 = {
	NATIVE_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_d9 = {
	EMULATION_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// PHX (DA)
Instruction<Ricoh5A22> n_da = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::XHigh, WriteTo::Stack0>, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::DecrementS),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::XLow, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::DecrementS),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_da = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::XLow, WriteTo::Stack0Emulation>),
	MakeHandler(Ricoh5A22Functions::DecrementSLow),
	NEXT_OPCODE
};

// STP (DB)
Instruction<Ricoh5A22> n_db = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22SpecialFunctions::STOP),
};
Instruction<Ricoh5A22> e_db = n_db;

// JML (DC)
Instruction<Ricoh5A22> n_dc = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOne, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::JMLDCRead),
	MakeHandler(Ricoh5A22Functions::PCOperandPBBank),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_dc = n_dc;

// CMP (DD)
Instruction<Ricoh5A22> n_dd = {
	NATIVE_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_dd = {
	EMULATION_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// DEC (DE)
Instruction<Ricoh5A22> n_de = {
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Decrease, Mode::Operand, Mode::MFlag>),
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_de = {
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Decrease, Mode::Operand, Mode::MFlag>),
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// CMP (DF)
Instruction<Ricoh5A22> n_df = {
	NATIVE_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_df = {
	EMULATION_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::CMP<Mode::Emulation>),
	NEXT_OPCODE
};

// CPX (E0)
Instruction<Ricoh5A22> n_e0 = {
	NATIVE_IMMEDIATE_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Native, Mode::RegisterX, Mode::PCIncrement>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_e0 = {
	EMULATION_IMMEDIATE_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Emulation, Mode::RegisterX, Mode::PCIncrement>),
	NEXT_OPCODE
};

// SBC (E1)
Instruction<Ricoh5A22> n_e1 = {
	NATIVE_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_e1 = {
	EMULATION_DIRECT_INDEXED_INDIRECT_D_X_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// SEP (E2)
Instruction<Ricoh5A22> n_e2 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::SEP<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_e2 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::SEP<Mode::Emulation>),
	NEXT_OPCODE
};

// SBC (E3)
Instruction<Ricoh5A22> n_e3 = {
	NATIVE_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_e3 = {
	EMULATION_STACK_RELATIVE_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// CPX (E4)
Instruction<Ricoh5A22> n_e4 = {
	NATIVE_DIRECT_READ_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Native, Mode::RegisterX>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_e4 = {
	EMULATION_DIRECT_READ_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Emulation, Mode::RegisterX>),
	NEXT_OPCODE
};

// SBC (E5)
Instruction<Ricoh5A22> n_e5 = {
	NATIVE_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_e5 = {
	EMULATION_DIRECT_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// INC (E6)
Instruction<Ricoh5A22> n_e6 = {
	NATIVE_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Increase, Mode::Operand, Mode::MFlag>),
	NATIVE_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_e6 = {
	EMULATION_DIRECT_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Increase, Mode::Operand, Mode::MFlag>),
	EMULATION_DIRECT_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// SBC (E7)
Instruction<Ricoh5A22> n_e7 = {
	NATIVE_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_e7 = {
	EMULATION_DIRECT_INDIRECT_LONG_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// INX (E8)
Instruction<Ricoh5A22> n_e8 = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Increase, Mode::RegisterX, Mode::XFlag>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_e8 = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Increase, Mode::RegisterX, Mode::XFlag>),
	NEXT_OPCODE
};

// SBC (E9)
Instruction<Ricoh5A22> n_e9 = {
	NATIVE_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native, Mode::PCIncrement>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_e9 = {
	EMULATION_IMMEDIATE_M
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation, Mode::PCIncrement>),
	NEXT_OPCODE
};

// NOP (EA)
Instruction<Ricoh5A22> n_ea = {
	IMPLIED
	MakeHandler(Ricoh5A22Functions::NOP),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ea = n_ea;

// XBA (EB)
Instruction<Ricoh5A22> n_eb = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::XBA),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_eb = n_eb;

// CPX (EC)
Instruction<Ricoh5A22> n_ec = {
	NATIVE_ABSOLUTE_READ_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Native, Mode::RegisterX>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ec = {
	EMULATION_ABSOLUTE_READ_X
	MakeHandler(Ricoh5A22Functions::CopyRegister<Mode::Emulation, Mode::RegisterX>),
	NEXT_OPCODE
};

// SBC (ED)
Instruction<Ricoh5A22> n_ed = {
	NATIVE_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ed = {
	EMULATION_ABSOLUTE_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// INC (EE)
Instruction<Ricoh5A22> n_ee = {
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Increase, Mode::Operand, Mode::MFlag>),
	NATIVE_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ee = {
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Increase, Mode::Operand, Mode::MFlag>),
	EMULATION_ABSOLUTE_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// SBC (EF)
Instruction<Ricoh5A22> n_ef = {
	NATIVE_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ef = {
	EMULATION_ABSOLUTE_LONG_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// BEQ (F0)
Instruction<Ricoh5A22> n_f0 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::Z_One>),
	NATIVE_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_f0 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::None, false, BranchMode::Z_One>),
	EMULATION_FLAG_BRANCH_LOGIC
	NEXT_OPCODE
};

// SBC (F1)
Instruction<Ricoh5A22> n_f1 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_f1 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_D_Y_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// SBC (F2)
Instruction<Ricoh5A22> n_f2 = {
	NATIVE_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_f2 = {
	EMULATION_DIRECT_INDIRECT_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// SBC (F3)
Instruction<Ricoh5A22> n_f3 = {
	NATIVE_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_f3 = {
	EMULATION_STACK_RELATIVE_INDIRECT_INDEXED_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// PEA (F4)
Instruction<Ricoh5A22> n_f4 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::DecrementS2),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_f4 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::OperandHigh>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::DecrementS2Low),
	NEXT_OPCODE
};

// SBC (F5)
Instruction<Ricoh5A22> n_f5 = {
	NATIVE_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_f5 = {
	EMULATION_DIRECT_X_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// INC (F6)
Instruction<Ricoh5A22> n_f6 = {
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Increase, Mode::Operand, Mode::MFlag>),
	NATIVE_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_f6 = {
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Increase, Mode::Operand, Mode::MFlag>),
	EMULATION_DIRECT_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// SBC (F7)
Instruction<Ricoh5A22> n_f7 = {
	NATIVE_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_f7 = {
	EMULATION_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// SED (F8)
Instruction<Ricoh5A22> n_f8 = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::SED),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_f8 = n_f8;


// SBC (F9)
Instruction<Ricoh5A22> n_f9 = {
	NATIVE_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_f9 = {
	EMULATION_ABSOLUTE_Y_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// PLX (FA)
Instruction<Ricoh5A22> n_fa = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementS),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack1, ReadTo::OperandHigh>, Ricoh5A22Predicates::XFlagSet),
	MakeHandler(Ricoh5A22Functions::PL<Mode::Native, Mode::RegisterX, Mode::XFlag>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_fa = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::IncrementSLow),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Stack0, ReadTo::OperandLow>),
	MakeHandler(Ricoh5A22Functions::PL<Mode::Emulation, Mode::RegisterX, Mode::XFlag>),
	NEXT_OPCODE
};

// XCE (FB)
Instruction<Ricoh5A22> n_fb = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),
	MakeHandler(Ricoh5A22Functions::XCE),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_fb = n_fb;

// JSR (FC)
Instruction<Ricoh5A22> n_fc = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(Ricoh5A22Functions::DecrementS2),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>),
	MakeHandler(Ricoh5A22Functions::JSRIndex),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::AddressLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankNoCarry, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::PCAddress),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_fc = {
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>),
	MakeHandler(Ricoh5A22Functions::IncrementPC),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCHigh, WriteTo::Stack0Emulation>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1Emulation>),
	MakeHandler(Ricoh5A22Functions::DecrementS2Low),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>),
	MakeHandler(Ricoh5A22Functions::JSRIndex),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::AddressLow>),
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankNoCarry, ReadTo::AddressHigh>),
	MakeHandler(Ricoh5A22Functions::PCAddress),
	NEXT_OPCODE
};

// SBC (FD)
Instruction<Ricoh5A22> n_fd = {
	NATIVE_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_fd = {
	EMULATION_ABSOLUTE_X_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

// INC (FE)
Instruction<Ricoh5A22> n_fe = {
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Native, Mode::Increase, Mode::Operand, Mode::MFlag>),
	NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_fe = {
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_START
	MakeHandler(Ricoh5A22Functions::INDE<Mode::Emulation, Mode::Increase, Mode::Operand, Mode::MFlag>),
	EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_END
	NEXT_OPCODE
};

// SBC (FF)
Instruction<Ricoh5A22> n_ff = {
	NATIVE_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Native>),
	NEXT_OPCODE
};
Instruction<Ricoh5A22> e_ff = {
	EMULATION_ABSOLUTE_LONG_X_READ
	MakeHandler(Ricoh5A22Functions::SBC<Mode::Emulation>),
	NEXT_OPCODE
};

Instruction<Ricoh5A22> nop = {
	MakeHandler(Ricoh5A22Functions::NOP),
	MakeHandler(Ricoh5A22Functions::NOP),
};