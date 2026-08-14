#include "spc_700_operations.hpp"
#include "spc_700_addressing_modes.hpp"
#include "spc_700.hpp"

// TO-DO WHEN COMPLETED, ORDER ALL Instruction<SPC700>S BY OPCODE

namespace SPC700Predicates {
	static bool NoJump(SPC700& cpu) {
		return !cpu.BufferJump;
	}
}

namespace SPC700SpecialFunctions {
	static void Sleep(SPC700& cpu, bool skipped) {
		return;
	}

	static void Stop(SPC700& cpu, bool skipped) {
		return;
	}
}

namespace SPC700Functions {
	static Word ya(SPC700& cpu) {
		return ((uint8_t)(cpu.regs.Y) << 8) | get_lo(cpu.regs.A);
	}

	static void SetNZ(SPC700& cpu, bool skipped, Word value) {
		if (value & 0x80) {
			cpu.set_flag_N();
		} else {
			cpu.clear_flag_N();
		}
		if (value == 0) {
			cpu.set_flag_Z();
		} else {
			cpu.clear_flag_Z();
		}
	}

	static void SetFuncOperand(SPC700& cpu, bool skipped) {
		cpu.BufferAddress = (cpu.BufferOperand | ((cpu.regs.P & 0x20) << 3));
	}

	static void SetFuncOperandPlusX(SPC700& cpu, bool skipped) {
		cpu.BufferAddress = (((cpu.BufferOperand + cpu.regs.X) & 0xFF) | ((cpu.regs.P & 0x20) << 3));
	}

	static void SetFuncOperandPlusY(SPC700& cpu, bool skipped) {
		cpu.BufferAddress = (((cpu.BufferOperand + cpu.regs.Y) & 0xFF) | ((cpu.regs.P & 0x20) << 3));
	}

	static void SetFuncX(SPC700& cpu, bool skipped) {
		cpu.BufferAddress = ((cpu.regs.X & 0xFF) | ((cpu.regs.P & 0x20) << 3));
	}

	static void sub_func(SPC700& cpu, bool skipped, SubFunc func) {
		switch(func) {
		case SubFunc::ClearISetX:
			cpu.clear_flag_I();
			cpu.set_flag_X();
			break;
		case SubFunc::SetNZFlagRegisterA:
			SPC700Functions::SetNZ(cpu, skipped, (uint8_t)cpu.regs.A);
			break;
		case SubFunc::SetNZFlagRegisterYA:
			SPC700Functions::SetNZ(cpu, skipped, (uint8_t)cpu.regs.Y);
			break;
		case SubFunc::SetNZFlagRegisterX:
			SPC700Functions::SetNZ(cpu, skipped, (uint8_t)cpu.regs.X);
			break;
		case SubFunc::SetNZFlagRegisterY:
			SPC700Functions::SetNZ(cpu, skipped, (uint8_t)cpu.regs.Y);
			break;
		case SubFunc::SetNZFlagOperand:
			SPC700Functions::SetNZ(cpu, skipped, (uint8_t)cpu.BufferOperand);
			break;
		case SubFunc::SetNZFlagOperand0:
			SPC700Functions::SetNZ(cpu, skipped, (uint8_t)cpu.BufferOperand0);
			break;
		case SubFunc::SetSubFunc:
			SPC700Functions::SetFuncOperand(cpu, skipped);
			break;
		case SubFunc::IncrementAddressByX:
			cpu.BufferAddress += (uint8_t)cpu.regs.X;
			break;
		case SubFunc::IncrementAddressByY:
			cpu.BufferAddress += (uint8_t)cpu.regs.Y;
			break;
		}
	}

	static void NOP(SPC700& cpu, bool skipped) {
		return;
	}

	template <SubFunc func = SubFunc::None>
	static void IncrementPC(SPC700& cpu, bool skipped) {
		cpu.regs.PC++;
		sub_func(cpu, skipped, func);
	}

	static void Next(SPC700& cpu, bool skipped) {
		cpu.BufferOpCode = cpu.read(cpu.regs.PC);
	}

	static void CLRC(SPC700& cpu, bool skipped) { cpu.regs.P = (cpu.regs.P & ~0x01); }
	static void SETC(SPC700& cpu, bool skipped) { cpu.regs.P = (cpu.regs.P |  0x01); }
	static void CLRP(SPC700& cpu, bool skipped) { cpu.regs.P = (cpu.regs.P & ~0x20); }
	static void SETP(SPC700& cpu, bool skipped) { cpu.regs.P = (cpu.regs.P |  0x20); }
	static void DI(SPC700& cpu, bool skipped)   { cpu.regs.P = (cpu.regs.P & ~0x04); }
	static void EI(SPC700& cpu, bool skipped)   { cpu.regs.P = (cpu.regs.P |  0x04); }

	static void CLRV(SPC700& cpu, bool skipped) {
		cpu.clear_flag_V();
		cpu.clear_flag_H();
	}

	static void NOTC(SPC700& cpu, bool skipped) {
		if (cpu.get_flag_C() ^ 1) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
	}

	template <typename From, typename To>
	static void Read(SPC700& cpu, bool skipped) {
		Word address;

		// read from...
		if constexpr (std::is_same_v<From, ReadFrom::PC>)     { address = cpu.regs.PC; }
		
		if constexpr (std::is_same_v<From, ReadFrom::StackMinus2>) { address = 0x0100 | (uint8_t)(cpu.regs.S - 2); }
		if constexpr (std::is_same_v<From, ReadFrom::StackMinus1>) { address = 0x0100 | (uint8_t)(cpu.regs.S - 1); }
		if constexpr (std::is_same_v<From, ReadFrom::Stack0>)      { address = 0x0100 | (uint8_t)(cpu.regs.S + 0); }
		if constexpr (std::is_same_v<From, ReadFrom::Stack1>)      { address = 0x0100 | (uint8_t)(cpu.regs.S + 1); }
		if constexpr (std::is_same_v<From, ReadFrom::Stack2>)      { address = 0x0100 | (uint8_t)(cpu.regs.S + 2); }

		if constexpr (std::is_same_v<From, ReadFrom::Pointer>)        { address = cpu.BufferPointer; }
		if constexpr (std::is_same_v<From, ReadFrom::PointerPlusOne>) { address = cpu.BufferPointer + 1; }

		if constexpr (std::is_same_v<From, ReadFrom::FFDE>) { address = 0xFFDE; }
		if constexpr (std::is_same_v<From, ReadFrom::FFDF>) { address = 0xFFDF; }

		if constexpr (std::is_same_v<From, ReadFrom::Address>)     { address = cpu.BufferAddress; }
		if constexpr (std::is_same_v<From, ReadFrom::Address1FFF>) { address = cpu.BufferAddress & 0x1FFF; }

		if constexpr (std::is_same_v<From, ReadFrom::XPSW>) { address = ((cpu.regs.X & 0xFF) | ((cpu.regs.P & 0x20) << 3)); }
		if constexpr (std::is_same_v<From, ReadFrom::YPSW>) { address = ((cpu.regs.Y & 0xFF) | ((cpu.regs.P & 0x20) << 3)); }
		if constexpr (std::is_same_v<From, ReadFrom::AddressPlusOnePSW>) { address = ((cpu.BufferAddress + 1) & 0xFF) | ((cpu.regs.P & 0x20) << 3); }

		Byte value = cpu.read(address);
		// read to...
		if constexpr (std::is_same_v<To, ReadTo::Discard>) { return; }
		if constexpr (std::is_same_v<To, ReadTo::P>)       { cpu.regs.P  = value; }
		if constexpr (std::is_same_v<To, ReadTo::A>)       { cpu.regs.A  = value; }
		if constexpr (std::is_same_v<To, ReadTo::X>)       { cpu.regs.X  = value; }
		if constexpr (std::is_same_v<To, ReadTo::Y>)       { cpu.regs.Y  = value; }
		if constexpr (std::is_same_v<To, ReadTo::PC>)      { cpu.regs.PC = value; }
		if constexpr (std::is_same_v<To, ReadTo::PCLow>)   { cpu.regs.PC = (get_hi(cpu.regs.PC) << 8) | (uint8_t)(value); }
		if constexpr (std::is_same_v<To, ReadTo::PCHigh>)  { cpu.regs.PC = ((uint8_t)value << 8) | get_lo(cpu.regs.PC); }
		
		if constexpr (std::is_same_v<To, ReadTo::AddressLow>)     { cpu.BufferAddress   = (get_hi(cpu.BufferAddress) << 8) | (uint8_t)(value); }
		if constexpr (std::is_same_v<To, ReadTo::AddressHigh>)    { cpu.BufferAddress   = ((uint8_t)(value) << 8) | get_lo(cpu.BufferAddress); }
		if constexpr (std::is_same_v<To, ReadTo::Operand16Low>)   { cpu.BufferOperand16 = (get_hi(cpu.BufferOperand16) << 8) | (uint8_t)(value); }
		if constexpr (std::is_same_v<To, ReadTo::Operand16High>)  { cpu.BufferOperand16 = ((uint8_t)(value) << 8) | get_lo(cpu.BufferOperand16); }
		if constexpr (std::is_same_v<To, ReadTo::PointerLow>)     { cpu.BufferPointer   = (get_hi(cpu.BufferPointer) << 8) | (uint8_t)(value); }
		if constexpr (std::is_same_v<To, ReadTo::PointerHigh>)    { cpu.BufferPointer   = ((uint8_t)(value) << 8) | get_lo(cpu.BufferPointer); }

		if constexpr (std::is_same_v<To, ReadTo::Operand>)  { cpu.BufferOperand  = value; }
		if constexpr (std::is_same_v<To, ReadTo::Operand0>) { cpu.BufferOperand0 = value; }
		if constexpr (std::is_same_v<To, ReadTo::Operand1>) { cpu.BufferOperand1 = value; }
	}

