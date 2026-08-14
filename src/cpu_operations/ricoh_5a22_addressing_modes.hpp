// DO NOT FORGET TO CHANGE EMULATION MODE (CHANGE EMULATION_DIRECT_INDEXED_INDIRECT_D_X_READ HALF-CYCLE 5.2)

#define INSTRUCTION_START_CHECK_ROUTINE ;
#define INSTRUCTION_END_CHECK_ROUTINE ; 
#define PREDICATE_CHECK_ROUTINE ;

#define NEXT_OPCODE MakeHandler(Ricoh5A22Functions::Next) 


#define NATIVE_IMMEDIATE_NO_M \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),

#define NATIVE_IMMEDIATE_M \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::MFlagSet), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),

#define NATIVE_IMMEDIATE_NO_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>),

#define NATIVE_IMMEDIATE_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::XFlagSet), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::OperandHigh>, Ricoh5A22Predicates::XFlagSet),

#define EMULATION_IMMEDIATE_M \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \

#define EMULATION_IMMEDIATE_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	
	
/*
#define NATIVE_RELATIVE ;
#define NATIVE_PROGRAM_COUNTER_RELATIVE_LONG ;*/

#define NATIVE_DIRECT_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \

#define NATIVE_DIRECT_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterALow, WriteTo::Address>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterAHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterALow, WriteTo::Address>), \

#define NATIVE_DIRECT_READ_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>, Ricoh5A22Predicates::XFlagSet),
#define EMULATION_DIRECT_READ_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \


/*#define NATIVE_DIRECT_READ_MODIFY_WRITE ;
#define NATIVE_DIRECT_WRITE ;*/

#define NATIVE_DIRECT_X_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOXD, false, BranchMode::None, true>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_X_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOXDEmulation, false, BranchMode::None, true>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>),

#define NATIVE_DIRECT_X_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOXD, false, BranchMode::None, true>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterALow, WriteTo::Address>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterAHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_X_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOXDEmulation, false, BranchMode::None, true>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterALow, WriteTo::Address>), \

#define NATIVE_DIRECT_Y_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOYD, false, BranchMode::None, true>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterALow, WriteTo::Address>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterAHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_Y_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOYDEmulation, false, BranchMode::None, true>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterALow, WriteTo::Address>), \


#define NATIVE_DIRECT_X_READ_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOXD, false, BranchMode::None, true>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>, Ricoh5A22Predicates::XFlagSet),
#define EMULATION_DIRECT_X_READ_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOXDEmulation, false, BranchMode::None, true>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>),


/*
#define NATIVE_DIRECT_X_READ_MODIFY_WRITE ;
#define NATIVE_DIRECT_X_WRITE ;
#define NATIVE_DIRECT_Y_READ ;
#define NATIVE_DIRECT_Y_WRITE ;
#define NATIVE_DIRECT_PAGE_INDIRECT ;*/

#define NATIVE_DIRECT_INDEXED_INDIRECT_D_X_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::DX>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::Pointer, ReadTo::Address, CopyMode::High>, Ricoh5A22Predicates::DLZero), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerDB, ReadTo::Operand>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneDBCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_INDEXED_INDIRECT_D_X_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::DXEmulation>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::Pointer, ReadTo::Address, CopyMode::High>, Ricoh5A22Predicates::DLZero), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusOneWrap16, ReadTo::PointerHigh>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerDB, ReadTo::Operand>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneDBCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),

#define NATIVE_DIRECT_INDEXED_INDIRECT_D_X_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::DX>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::Pointer, ReadTo::Address, CopyMode::High>, Ricoh5A22Predicates::DLZero), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerDB>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::PointerPlusOneDBCarry>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_INDEXED_INDIRECT_D_X_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::DXEmulation>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::Pointer, ReadTo::Address, CopyMode::High>, Ricoh5A22Predicates::DLZero), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusOneWrap16, ReadTo::PointerHigh>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerDB>), \

