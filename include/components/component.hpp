#pragma once
#include "common.hpp"

class Component {
public:
	
	virtual ~Component() = default;

	virtual void add_cycles(i64 cycles) = 0;

	virtual void tick_component() = 0;
	virtual i64 get_cycle() = 0;
	virtual i64 get_tick() = 0;

	virtual void reset() {}
	virtual void initialise() {}

	virtual u8 read(u32 addr) = 0;
	virtual void write(u32 addr, u8 value) = 0;

	virtual u8 communication_read(SNESAddress addr) = 0;
	virtual void communication_write(SNESAddress addr, u8 value) = 0;

};