	template <typename Value, typename To>
	static void Write(SPC700& cpu, bool skipped) {
		// write the value...
		Byte value;
		if constexpr (std::is_same_v<Value, WriteValue::P>)      { value = cpu.regs.P; }
		if constexpr (std::is_same_v<Value, WriteValue::A>)      { value = cpu.regs.A; }
		if constexpr (std::is_same_v<Value, WriteValue::X>)      { value = cpu.regs.X; }
		if constexpr (std::is_same_v<Value, WriteValue::Y>)      { value = cpu.regs.Y; }
		if constexpr (std::is_same_v<Value, WriteValue::PC>)     { value = cpu.regs.PC; }
		if constexpr (std::is_same_v<Value, WriteValue::PCHigh>) { value = get_hi(cpu.regs.PC); }
		if constexpr (std::is_same_v<Value, WriteValue::PCLow>)  { value = get_lo(cpu.regs.PC); }

		if constexpr (std::is_same_v<Value, WriteValue::Operand>)   { value = cpu.BufferOperand; }
		if constexpr (std::is_same_v<Value, WriteValue::Operand0>)  { value = cpu.BufferOperand0; }
		if constexpr (std::is_same_v<Value, WriteValue::Operand1>)  { value = cpu.BufferOperand1; }

		Word address;
		if constexpr (std::is_same_v<To, WriteTo::StackMinus2>) { address = 0x0100 | (uint8_t)(cpu.regs.S - 2); }
		if constexpr (std::is_same_v<To, WriteTo::StackMinus1>) { address = 0x0100 | (uint8_t)(cpu.regs.S - 1); }
		if constexpr (std::is_same_v<To, WriteTo::Stack0>)      { address = 0x0100 | (uint8_t)(cpu.regs.S + 0); }
		if constexpr (std::is_same_v<To, WriteTo::Stack1>)      { address = 0x0100 | (uint8_t)(cpu.regs.S + 1); }
		if constexpr (std::is_same_v<To, WriteTo::Stack2>)      { address = 0x0100 | (uint8_t)(cpu.regs.S + 2); }
		if constexpr (std::is_same_v<To, WriteTo::Address>)     { address = cpu.BufferAddress; }
		if constexpr (std::is_same_v<To, WriteTo::Address1FFF>) { address = cpu.BufferAddress & 0x1FFF; }
		if constexpr (std::is_same_v<To, WriteTo::XPSW>)        { address = ((cpu.regs.X & 0xFF) | ((cpu.regs.P & 0x20) << 3)); }
		if constexpr (std::is_same_v<To, WriteTo::Pointer>)     { address = cpu.BufferPointer; }
		
		if constexpr (std::is_same_v<To, WriteTo::AddressPlusOnePSW>) { address = ((cpu.BufferAddress + 1) & 0xFF) | ((cpu.regs.P & 0x20) << 3); }

		// likely to expand to more stack + ...

		cpu.write(address, value);
	}

	template <int value = 1, SubFunc func = SubFunc::None>
	static void DecrementS(SPC700& cpu, bool skipped) {
		cpu.regs.S = (uint8_t)(cpu.regs.S - value);
		SPC700Functions::sub_func(cpu, skipped, func);
	}

	template <int value = 1, SubFunc func = SubFunc::None>
	static void IncrementS(SPC700& cpu, bool skipped) {
		cpu.regs.S = (uint8_t)(cpu.regs.S + value);
		SPC700Functions::sub_func(cpu, skipped, func);
	}

	template <int value = 1, SubFunc func = SubFunc::None>
	static void DecrementX(SPC700& cpu, bool skipped) {
		cpu.regs.X = (uint8_t)(cpu.regs.X - value);
		SPC700Functions::sub_func(cpu, skipped, func);
	}

	template <int value = 1, SubFunc func = SubFunc::None>
	static void IncrementX(SPC700& cpu, bool skipped) {
		cpu.regs.X = (uint8_t)(cpu.regs.X + value);
		SPC700Functions::sub_func(cpu, skipped, func);
	}

	template<int call = 0>
	static void TCallLow(SPC700& cpu, bool skipped) {
		Byte value = cpu.read(0xFFDE - (2 * call));
		cpu.regs.PC = (get_hi(cpu.regs.PC) << 8) | value;
	}

	template<int call = 0>
	static void TCallHigh(SPC700& cpu, bool skipped) {
		Byte value = cpu.read(0xFFDF - (2 * call));
		cpu.regs.PC = (value << 8) | get_lo(cpu.regs.PC);
	}