/*#define NATIVE_DIRECT_INDEXED_INDIRECT_D_X_READ_MODIFY_WRITE ;
#define NATIVE_DIRECT_INDEXED_INDIRECT_D_X_WRITE ;
*/

#define NATIVE_DIRECT_INDIRECT_LONG_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusTwo, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet), 
#define EMULATION_DIRECT_INDIRECT_LONG_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusTwo, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \

#define NATIVE_DIRECT_INDIRECT_LONG_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusTwo, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::PointerPlusOneBankCarry>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_INDIRECT_LONG_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusTwo, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \

#define NATIVE_DIRECT_INDIRECT_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerDB, ReadTo::Operand>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneDBCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet), 
#define EMULATION_DIRECT_INDIRECT_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerDB, ReadTo::Operand>), \

#define NATIVE_DIRECT_INDIRECT_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusOneWrap16, ReadTo::PointerHigh>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerDB>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::PointerPlusOneDBCarry>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_INDIRECT_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerDB>), \


/*#define NATIVE_DIRECT_INDIRECT_WRITE ; */

#define NATIVE_FLAG_BRANCH_LOGIC \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand, false, Branching::FlagNative>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::Address, ReadTo::PC>, Ricoh5A22Predicates::NoBranching), \
	MakeHandler(Ricoh5A22Functions::PollInterrupts, Ricoh5A22Predicates::NoBranching),

#define EMULATION_FLAG_BRANCH_LOGIC \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand, false, Branching::FlagEmulation>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::NoBranching), \
	MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::Address, ReadTo::PC, CopyMode::Low>, Ricoh5A22Predicates::NoBranching), \
	MakeHandler(Ricoh5A22Functions::PollInterrupts, Ricoh5A22Predicates::NoBranchingOrNoBoundaryCrossed), \
	MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::Address, ReadTo::PC, CopyMode::High>, Ricoh5A22Predicates::NoBranchingOrNoBoundaryCrossed),

#define NATIVE_DIRECT_INDIRECT_INDEXED_D_Y_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::DirectIndirectYIndex), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_INDIRECT_INDEXED_D_Y_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::DirectIndirectYIndex), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \

#define NATIVE_DIRECT_INDIRECT_INDEXED_D_Y_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::DirectIndirectYIndex), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::PointerPlusOneBankCarry>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_INDIRECT_INDEXED_D_Y_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::DirectIndirectYIndex), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \

#define NATIVE_STACK_RELATIVE_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::APS>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_STACK_RELATIVE_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::APSEmulation>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \

#define NATIVE_STACK_RELATIVE_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::APS>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_STACK_RELATIVE_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::APSEmulation>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>), \
	
/*#define NATIVE_STACK_RELATIVE_WRITE ;*/

#define NATIVE_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusTwo, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::DirectIndirectIndexedLongYIndex), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_INDIRECT_INDEXED_LONG_D_Y_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusTwo, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::DirectIndirectIndexedLongYIndex), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \

#define NATIVE_DIRECT_INDIRECT_INDEXED_LONG_D_Y_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusTwo, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::DirectIndirectIndexedLongYIndex), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterALow, WriteTo::PointerBank>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterAHigh, WriteTo::PointerPlusOneBankCarry>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_DIRECT_INDIRECT_INDEXED_LONG_D_Y_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, Mode::IfSkipped>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusTwo, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::DirectIndirectIndexedLongYIndex), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterALow, WriteTo::PointerBank>), \


#define NATIVE_ABSOLUTE_Y_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteYIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_ABSOLUTE_Y_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteYIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \

#define NATIVE_ABSOLUTE_Y_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteYIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::PointerPlusOneBankCarry>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_ABSOLUTE_Y_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteYIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \


#define NATIVE_ABSOLUTE_Y_READ_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteYIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::XFlagSet),
#define EMULATION_ABSOLUTE_Y_READ_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteYIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \

#define NATIVE_DIRECT_Y_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOYD, false, BranchMode::None, true>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>, Ricoh5A22Predicates::XFlagSet),
#define EMULATION_DIRECT_Y_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOYDEmulation, false, BranchMode::None, true>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \


#define NATIVE_ABSOLUTE_X_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteXIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_ABSOLUTE_X_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteXIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \

#define NATIVE_ABSOLUTE_X_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteXIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::PointerPlusOneBankCarry>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_ABSOLUTE_X_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteXIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \


#define NATIVE_ABSOLUTE_X_READ_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteXIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Native>), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::XFlagSet),
#define EMULATION_ABSOLUTE_X_READ_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteXIndex<Mode::PCIncrement>), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::ReadingCondition<Mode::Emulation>), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \


#define NATIVE_STACK_RELATIVE_INDIRECT_INDEXED_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::APS>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::StackRelativeIndirectIndexed), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_STACK_RELATIVE_INDIRECT_INDEXED_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::APSEmulation>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::StackRelativeIndirectIndexed), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \

#define NATIVE_STACK_RELATIVE_INDIRECT_INDEXED_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::APS>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::StackRelativeIndirectIndexed), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::PointerPlusOneBankCarry>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_STACK_RELATIVE_INDIRECT_INDEXED_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::APSEmulation>), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Pointer, Mode::PlusOne>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::StackRelativeIndirectIndexed), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::NOP), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \


/*
#define NATIVE_STACK_RELATIVE_INDIRECT_INDEXED_WRITE ;*/

/*
#define NATIVE_ABSOLUTE ;
#define NATIVE_ABSOLUTE_PUSH ;*/
#define NATIVE_ABSOLUTE_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressDB, ReadTo::Operand>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusOneDBCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_ABSOLUTE_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressDB, ReadTo::Operand>),

#define NATIVE_ABSOLUTE_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand, CopyMode::All, true>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOneDBCarry>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_ABSOLUTE_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand, CopyMode::All, true>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>), \


#define NATIVE_ABSOLUTE_READ_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressDB, ReadTo::Operand>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::XFlagSet), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusOneDBCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::XFlagSet),
#define EMULATION_ABSOLUTE_READ_X \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressDB, ReadTo::Operand>),

/*
#define NATIVE_ABSOLUTE_READ_MODIFY_WRITE ;
#define NATIVE_ABSOLUTE_WRITE ;
#define NATIVE_ABSOLUTE_X_READ ;
#define NATIVE_ABSOLUTE_X_WRITE ;
#define NATIVE_ABSOLUTE_Y_READ ;
#define NATIVE_ABSOLUTE_Y_WRITE ;
#define NATIVE_ABSOLUTE_LONG ;*/

#define NATIVE_ABSOLUTE_LONG_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressBank, ReadTo::Operand>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusOneBankCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_ABSOLUTE_LONG_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressBank, ReadTo::Operand>), \

#define NATIVE_ABSOLUTE_LONG_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand, CopyMode::All, true>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressBank>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOneBankCarry>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_ABSOLUTE_LONG_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::Copy<ReadFrom::RegisterA, ReadTo::Operand, CopyMode::All, true>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressBank>), \


#define NATIVE_ABSOLUTE_LONG_X_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteLongXIndex<Mode::PCIncrement>), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \
	/* 5.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 5.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_ABSOLUTE_LONG_X_READ \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>), \
	/* 4.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteLongXIndex<Mode::PCIncrement>), \
	/* 4.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \

#define NATIVE_ABSOLUTE_LONG_X_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteLongXIndex<Mode::PCIncrement>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterALow, WriteTo::PointerBank>), \
	/* 7.1 */ MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	/* 7.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterAHigh, WriteTo::PointerPlusOneBankCarry>, Ricoh5A22Predicates::MFlagSet),
#define EMULATION_ABSOLUTE_LONG_X_WRITE \
	/* 1.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 1.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	/* 2.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 2.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	/* 3.1 */ MakeHandler(Ricoh5A22Functions::IncrementPC), \
	/* 3.2 */ MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Bank>), \
	/* 6.1 */ MakeHandler(Ricoh5A22Functions::AbsoluteLongXIndex<Mode::PCIncrement>), \
	/* 6.2 */ MakeHandler(Ricoh5A22Functions::Write<WriteValue::RegisterALow, WriteTo::PointerBank>), \