	template <int step = 1>
	static void DAA(SPC700& cpu, bool skipped) {
		switch(step) {
		case 1:
			if (cpu.get_flag_C() || (cpu.regs.A & 0xFF) > 0x99) {
				cpu.regs.A = (uint8_t)(cpu.regs.A + 0x60);
				cpu.set_flag_C();
			}
			break;
		case 2:
			if (cpu.get_flag_H() || (cpu.regs.A & 0x0F) > 0x09) {
				cpu.regs.A = (uint8_t)(cpu.regs.A + 0x06);
			}
			if (cpu.regs.A & 0x80) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (cpu.regs.A == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
			break;
		}
	}

	template <int step = 1>
	static void DAS(SPC700& cpu, bool skipped) {
		switch (step) {
		case 1:
			if (!cpu.get_flag_C() || (cpu.regs.A & 0xFF) > 0x99) {
				cpu.regs.A = (uint8_t)(cpu.regs.A - 0x60);
				cpu.clear_flag_C();
			}
			break;
		case 2:
			if (!cpu.get_flag_H() || (cpu.regs.A & 0x0F) > 0x09) {
				cpu.regs.A = (uint8_t)(cpu.regs.A - 0x06);
			}
			if (cpu.regs.A & 0x80) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (cpu.regs.A == 0) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
			break;
		}
	}

	template <SubFunc func = SubFunc::None>
	static void XCN(SPC700& cpu, bool skipped) {
		Byte value = cpu.regs.A;
		value = (value >> 7) | (value << 1);
		cpu.regs.A = (uint8_t)(value);
		SPC700Functions::sub_func(cpu, skipped, func);
	}

	template<int step = 1, SubFunc func = SubFunc::None>
	static void DIV(SPC700& cpu, bool skipped) {
		switch(step) {
		case 1:
		    if ((cpu.regs.A & 0x0F) >= (cpu.regs.X & 0x0F)) {
		        cpu.set_flag_H();
		    } else {
		        cpu.clear_flag_H();
		    }

		    if (cpu.regs.Y >= cpu.regs.X) {
		        cpu.set_flag_V();
		    } else {
		        cpu.clear_flag_V();
		    }

		    cpu.DivYa = static_cast<uint32_t>(ya(cpu));
		    cpu.ShiftedX = static_cast<uint32_t>(cpu.regs.X) << 9;
		    break;
		case 2:
			cpu.DivYa = cpu.DivYa << 1;
			if (cpu.DivYa & 0x20000) {
				cpu.DivYa = cpu.DivYa ^ 0x20001;
			}
			if (cpu.DivYa >= cpu.ShiftedX) {
				cpu.DivYa = cpu.DivYa ^ 1;
			}
			if (cpu.DivYa & 1) {
				cpu.DivYa = (cpu.DivYa - cpu.ShiftedX) & 0x1FFFF;
			}
			break;
		case 3:
			cpu.regs.Y = (uint8_t)(cpu.DivYa >> 9);
			cpu.regs.A = (uint8_t)(cpu.DivYa);
			break;
		}
		SPC700Functions::sub_func(cpu, skipped, func);
	}

	// write the value...

	template<int step = 1, SubFunc func = SubFunc::None>
	static void MUL(SPC700& cpu, bool skipped) {
		switch(step) {
		case 1:
			cpu.YABuffer = (cpu.regs.Y & 0xFF) * (cpu.regs.A & 0xFF);
			break;
		case 2:
			cpu.regs.A = (uint8_t)get_lo(cpu.YABuffer);
			cpu.regs.Y = (uint8_t)get_hi(cpu.YABuffer);
			break;
		}
		SPC700Functions::sub_func(cpu, skipped, func);
	}

	template<int shift = 0>
	static void SET(SPC700& cpu, bool skipped) {
		Byte mask = (1 << shift);
		cpu.BufferOperand = ((cpu.BufferOperand & ~mask) | mask);
	}

	template<int shift = 0>
	static void CLR(SPC700& cpu, bool skipped) {
		Byte mask = (1 << shift);
		cpu.BufferOperand = ((cpu.BufferOperand & ~mask) | 0x00);
	}

	template<int shift = 0>
	static void BBS(SPC700& cpu, bool skipped) {
		cpu.BufferJump = ((cpu.BufferOperand & (1 << shift)) == (1 << shift));
	}

	template<int shift = 0>
	static void BBC(SPC700& cpu, bool skipped) {
		cpu.BufferJump = ((cpu.BufferOperand & (1 << shift)) == (0 << shift));
	}

	template <int step = 0>
	static void Branch(SPC700& cpu, bool skipped) {
		switch(step) {
		case 1:
			cpu.BufferAddress = cpu.regs.PC + (int8_t)(cpu.BufferOperand);
			cpu.regs.PC = (get_hi(cpu.regs.PC) << 8) | get_lo(cpu.BufferAddress);
			break;
		case 2:
			cpu.regs.PC = (get_hi(cpu.BufferAddress) << 8) | get_lo(cpu.regs.PC);
			break;
		}
	}

	template <int step = 0>
	static void MOV_5D(SPC700& cpu, bool skipped) {
		cpu.regs.X = (uint8_t)cpu.regs.A;
	}

	template <int step = 0>
	static void MOV_7D(SPC700& cpu, bool skipped) {
		cpu.regs.A = (uint8_t)cpu.regs.X;
	}

	template <int step = 0>
	static void MOV_Operand_To_Y(SPC700& cpu, bool skipped) {
		cpu.regs.Y = (uint8_t)cpu.BufferOperand;
	}

	template <int step = 0>
	static void MOV_8F(SPC700& cpu, bool skipped) {
		cpu.BufferOperand = (uint8_t)cpu.regs.A;
	}

	template <int step = 0>
	static void MOV_9D(SPC700& cpu, bool skipped) {
		cpu.regs.X = (uint8_t)cpu.regs.S;
	}

	template <int step = 0>
	static void MOV_BD(SPC700& cpu, bool skipped) {
		cpu.regs.S = (uint8_t)cpu.regs.X;
	}

	template <int step = 0>
	static void MOV_DD(SPC700& cpu, bool skipped) {
		cpu.regs.A = (uint8_t)cpu.regs.Y;
	}

	template <int step = 0>
	static void MOV_FD(SPC700& cpu, bool skipped) {
		cpu.regs.Y = (uint8_t)cpu.regs.A;
	}

	template <int step = 0>
	static void MOV_Operand_To_A(SPC700& cpu, bool skipped) {
		cpu.regs.A = (uint8_t)cpu.BufferOperand;
	}

	template <int step = 0>
	static void MOV_Operand_To_X(SPC700& cpu, bool skipped) {
		cpu.regs.X = (uint8_t)cpu.BufferOperand;
	}

	template <int code = 0, int step = 0, SubFunc func = SubFunc::None, bool pc_increment = false>
	static void MOV(SPC700& cpu, bool skipped) {
		if (pc_increment) { cpu.regs.PC++; }
		switch (code) {
		case 0x5D: SPC700Functions::MOV_5D<step>(cpu, skipped); break;
		case 0x7D: SPC700Functions::MOV_7D<step>(cpu, skipped); break;
		case 0x8F: SPC700Functions::MOV_8F<step>(cpu, skipped); break;
		case 0x9D: SPC700Functions::MOV_9D<step>(cpu, skipped); break;
		case 0xBD: SPC700Functions::MOV_BD<step>(cpu, skipped); break;
		
		case 0xDD: SPC700Functions::MOV_DD<step>(cpu, skipped); break;
		case 0xFD: SPC700Functions::MOV_FD<step>(cpu, skipped); break;

		case 0xE4:
		case 0xE5: 
		case 0xE6:
		case 0xE7:
		case 0xE8:
		case 0xF4:
		case 0xF6:
		case 0xF7:
		case 0xBF: SPC700Functions::MOV_Operand_To_A<step>(cpu, skipped); break;
		
		case 0xF8:
		case 0xF9: 
		case 0xCD:
		case 0xE9: SPC700Functions::MOV_Operand_To_X<step>(cpu, skipped); break;

		case 0x8D:
		case 0xEB:
		case 0xEC:
		case 0xFB: SPC700Functions::MOV_Operand_To_Y<step>(cpu, skipped); break;
		
		}

		SPC700Functions::sub_func(cpu, skipped, func);
	}

	template <Bitwise bitwise = Bitwise::OR, typename ApplyTo, typename With, SubFunc func = SubFunc::None, bool increment_pc = false>
	static void BITWISE(SPC700& cpu, bool skipped) {
		Word val = 0x00;
		if constexpr (increment_pc) {
			cpu.regs.PC++;
		}

		if constexpr (std::is_same_v<ApplyTo, Value::A>) { val = cpu.regs.A & 0xFF; }
		if constexpr (std::is_same_v<ApplyTo, Value::Operand0>) { val = cpu.BufferOperand0 & 0xFF; }
		if constexpr (std::is_same_v<ApplyTo, Value::Operand>) { val = cpu.BufferOperand & 0xFF; }
		if constexpr (std::is_same_v<ApplyTo, Value::X>) { val = cpu.regs.X & 0xFF; }
		if constexpr (std::is_same_v<ApplyTo, Value::Y>) { val = cpu.regs.Y & 0xFF; }

		Byte with;
		if constexpr (std::is_same_v<With, Value::Operand>)  { with = cpu.BufferOperand & 0xFF; }
		if constexpr (std::is_same_v<With, Value::Operand1>) { with = cpu.BufferOperand1 & 0xFF; }
		if constexpr (std::is_same_v<With, Value::X>) { with = cpu.regs.X & 0xFF; }
		if constexpr (std::is_same_v<With, Value::Y>) { with = cpu.regs.Y & 0xFF; }
		
		switch(bitwise) {
		case Bitwise::OR:  val = val | with; break;
		case Bitwise::AND: val = val & with; break;
		case Bitwise::EOR: val = val ^ with; break;
		case Bitwise::ADC: break;
		case Bitwise::SBC: with = ~with; break;
		case Bitwise::CMP:
			int tmp = val - with;
			if (tmp >= 0) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}
			if ((tmp & 0xFF) & 0x80) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (!tmp) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
			break;
		
		}

		if (bitwise == Bitwise::ADC || bitwise == Bitwise::SBC) {
			cpu.BufferTmp = (uint32_t)(val) + (uint32_t)(with) + (uint32_t)(cpu.get_flag_C());
			if (cpu.BufferTmp > 0xFF) {
				cpu.set_flag_C();
			} else {
				cpu.clear_flag_C();
			}
			if ((uint8_t)(cpu.BufferTmp) == 0x00) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
			if ((val ^ with ^ (uint8_t)(cpu.BufferTmp)) & 0x10) {
				cpu.set_flag_H();
			} else {
				cpu.clear_flag_H();
			}
			if (~(val ^ with) & (val ^ (uint8_t)(cpu.BufferTmp)) & 0x80) {
				cpu.set_flag_V();
			} else {
				cpu.clear_flag_V();
			}
			if ((uint8_t)cpu.BufferTmp & 0x80) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			val = (uint8_t)cpu.BufferTmp;
			
		}

		if constexpr (std::is_same_v<ApplyTo, Value::A>) { cpu.regs.A = val & 0xFF; }
		if constexpr (std::is_same_v<ApplyTo, Value::Operand0>) { cpu.BufferOperand0 = val & 0xFF; }
		if constexpr (std::is_same_v<ApplyTo, Value::Operand>) { cpu.BufferOperand = val & 0xFF; }
		if constexpr (std::is_same_v<ApplyTo, Value::X>) { cpu.regs.X = val & 0xFF; }
		if constexpr (std::is_same_v<ApplyTo, Value::Y>) { cpu.regs.Y = val & 0xFF; }

		SPC700Functions::sub_func(cpu, skipped, func);
	}

	static void SetPointerXPlusAddress(SPC700& cpu, bool skipped) {
		cpu.BufferPointer = (cpu.regs.X & 0xFF) + cpu.BufferAddress;
	}

	static void SetPointerYPlusAddress(SPC700& cpu, bool skipped) {
		cpu.BufferPointer = (cpu.regs.Y & 0xFF) + cpu.BufferAddress;
	}

	template <int AndVal, int EqVal>
	static void Jump(SPC700& cpu, bool skipped) {
		cpu.regs.PC++;
		cpu.BufferJump = ((cpu.regs.P & AndVal) == EqVal);
	}

	static void JumpAlways(SPC700& cpu, bool skipped) {
		cpu.regs.PC++;
		cpu.BufferJump = true;
	}

	template <int step>
	static void DoJump(SPC700& cpu, bool skipped) {
		switch(step) {
		case 1:
			cpu.BufferAddress = cpu.regs.PC + (int8_t)(cpu.BufferOperand);
			cpu.regs.PC = (get_hi(cpu.regs.PC) << 8) | get_lo(cpu.BufferAddress);
			break;
		case 2:
			cpu.regs.PC = (get_hi(cpu.BufferAddress) << 8) | get_lo(cpu.regs.PC);
			break;
		}
	}

	template <SubFunc func = SubFunc::None>
	static void IncrementOperand(SPC700& cpu, bool skipped) {
		cpu.BufferOperand += 1;
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void DecrementOperand(SPC700& cpu, bool skipped) {
		cpu.BufferOperand -= 1;
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void IncrementRegA(SPC700& cpu, bool skipped) {
		cpu.regs.A = (cpu.regs.A + 1) & 0xFF;
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void DecrementRegA(SPC700& cpu, bool skipped) {
		cpu.regs.A = (cpu.regs.A - 1) & 0xFF;
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void IncrementRegX(SPC700& cpu, bool skipped) {
		cpu.regs.X = (cpu.regs.X + 1) & 0xFF;
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void DecrementRegX(SPC700& cpu, bool skipped) {
		cpu.regs.X = (cpu.regs.X - 1) & 0xFF;
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void IncrementRegY(SPC700& cpu, bool skipped) {
		cpu.regs.Y = (cpu.regs.Y + 1) & 0xFF;
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void DecrementRegY(SPC700& cpu, bool skipped) {
		cpu.regs.Y = (cpu.regs.Y - 1) & 0xFF;
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void ASL(SPC700& cpu, bool skipped) {
		if (cpu.BufferOperand & 0x80) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}

		cpu.BufferOperand = cpu.BufferOperand << 1;

		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void ASL_A(SPC700& cpu, bool skipped) {
		if (cpu.regs.A & 0x80) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}

		cpu.regs.A = (cpu.regs.A << 1) & 0xFF;
		
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void LSR(SPC700& cpu, bool skipped) {
		if (cpu.BufferOperand & 0x01) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}

		cpu.BufferOperand = cpu.BufferOperand >> 1;

		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void LSR_A(SPC700& cpu, bool skipped) {
		if (cpu.regs.A & 0x01) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}

		cpu.regs.A = (cpu.regs.A & 0xFF) >> 1;
		
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void ROL(SPC700& cpu, bool skipped) {
		cpu.BufferTmp = cpu.get_flag_C();
		if (cpu.BufferOperand & 0x80) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
		cpu.BufferOperand = (cpu.BufferOperand << 1) | cpu.BufferTmp;
		
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void ROL_A(SPC700& cpu, bool skipped) {
		cpu.BufferTmp = cpu.get_flag_C();
		if (cpu.regs.A & 0x80) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
		cpu.regs.A = ((cpu.regs.A << 1) & 0xFF) | cpu.BufferTmp;
		
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void ROR(SPC700& cpu, bool skipped) {
		cpu.BufferTmp = (cpu.get_flag_C() << 7);
		if (cpu.BufferOperand & 0x01) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
		cpu.BufferOperand = (cpu.BufferOperand >> 1) | cpu.BufferTmp;
		
		sub_func(cpu, skipped, func);
	}

	template <SubFunc func = SubFunc::None>
	static void ROR_A(SPC700& cpu, bool skipped) {
		cpu.BufferTmp = (cpu.get_flag_C() << 7);
		if (cpu.regs.A & 0x01) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
		cpu.regs.A = ((cpu.regs.A & 0xFF) >> 1) | cpu.BufferTmp;
		
		sub_func(cpu, skipped, func);
	}

	static void SetAddressXPSW(SPC700& cpu, bool skipped) {
		cpu.BufferAddress = ((cpu.regs.X & 0xFF) | ((cpu.regs.P & 0x20) << 3));
	}

	static void OR1Neq(SPC700& cpu, bool skipped) {
		Byte bit = cpu.BufferAddress >> 13;
		bool flag = cpu.get_flag_C();
		if (flag || (cpu.BufferOperand & (1 << bit)) != 0) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
	}

	static void OR1Eq(SPC700& cpu, bool skipped) {
		Byte bit = cpu.BufferAddress >> 13;
		bool flag = cpu.get_flag_C();
		if (flag || (cpu.BufferOperand & (1 << bit)) == 0) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
	}

	static void EOR1(SPC700& cpu, bool skipped) {
		Byte bit = cpu.BufferAddress >> 13;
		bool flag = cpu.get_flag_C();
		if (flag ^ (cpu.BufferOperand & (1 << bit)) != 0) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
	}

	static void AND1Neq(SPC700& cpu, bool skipped) {
		Byte bit = cpu.BufferAddress >> 13;
		bool flag = cpu.get_flag_C();
		if (flag && (cpu.BufferOperand & (1 << bit)) != 0) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
	}

	static void AND1Eq(SPC700& cpu, bool skipped) {
		Byte bit = cpu.BufferAddress >> 13;
		bool flag = cpu.get_flag_C();
		if (flag && (cpu.BufferOperand & (1 << bit)) == 0) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
	}

	static void MOV1_AA(SPC700& cpu, bool skipped) {
		Byte bit = cpu.BufferAddress >> 13;
		if ((cpu.BufferOperand & (1 << bit)) != 0) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
	}

	static void MOV1_CA(SPC700& cpu, bool skipped) {
		Byte bit = cpu.BufferAddress >> 13;
		if (cpu.get_flag_C()) {
			cpu.BufferOperand = cpu.BufferOperand | (1 << bit);
		} else {
			cpu.BufferOperand = cpu.BufferOperand & ~(1 << bit);
		}
	}

	static void NOT1(SPC700& cpu, bool skipped) {
		Byte bit = cpu.BufferAddress >> 13;
		cpu.BufferOperand = cpu.BufferOperand ^ (1 << bit);
	}

	static void TSET1(SPC700& cpu, bool skipped) {
		cpu.BufferTmp = (cpu.regs.A & 0xFF) - cpu.BufferOperand;
		cpu.BufferOperand = cpu.BufferOperand | (cpu.regs.A & 0xFF);
		if (cpu.BufferTmp == 0) {
			cpu.set_flag_Z();
		} else {
			cpu.clear_flag_Z();
		}
		if (cpu.BufferTmp & 0x80) {
			cpu.set_flag_N();
		} else {
			cpu.clear_flag_N();
		}
	}

	static void TCLR1(SPC700& cpu, bool skipped) {
		cpu.BufferTmp = (cpu.regs.A & 0xFF) - cpu.BufferOperand;
		cpu.BufferOperand = cpu.BufferOperand & ~(cpu.regs.A & 0xFF);
		if (cpu.BufferTmp == 0) {
			cpu.set_flag_Z();
		} else {
			cpu.clear_flag_Z();
		}
		if (cpu.BufferTmp & 0x80) {
			cpu.set_flag_N();
		} else {
			cpu.clear_flag_N();
		}
	}

	template <int step>
	static void CBNE(SPC700& cpu, bool skipped) {
		switch(step) {
		case 1:
			cpu.BufferJump = ((cpu.regs.A & 0xFF) != cpu.BufferOperand);
			break;
		case 2:
			cpu.BufferAddress = cpu.regs.PC + (int8_t)(cpu.BufferOperand);
			cpu.regs.PC = (get_hi(cpu.regs.PC) << 8) | (uint8_t)(get_lo(cpu.BufferAddress));
			break;
		case 3:
			cpu.regs.PC = (get_hi(cpu.BufferAddress) << 8) | (get_lo(cpu.regs.PC));
			break;
		}
	}

	template <int step>
	static void DBNZ_6E(SPC700& cpu, bool skipped) {
		switch(step) {
		case 1:
			cpu.BufferOperand -= 1;
			cpu.BufferJump = (cpu.BufferOperand != 0);
			break;
		case 2:
			cpu.BufferAddress = cpu.regs.PC + (int8_t)(cpu.BufferOperand);
			cpu.regs.PC = (get_hi(cpu.regs.PC) << 8) | (uint8_t)(get_lo(cpu.BufferAddress));
			break;
		case 3:
			cpu.regs.PC = (get_hi(cpu.BufferAddress) << 8) | (get_lo(cpu.regs.PC));
			break;
		}
	}

	template <int step>
	static void DBNZ_FE(SPC700& cpu, bool skipped) {
		switch(step) {
		case 1:
			cpu.regs.PC += 1;
			cpu.regs.Y = (cpu.regs.Y - 1) & 0xFF;
			cpu.BufferJump = (cpu.regs.Y != 0);
			break;
		case 2:
			cpu.BufferAddress = cpu.regs.PC + (int8_t)(cpu.BufferOperand);
			cpu.regs.PC = (get_hi(cpu.regs.PC) << 8) | (uint8_t)(get_lo(cpu.BufferAddress));
			break;
		case 3:
			cpu.regs.PC = (get_hi(cpu.BufferAddress) << 8) | (get_lo(cpu.regs.PC));
			break;
		}
	}

	template <int step>
	static void DECW(SPC700& cpu, bool skipped) {
		switch(step) {
		case 1:
			cpu.BufferUnderflow = (cpu.BufferOperand == 0);
			cpu.BufferOperand -= 1;
			cpu.BufferLowZero = (cpu.BufferOperand == 0);
			break;
		case 2:
			cpu.BufferOperand -= cpu.BufferUnderflow;
			if (cpu.BufferOperand & 0x80) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (cpu.BufferLowZero && !cpu.BufferOperand) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
	}

	template <int step>
	static void INCW(SPC700& cpu, bool skipped) {
		Byte value = cpu.BufferOperand & 0xFF;
		switch(step) {
		case 1:
			cpu.BufferOverflow = (value == 0xFF);
			value += 1;
			cpu.BufferLowZero = (value == 0);
			break;
		case 2:
			value += cpu.BufferOverflow;
			if (value & 0x80) {
				cpu.set_flag_N();
			} else {
				cpu.clear_flag_N();
			}
			if (cpu.BufferLowZero && !value) {
				cpu.set_flag_Z();
			} else {
				cpu.clear_flag_Z();
			}
		}
		cpu.BufferOperand = value;
	}

	static void CMPW(SPC700& cpu, bool skipped) {
		int tmp = ya(cpu) - cpu.BufferOperand16;
		if (tmp >= 0) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
		if (tmp & 0x8000) {
			cpu.set_flag_N();
		} else {
			cpu.clear_flag_N();
		}
		if (!tmp) {
			cpu.set_flag_Z();
		} else {
			cpu.clear_flag_Z();
		}
	}

	template <bool subw>
	static void ADDSUBW(SPC700& cpu, bool skipped) {

		int tmp_r = 0;

		if constexpr (subw) {
			tmp_r = ~cpu.BufferOperand16;
		} else {
			tmp_r = cpu.BufferOperand16;
		}

		int tmp = (uint32_t)(cpu.regs.A & 0xFF) + (uint32_t)(get_lo(tmp_r)) + (subw ? 1 : 0);
		if (tmp > 0xFF) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}
		cpu.regs.A = (uint8_t)(tmp);

		tmp = (uint32_t)(cpu.regs.Y & 0xFF) + (uint32_t)(get_hi(tmp_r)) + (uint32_t)(cpu.get_flag_C());

		if (tmp > 0xFF) {
			cpu.set_flag_C();
		} else {
			cpu.clear_flag_C();
		}

		if (((cpu.regs.Y & 0xFF) ^ get_hi(tmp_r) ^ (uint8_t)(tmp)) & 0x10) {
			cpu.set_flag_H();
		} else {
			cpu.clear_flag_H();
		}

		if (~((cpu.regs.Y & 0xFF) ^ get_hi(tmp_r)) & ((cpu.regs.Y & 0xFF) ^ (uint8_t)(tmp)) & 0x80) {
			cpu.set_flag_V();
		} else {
			cpu.clear_flag_V();
		}

		if ((uint8_t)(tmp) & 0x80) {
			cpu.set_flag_N();
		} else {
			cpu.clear_flag_N();
		}

		cpu.regs.Y = (uint8_t)(tmp);

		if (ya(cpu) == 0x0000) {
			cpu.set_flag_Z();
		} else {
			cpu.clear_flag_Z();
		}
	}

	static void MOVW(SPC700& cpu, bool skipped) {
		if (ya(cpu) & 0x8000) {
			cpu.set_flag_N();
		} else {
			cpu.clear_flag_N();
		}
		if (!ya(cpu)) {
			cpu.set_flag_Z();
		} else {
			cpu.clear_flag_Z();
		}
	}

	template <int step>
	static void CALL_3F(SPC700& cpu, bool skipped) {
		switch(step) {
		case 1:
			cpu.regs.PC = (get_hi(cpu.regs.PC) << 8) | (get_lo(cpu.BufferAddress));
			break;
		case 2:
			cpu.regs.PC = (get_hi(cpu.BufferAddress) << 8) | (get_lo(cpu.regs.PC));
			break;
		}
	}

	static void PCALL(SPC700& cpu, bool skipped) {
		cpu.regs.PC = 0xFF00 | get_lo(cpu.BufferAddress);
	}

	static void SetPCToAddress(SPC700& cpu, bool skipped) {
		cpu.regs.PC = cpu.BufferAddress;
	}

	static void IncrementPointerByY(SPC700& cpu, bool skipped) {
		cpu.BufferPointer = (cpu.BufferPointer + (cpu.regs.Y & 0xFF)) & 0xFFFF;
	}
}

// NOP (00)
Instruction<SPC700> s_00 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// TCALL0 (01)
Instruction<SPC700> s_01 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// SET1 (02)
Instruction<SPC700> s_02 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::SET<0>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBS0 (03)
Instruction<SPC700> s_03 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBS<0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// PUSH (0D)
Instruction<SPC700> s_0d = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::P, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::DecrementS),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BRK (0F)
Instruction<SPC700> s_0f = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::P, WriteTo::StackMinus2>),
	MakeHandler(SPC700Functions::DecrementS<3, SubFunc::ClearISetX>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::FFDE, ReadTo::PCLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::FFDF, ReadTo::PCHigh>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// TCALL1 (11)
Instruction<SPC700> s_11 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// CLR1 (12)
Instruction<SPC700> s_12 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::CLR<0>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBC0 (13)
Instruction<SPC700> s_13 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBC<0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// CLRP (20)
Instruction<SPC700> s_20 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::CLRP),
	MakeHandler(SPC700Functions::Next)
};

// TCALL2 (21)
Instruction<SPC700> s_21 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// SET2 (22)
Instruction<SPC700> s_22 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::SET<1>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBS1 (23)
Instruction<SPC700> s_23 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBS<1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// PUSH (2D)
Instruction<SPC700> s_2d = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::A, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::DecrementS),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// TCALL3 (31)
Instruction<SPC700> s_31 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<3>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<3>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// CLR2 (32)
Instruction<SPC700> s_32 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::CLR<1>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBC1 (33)
Instruction<SPC700> s_33 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBC<1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// SETP (40)
Instruction<SPC700> s_40 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SETP),
	MakeHandler(SPC700Functions::Next)
};

// TCALL4 (41)
Instruction<SPC700> s_41 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<4>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<4>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// SET3 (42)
Instruction<SPC700> s_42 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::SET<2>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBS2 (43)
Instruction<SPC700> s_43 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// PUSH (4D)
Instruction<SPC700> s_4d = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::X, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::DecrementS),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// TCALL5 (51)
Instruction<SPC700> s_51 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<5>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<5>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// CLR3 (52)
Instruction<SPC700> s_52 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::CLR<2>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBC2 (53)
Instruction<SPC700> s_53 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBC<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// MOV (5D) (Implied)
Instruction<SPC700> s_5d = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::MOV<0x5D, 0, SubFunc::SetNZFlagRegisterX>),
	MakeHandler(SPC700Functions::Next)
};

// CLRC (60)
Instruction<SPC700> s_60 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::CLRC),
	MakeHandler(SPC700Functions::Next)
};