#define NATIVE_DIRECT_READ_MODIFY_WRITE_START \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>, Ricoh5A22Predicates::MFlagSet),
	
#define NATIVE_DIRECT_READ_MODIFY_WRITE_END \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>), \
	MakeHandler(Ricoh5A22Functions::NOP),

#define EMULATION_DIRECT_READ_MODIFY_WRITE_START \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC, Ricoh5A22Predicates::DLZero), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOD, true>), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>),
	
#define EMULATION_DIRECT_READ_MODIFY_WRITE_END \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>), \
	MakeHandler(Ricoh5A22Functions::NOP),


#define IMPLIED \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Discard>),

#define NATIVE_ABSOLUTE_READ_MODIFY_WRITE_START \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressDB, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressPlusOneDBCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet), \

#define NATIVE_ABSOLUTE_READ_MODIFY_WRITE_END \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOneDBCarry>, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>), \
	MakeHandler(Ricoh5A22Functions::NOP),

#define EMULATION_ABSOLUTE_READ_MODIFY_WRITE_START \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Address>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::AddressHigh>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::AddressDB, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>),

#define EMULATION_ABSOLUTE_READ_MODIFY_WRITE_END \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::AddressDB>), \
	MakeHandler(Ricoh5A22Functions::NOP),

#define NATIVE_DIRECT_X_READ_MODIFY_WRITE_START \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOXD, false, BranchMode::None, true>), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand, Mode::PlusOne>, Ricoh5A22Predicates::MFlagSet),

#define NATIVE_DIRECT_X_READ_MODIFY_WRITE_END \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::AddressPlusOne>, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>), \
	MakeHandler(Ricoh5A22Functions::NOP),

#define EMULATION_DIRECT_X_READ_MODIFY_WRITE_START \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::IncrementPC<SetMode::AOXDEmulation, false, BranchMode::None, true>), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::DLZero), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::Address, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>),

#define EMULATION_DIRECT_X_READ_MODIFY_WRITE_END \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::Address>), \
	MakeHandler(Ricoh5A22Functions::NOP),


#define NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_START \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	MakeHandler(Ricoh5A22Functions::AbsoluteXRMWIndex<Mode::PCIncrement>), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerPlusOneBankCarry, ReadTo::OperandHigh>, Ricoh5A22Predicates::MFlagSet), \

#define NATIVE_ABSOLUTE_X_READ_MODIFY_WRITE_END \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::NOP, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandHigh, WriteTo::PointerPlusOneBankCarry>, Ricoh5A22Predicates::MFlagSet), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \
	MakeHandler(Ricoh5A22Functions::NOP), \

#define EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_START \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::Pointer>), \
	MakeHandler(Ricoh5A22Functions::IncrementPC), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PCPB, ReadTo::PointerHigh>), \
	MakeHandler(Ricoh5A22Functions::AbsoluteXRMWIndex<Mode::PCIncrement>), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::Read<ReadFrom::PointerBank, ReadTo::Operand>), \
	MakeHandler(Ricoh5A22Functions::NOP), \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>),

#define EMULATION_ABSOLUTE_X_READ_MODIFY_WRITE_END \
	MakeHandler(Ricoh5A22Functions::Write<WriteValue::OperandLow, WriteTo::PointerBank>), \
	MakeHandler(Ricoh5A22Functions::NOP),

/*
#define NATIVE_ABSOLUTE_LONG_WRITE ;
#define NATIVE_ABSOLUTE_LONG_X_READ ;
#define NATIVE_ABSOLUTE_LONG_X_WRITE ;
#define NATIVE_ABSOLUTE_INDIRECT_JUMP ;
#define NATIVE_ABSOLUTE_INDIRECT_LONG_JUMP ;

#define NATIVE_BLOCK_MOVE_POSITIVE ;
#define NATIVE_BLOCK_MOVE_NEGATIVE ;
*/