// TCALL6 (61)
Instruction<SPC700> s_61 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<6>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<6>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// SET4 (62)
Instruction<SPC700> s_62 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::SET<3>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBS3 (63)
Instruction<SPC700> s_63 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBS<3>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// PUSH (6D)
Instruction<SPC700> s_6d = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::Y, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::DecrementS),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// RET (6F)
Instruction<SPC700> s_6f = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::IncrementS),
	MakeHandler(SPC700Functions::Read<ReadFrom::Stack0, ReadTo::PCLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Stack1, ReadTo::PCHigh>),
	MakeHandler(SPC700Functions::IncrementS),
	MakeHandler(SPC700Functions::Next)
};

// TCALL7 (71)
Instruction<SPC700> s_71 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<7>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<7>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// CLR4 (72)
Instruction<SPC700> s_72 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::CLR<3>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBC3 (73)
Instruction<SPC700> s_73 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBC<3>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// MOV (7D) (Implied)
Instruction<SPC700> s_7d = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::MOV<0x7D, 0, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// RETI (7F)
Instruction<SPC700> s_7f = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::IncrementS),
	MakeHandler(SPC700Functions::Read<ReadFrom::Stack0, ReadTo::P>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Stack1, ReadTo::PCLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Stack2, ReadTo::PCHigh>),
	MakeHandler(SPC700Functions::IncrementS<2>),
	MakeHandler(SPC700Functions::Next)
};

// SETC (80)
Instruction<SPC700> s_80 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SETC),
	MakeHandler(SPC700Functions::Next)
};

// TCALL8 (81)
Instruction<SPC700> s_81 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<8>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<8>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// SET5 (82)
Instruction<SPC700> s_82 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::SET<4>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBS4 (83)
Instruction<SPC700> s_83 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBS<4>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// MOV (8D) (Immediate)
Instruction<SPC700> s_8d = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0x8D, 0, SubFunc::SetNZFlagOperand, true>),
	MakeHandler(SPC700Functions::Next)
};

// POP (8E)
Instruction<SPC700> s_8e = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::IncrementS),
	MakeHandler(SPC700Functions::Read<ReadFrom::Stack0, ReadTo::P>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (8F) (Immediate Data to Direct Page d, #i (Read/Modify/Write))
Instruction<SPC700> s_8f = {
	IMMEDIATE_DATA_TO_DIRECT_PAGE_D_READ_MODIFY_WRITE_START
	MakeHandler(SPC700Functions::MOV<0x8f, 0, SubFunc::None>),
	IMMEDIATE_DATA_TO_DIRECT_PAGE_D_READ_MODIFY_WRITE_END
};

// TCALL9 (91)
Instruction<SPC700> s_91 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<9>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<9>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// CLR5 (92)
Instruction<SPC700> s_92 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::CLR<4>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBC4 (93)
Instruction<SPC700> s_93 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBC<4>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// MOV (9D)
Instruction<SPC700> s_9d = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::MOV<0x9d, 0, SubFunc::SetNZFlagRegisterX>),
	MakeHandler(SPC700Functions::Next)
};

// DIV (9E)
Instruction<SPC700> s_9e = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DIV<1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DIV<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DIV<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DIV<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DIV<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DIV<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DIV<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DIV<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DIV<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DIV<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DIV<3, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next),
};

// XCN (9F)
Instruction<SPC700> s_9f = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::XCN),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::XCN),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::XCN),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::XCN<SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next),
};

// EI (A0)
Instruction<SPC700> s_a0 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::EI),
	MakeHandler(SPC700Functions::Next)
};

// TCALL10 (A1)
Instruction<SPC700> s_a1 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<10>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<10>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// SET6 (A2)
Instruction<SPC700> s_a2 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::SET<5>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBS5 (A3)
Instruction<SPC700> s_a3 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBS<5>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// POP (AE)
Instruction<SPC700> s_ae = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::IncrementS),
	MakeHandler(SPC700Functions::Read<ReadFrom::Stack0, ReadTo::A>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (AF)
Instruction<SPC700> s_af = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::SetFuncX),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::A, WriteTo::Address>),
	MakeHandler(SPC700Functions::IncrementX<1>),
	MakeHandler(SPC700Functions::Next)
};

// TCALL11 (B1)
Instruction<SPC700> s_b1 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<11>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<11>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// CLR6 (B2)
Instruction<SPC700> s_b2 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::CLR<5>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBC5 (B3)
Instruction<SPC700> s_b3 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBC<5>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// DAS  (BE)
Instruction<SPC700> s_be = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DAS<1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DAS<2>),
	MakeHandler(SPC700Functions::Next)
};

// DI (C0)
Instruction<SPC700> s_c0 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DI),
	MakeHandler(SPC700Functions::Next)
};

// TCALL12 (C1)
Instruction<SPC700> s_c1 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<12>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<12>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// SET7 (C2)
Instruction<SPC700> s_c2 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::SET<6>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBS6 (C3)
Instruction<SPC700> s_c3 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBS<6>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// POP (CE)
Instruction<SPC700> s_ce = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::IncrementS),
	MakeHandler(SPC700Functions::Read<ReadFrom::Stack0, ReadTo::X>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MUL (CF)
Instruction<SPC700> s_cf = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::MUL<1>),
	MakeHandler(SPC700Functions::MUL<2, SubFunc::SetNZFlagRegisterYA>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// TCALL13 (D1)
Instruction<SPC700> s_d1 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<13>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<13>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// CLR7 (D2)
Instruction<SPC700> s_d2 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::CLR<6>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBC6 (D3)
Instruction<SPC700> s_d3 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBC<6>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// DAA  (DF)
Instruction<SPC700> s_df = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DAA<1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DAA<2>),
	MakeHandler(SPC700Functions::Next)
};

// CLRV (E0)
Instruction<SPC700> s_e0 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::CLRV),
	MakeHandler(SPC700Functions::Next)
};

// TCALL14 (E1)
Instruction<SPC700> s_e1 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<14>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<14>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// SET8 (E2)
Instruction<SPC700> s_e2 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::SET<7>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBS7 (E3)
Instruction<SPC700> s_e3 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBS<7>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// MOV (E4)
Instruction<SPC700> s_e4 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xE4, 0, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (E5)
Instruction<SPC700> s_e5 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xE5, 0, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// NOTC (ED)
Instruction<SPC700> s_ed = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOTC),
	MakeHandler(SPC700Functions::Next)
};

// POP (EE)
Instruction<SPC700> s_ee = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::IncrementS),
	MakeHandler(SPC700Functions::Read<ReadFrom::Stack0, ReadTo::Y>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// TCALL15 (F1)
Instruction<SPC700> s_f1 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallLow<15>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::TCallHigh<15>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next),	
};

// CLR8 (F2)
Instruction<SPC700> s_f2 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::CLR<7>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// BBC7 (F3)
Instruction<SPC700> s_f3 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BBC<7>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Branch<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// MOV (F4)
Instruction<SPC700> s_f4 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xF4, 0, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (F5)
Instruction<SPC700> s_f5 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::IncrementAddressByX>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xE5, 0, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};


Instruction<SPC700> s_nop = {
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
};



// BITWISE OPS

// Direct Page d (Read)
// OR (04)
Instruction<SPC700> s_04 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// AND (24)
Instruction<SPC700> s_24 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// EOR (44)
Instruction<SPC700> s_44 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// ADC (84)
Instruction<SPC700> s_84 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// SBC (A4)
Instruction<SPC700> s_a4 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// CMP (64)
Instruction<SPC700> s_64 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// X-Indexed Direct Page d+X (Read)

// OR (14)
Instruction<SPC700> s_14 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// AND (34)
Instruction<SPC700> s_34 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// EOR (54)
Instruction<SPC700> s_54 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// ADC (94)
Instruction<SPC700> s_94 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// SBC (B4)
Instruction<SPC700> s_b4 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// CMP (74)
Instruction<SPC700> s_74 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// Absolute !a (Read)

// OR (05)
Instruction<SPC700> s_05 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// AND (25)
Instruction<SPC700> s_25 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// EOR (45)
Instruction<SPC700> s_45 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// ADC (85)
Instruction<SPC700> s_85 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// SBC (A5)
Instruction<SPC700> s_a5 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// CMP (65)
Instruction<SPC700> s_65 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// X-Indexed Absolute !a+X (Read)

// OR (15)
Instruction<SPC700> s_15 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerXPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// AND (35)
Instruction<SPC700> s_35 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerXPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// EOR (55)
Instruction<SPC700> s_55 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerXPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// ADC (95)
Instruction<SPC700> s_95 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerXPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// SBC (B5)
Instruction<SPC700> s_b5 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerXPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// CMP (75)
Instruction<SPC700> s_75 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerXPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// Y-Indexed Absolute !a+Y (Read)

// OR (16)
Instruction<SPC700> s_16 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerYPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// AND (36)
Instruction<SPC700> s_36 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerYPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// EOR (56)
Instruction<SPC700> s_56 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerYPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// ADC (96)
Instruction<SPC700> s_96 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerYPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// SBC (B6)
Instruction<SPC700> s_b6 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerYPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// CMP (76)
Instruction<SPC700> s_76 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerYPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// Indirect X (X) (Read)

// OR (06)
Instruction<SPC700> s_06 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// AND (26)
Instruction<SPC700> s_26 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// EOR (46)
Instruction<SPC700> s_46 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// ADC (86)
Instruction<SPC700> s_86 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// SBC (A6)
Instruction<SPC700> s_a6 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// CMP (66)
Instruction<SPC700> s_66 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// X-Indexed Indirect [d+X] (Read)

// OR (07)
Instruction<SPC700> s_07 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// AND (27)
Instruction<SPC700> s_27 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// EOR (47)
Instruction<SPC700> s_47 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// ADC (87)
Instruction<SPC700> s_87 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// SBC (A7)
Instruction<SPC700> s_a7 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// CMP (67)
Instruction<SPC700> s_67 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// Immediate

// OR (08)
Instruction<SPC700> s_08 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA, true>),
	MakeHandler(SPC700Functions::Next)
};

// AND (28)
Instruction<SPC700> s_28 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA, true>),
	MakeHandler(SPC700Functions::Next)
};

// EOR (48)
Instruction<SPC700> s_48 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA, true>),
	MakeHandler(SPC700Functions::Next)
};

// ADC (88)
Instruction<SPC700> s_88 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::A, Value::Operand, SubFunc::None, true>),
	MakeHandler(SPC700Functions::Next)
};

// SBC (A8)
Instruction<SPC700> s_a8 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::A, Value::Operand, SubFunc::None, true>),
	MakeHandler(SPC700Functions::Next)
};

// CMP (68)
Instruction<SPC700> s_68 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::A, Value::Operand, SubFunc::None, true>),
	MakeHandler(SPC700Functions::Next)
};


// Immediate Data to Direct Page d, #i (Read/Modify/Write)

// OR (18)
Instruction<SPC700> s_18 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::Operand0, Value::Operand1, SubFunc::SetNZFlagOperand0>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// AND (38)
Instruction<SPC700> s_38 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::Operand0, Value::Operand1, SubFunc::SetNZFlagOperand0>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// EOR (58)
Instruction<SPC700> s_58 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::Operand0, Value::Operand1, SubFunc::SetNZFlagOperand0>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ADC (98)
Instruction<SPC700> s_98 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::Operand0, Value::Operand1>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// SBC (B8)
Instruction<SPC700> s_b8 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::Operand0, Value::Operand1>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// CMP (78)
Instruction<SPC700> s_78 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::Operand0, Value::Operand1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// Direct Page to Direct Page dd, ds (Read/Modify/Write)

// OR (09)
Instruction<SPC700> s_09 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::Operand0, Value::Operand1, SubFunc::SetNZFlagOperand0>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// AND (29)
Instruction<SPC700> s_29 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::Operand0, Value::Operand1, SubFunc::SetNZFlagOperand0>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// EOR (49)
Instruction<SPC700> s_49 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::Operand0, Value::Operand1, SubFunc::SetNZFlagOperand0>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ADC (89)
Instruction<SPC700> s_89 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::Operand0, Value::Operand1>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// SBC (A9)
Instruction<SPC700> s_a9 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::Operand0, Value::Operand1>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// CMP (69)
Instruction<SPC700> s_69 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::Operand0, Value::Operand1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// Indirect Page to Indirect Page (Read)

// OR (19)
Instruction<SPC700> s_19 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::YPSW, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::Operand0, Value::Operand1, SubFunc::SetNZFlagOperand0>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::XPSW>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// AND (39)
Instruction<SPC700> s_39 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::YPSW, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::Operand0, Value::Operand1, SubFunc::SetNZFlagOperand0>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::XPSW>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// EOR (59)
Instruction<SPC700> s_59 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::YPSW, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::Operand0, Value::Operand1, SubFunc::SetNZFlagOperand0>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::XPSW>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ADC (99)
Instruction<SPC700> s_99 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::YPSW, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::Operand0, Value::Operand1>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::XPSW>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// SBC (B9)
Instruction<SPC700> s_b9 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::YPSW, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::Operand0, Value::Operand1>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::XPSW>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// CMP (79)
Instruction<SPC700> s_79 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::YPSW, ReadTo::Operand1>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::Operand0, Value::Operand1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// Absolute !a (Read)
// CMP X (1E)
Instruction<SPC700> s_1e = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::X, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// CMP Y (5E)
Instruction<SPC700> s_5e = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::Y, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// Immediate
// CMP X (C8)
Instruction<SPC700> s_c8 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::X, Value::Operand, SubFunc::None, true>),
	MakeHandler(SPC700Functions::Next)
};

// CMP Y (AD)
Instruction<SPC700> s_ad = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::Y, Value::Operand, SubFunc::None, true>),
	MakeHandler(SPC700Functions::Next)
};

// Rest of the MOV Instruction<SPC700>s

// MOV (F6)
Instruction<SPC700> s_f6 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::IncrementAddressByY>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xF6, 0, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (E6)
Instruction<SPC700> s_e6 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::XPSW, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xE6, 0, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (E7)
Instruction<SPC700> s_e7 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xE7, 0, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (C4)
Instruction<SPC700> s_c4 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::A, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (D4)
Instruction<SPC700> s_d4 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::A, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (C5)
Instruction<SPC700> s_c5 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::A, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (D5)
Instruction<SPC700> s_d5 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::IncrementAddressByX>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::A, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (C6)
Instruction<SPC700> s_c6 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::SetAddressXPSW),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::A, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (D6)
Instruction<SPC700> s_d6 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::IncrementAddressByY>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::A, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (C7)
Instruction<SPC700> s_c7 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::A, WriteTo::Pointer>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (F8)
Instruction<SPC700> s_f8 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xF8, 0, SubFunc::SetNZFlagRegisterX>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (F9)
Instruction<SPC700> s_f9 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusY),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xF9, 0, SubFunc::SetNZFlagRegisterX>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (E8)
Instruction<SPC700> s_e8 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xE8, 0, SubFunc::SetNZFlagRegisterA, true>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (D8)
Instruction<SPC700> s_d8 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::X, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (D9)
Instruction<SPC700> s_d9 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::SetFuncOperandPlusY),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::X, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (EB)
Instruction<SPC700> s_eb = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xEB, 0, SubFunc::SetNZFlagRegisterY>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (FB)
Instruction<SPC700> s_fb = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xFB, 0, SubFunc::SetNZFlagRegisterY>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (EC)
Instruction<SPC700> s_ec = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xEC, 0, SubFunc::SetNZFlagRegisterY>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (CB)
Instruction<SPC700> s_cb = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::Y, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (DB)
Instruction<SPC700> s_db = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::Y, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (CC)
Instruction<SPC700> s_cc = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::Y, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};


// Branches (all very similar)

// BPL (10)
Instruction<SPC700> s_10 = {
	MakeHandler(SPC700Functions::Jump<0x80, 0x00>),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// BMI (30)
Instruction<SPC700> s_30 = {
	MakeHandler(SPC700Functions::Jump<0x80, 0x80>),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// BVC (50)
Instruction<SPC700> s_50 = {
	MakeHandler(SPC700Functions::Jump<0x40, 0x00>),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// BPL (70)
Instruction<SPC700> s_70 = {
	MakeHandler(SPC700Functions::Jump<0x40, 0x40>),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// BCC (90)
Instruction<SPC700> s_90 = {
	MakeHandler(SPC700Functions::Jump<0x01, 0x00>),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// BCS (b0)
Instruction<SPC700> s_b0 = {
	MakeHandler(SPC700Functions::Jump<0x01, 0x01>),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// BNE (d0)
Instruction<SPC700> s_d0 = {
	MakeHandler(SPC700Functions::Jump<0x02, 0x00>),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// BEQ (f0)
Instruction<SPC700> s_f0 = {
	MakeHandler(SPC700Functions::Jump<0x02, 0x02>),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// BRA (2f)
Instruction<SPC700> s_2f = {
	MakeHandler(SPC700Functions::JumpAlways),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<1>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DoJump<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// INC/DEC (Reuse addressing modes from Bitwise Ops)

// Direct Page d (Read/Modify/Write)

// INC (AB)
Instruction<SPC700> s_ab = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementOperand<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// DEC (8B)
Instruction<SPC700> s_8b = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::DecrementOperand<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// X-Indexed Direct Page d+X (Read/Modify/Write)

// INC (BB)
Instruction<SPC700> s_bb = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementOperand<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// DEC (9B)
Instruction<SPC700> s_9b = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::DecrementOperand<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// Absolute !a (Read/Modify/Write)

// INC (AC)
Instruction<SPC700> s_ac = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementOperand<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// DEC (8C)
Instruction<SPC700> s_8c = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::DecrementOperand<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// Accumulator A

// INC (BC)
Instruction<SPC700> s_bc = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::IncrementRegA<SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// DEC (9C)
Instruction<SPC700> s_9c = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::DecrementRegA<SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// Implied

// INC (3D)
Instruction<SPC700> s_3d = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::IncrementRegX<SubFunc::SetNZFlagRegisterX>),
	MakeHandler(SPC700Functions::Next)
};

// DEC (1D)
Instruction<SPC700> s_1d = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DecrementRegX<SubFunc::SetNZFlagRegisterX>),
	MakeHandler(SPC700Functions::Next)
};

// Implied

// INC (FC)
Instruction<SPC700> s_fc = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::IncrementRegY<SubFunc::SetNZFlagRegisterY>),
	MakeHandler(SPC700Functions::Next)
};

// DEC (DC)
Instruction<SPC700> s_dc = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::DecrementRegY<SubFunc::SetNZFlagRegisterY>),
	MakeHandler(SPC700Functions::Next)
};

// ASL/ROL/LSR/ROR (Reuse addressing modes from Bitwise Ops)

// ASL (0B)
Instruction<SPC700> s_0b = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::ASL<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ROL (2B)
Instruction<SPC700> s_2b = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::ROL<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// LSR (4B)
Instruction<SPC700> s_4b = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::LSR<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ROR (6B)
Instruction<SPC700> s_6b = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::ROR<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ASL (1B)
Instruction<SPC700> s_1b = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::ASL<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ROL (3B)
Instruction<SPC700> s_3b = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::ROL<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// LSR (5B)
Instruction<SPC700> s_5b = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::LSR<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ROR (7B)
Instruction<SPC700> s_7b = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::ROR<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ASL (0C)
Instruction<SPC700> s_0c = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::ASL<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ROL (2C)
Instruction<SPC700> s_2c = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::ROL<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// LSR (4C)
Instruction<SPC700> s_4c = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::LSR<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ROR (6C)
Instruction<SPC700> s_6c = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::ROR<SubFunc::SetNZFlagOperand>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// ASL (1C)
Instruction<SPC700> s_1c = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::ASL_A<SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// ROL (3C)
Instruction<SPC700> s_3c = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::ROL_A<SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// LSR (5C)
Instruction<SPC700> s_5c = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::LSR_A<SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// ROR (7C)
Instruction<SPC700> s_7c = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::ROR_A<SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// CALL/JMP

// CALL (3F)
Instruction<SPC700> s_3f = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::CALL_3F<1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::CALL_3F<2>),
	MakeHandler(SPC700Functions::Next)
};

// PCALL (4F)
Instruction<SPC700> s_4f = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCHigh, WriteTo::Stack0>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::PCLow, WriteTo::StackMinus1>),
	MakeHandler(SPC700Functions::DecrementS<2>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::PCALL),
	MakeHandler(SPC700Functions::Next)
};

// JMP (1F)
Instruction<SPC700> s_1f = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetPointerXPlusAddress),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::PCLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PointerPlusOne, ReadTo::PCHigh>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// JMP (5F)
Instruction<SPC700> s_5f = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::SetPCToAddress),
	MakeHandler(SPC700Functions::Next)
};

// CBNE/DBNZ

// CBNE (2E)
Instruction<SPC700> s_2e = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::CBNE<1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::CBNE<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::CBNE<3>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// CBNE (DE)
Instruction<SPC700> s_de = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::SetFuncOperandPlusX),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::CBNE<1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::CBNE<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::CBNE<3>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// DBNZ (6E)
Instruction<SPC700> s_6e = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::DBNZ_6E<1>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DBNZ_6E<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DBNZ_6E<3>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// DBNZ (FE)
Instruction<SPC700> s_fe = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::DBNZ_FE<1>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DBNZ_FE<2>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::NOP, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::DBNZ_FE<3>, SPC700Predicates::NoJump),
	MakeHandler(SPC700Functions::Next)
};

// 16-bit ops

// DECW (1A)
Instruction<SPC700> s_1a = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::DECW<1>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::Operand>),
	MakeHandler(SPC700Functions::DECW<2>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::AddressPlusOnePSW>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// INCW (3A)
Instruction<SPC700> s_3a = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::INCW<1>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::Operand>),
	MakeHandler(SPC700Functions::INCW<2>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::AddressPlusOnePSW>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// CMPW (5A)
Instruction<SPC700> s_5a = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand16Low>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::Operand16High>),
	MakeHandler(SPC700Functions::CMPW),
	MakeHandler(SPC700Functions::Next)
};

// ADDW (7A)
Instruction<SPC700> s_7a = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand16Low>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::Operand16High>),
	MakeHandler(SPC700Functions::ADDSUBW<false>),
	MakeHandler(SPC700Functions::Next)
};

// SUBW (9A)
Instruction<SPC700> s_9a = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand16Low>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::Operand16High>),
	MakeHandler(SPC700Functions::ADDSUBW<true>),
	MakeHandler(SPC700Functions::Next)
};

// MOVW (BA)
Instruction<SPC700> s_ba = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::A>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::Y>),
	MakeHandler(SPC700Functions::MOVW),
	MakeHandler(SPC700Functions::Next)
};

// MOVW (DA)
Instruction<SPC700> s_da = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Write<WriteValue::A, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::Y, WriteTo::AddressPlusOnePSW>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
}; 

// Bit processor

// OR1 (0A)

Instruction<SPC700> s_0a = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address1FFF, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::OR1Neq),
	MakeHandler(SPC700Functions::Next)
};

// OR1 (2A)

Instruction<SPC700> s_2a = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address1FFF, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::OR1Eq),
	MakeHandler(SPC700Functions::Next)
};

// AND1 (4A)
Instruction<SPC700> s_4a = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address1FFF, ReadTo::Operand>),
	MakeHandler(SPC700Functions::AND1Neq),
	MakeHandler(SPC700Functions::Next)
};

// AND1 (6A)
Instruction<SPC700> s_6a = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address1FFF, ReadTo::Operand>),
	MakeHandler(SPC700Functions::AND1Eq),
	MakeHandler(SPC700Functions::Next)
};

// EOR1 (8A)

Instruction<SPC700> s_8a = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address1FFF, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::EOR1),
	MakeHandler(SPC700Functions::Next)
};

// MOV1 (AA)

Instruction<SPC700> s_aa = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address1FFF, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV1_AA),
	MakeHandler(SPC700Functions::Next)
};

// (CA)
Instruction<SPC700> s_ca = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address1FFF, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::MOV1_CA),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address1FFF>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// NOT1 (EA)
Instruction<SPC700> s_ea = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address1FFF, ReadTo::Operand>),
	MakeHandler(SPC700Functions::NOT1),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address1FFF>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// TSET/TCLR

// TSET1 (0E)
Instruction<SPC700> s_0e = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::TSET1),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// TCLR1 (4E)
Instruction<SPC700> s_4e = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::TCLR1),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// Miscellaneous (final 20, hooray)

// OR (17)
Instruction<SPC700> s_17 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::IncrementPointerByY),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::OR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// AND (37)
Instruction<SPC700> s_37 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::IncrementPointerByY),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::AND, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// EOR (57)
Instruction<SPC700> s_57 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::IncrementPointerByY),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::EOR, Value::A, Value::Operand, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// CMP (77)
Instruction<SPC700> s_77 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::IncrementPointerByY),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// ADC (97)
Instruction<SPC700> s_97 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::IncrementPointerByY),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::ADC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// SBC (B7)
Instruction<SPC700> s_b7 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::IncrementPointerByY),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::SBC, Value::A, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (F7)
Instruction<SPC700> s_f7 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::IncrementPointerByY),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xF7, 0, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (D7)
Instruction<SPC700> s_d7 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::PointerLow>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::AddressPlusOnePSW, ReadTo::PointerHigh>),
	MakeHandler(SPC700Functions::IncrementPointerByY),
	MakeHandler(SPC700Functions::Read<ReadFrom::Pointer, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::A, WriteTo::Pointer>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// CMP X (3E)
Instruction<SPC700> s_3e = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::X, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// CMP Y (7E)
Instruction<SPC700> s_7e = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::BITWISE<Bitwise::CMP, Value::Y, Value::Operand>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (BD)
Instruction<SPC700> s_bd = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::MOV<0xBD, 0, SubFunc::None>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (BF)
Instruction<SPC700> s_bf = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Discard>),
	MakeHandler(SPC700Functions::SetFuncX),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xBF, 0, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::IncrementX<1>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (C9)
Instruction<SPC700> s_c9 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Discard>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Write<WriteValue::X, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// MOV (CD)
Instruction<SPC700> s_cd = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xCD, 0, SubFunc::SetNZFlagRegisterX, true>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (DD)
Instruction<SPC700> s_dd = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::MOV<0xDD, 0, SubFunc::SetNZFlagRegisterA>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (FD)
Instruction<SPC700> s_fd = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::MOV<0xFD, 0, SubFunc::SetNZFlagRegisterY>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (E9)
Instruction<SPC700> s_e9 = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressLow>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::AddressHigh>),
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand>),
	MakeHandler(SPC700Functions::MOV<0xE9, 0, SubFunc::SetNZFlagRegisterX>),
	MakeHandler(SPC700Functions::Next)
};

// MOV (FA)
Instruction<SPC700> s_fa = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Read<ReadFrom::Address, ReadTo::Operand0>),
	MakeHandler(SPC700Functions::Read<ReadFrom::PC, ReadTo::Operand>),
	MakeHandler(SPC700Functions::IncrementPC<SubFunc::SetSubFunc>),
	MakeHandler(SPC700Functions::Write<WriteValue::Operand0, WriteTo::Address>),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::Next)
};

// SLEEP (EF)
Instruction<SPC700> s_ef = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700SpecialFunctions::Sleep),
	MakeHandler(SPC700Functions::NOP)
};

// STOP (FF)
Instruction<SPC700> s_ff = {
	MakeHandler(SPC700Functions::IncrementPC),
	MakeHandler(SPC700Functions::NOP),
	MakeHandler(SPC700SpecialFunctions::Stop),
	MakeHandler(SPC700Functions::NOP)
};