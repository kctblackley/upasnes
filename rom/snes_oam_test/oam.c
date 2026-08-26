// Behavior os SNES OAM registers

#include <stdint.h>

// State
struct {
	unsigned odd  : 1; // even/odd byte flag
	unsigned addr : 8; // 8-bit word index
	unsigned bank : 1; // low/high bank flag
	unsigned buf  : 8; // 8-bit buffer for word writes
	uint8_t oam [544]; // memory
} s;

// Internals
int index()
{
	int addr = s.addr*2 + s.odd;
	if ( s.bank == 1 )
		addr = 0x200 + (addr & 0x1F);
	
	return addr;
}

void next()
{
	s.odd ^= 1;
	if ( s.odd == 0 )
	{
		s.addr++;
		
		if ( s.addr == 0 )
			s.bank ^= 1;
	}
}

// Read/write events

// OAMADDL
void write_2102( int data )
{
	s.addr = data;
	s.odd  = 0;
}

// OAMADDH
void write_2103( int data )
{
	s.bank = data & 1;
	s.odd  = 0;
}

// OAMDATA
void write_2104( int data )
{
	// Buffer always set by even write
	if ( s.odd == 0 )
		s.buf = data;
	
	// High bank goes directly through
	if ( s.bank == 1 )
		s.oam [index()] = data;
	
	// Low bank does word only on odd writes
	if ( s.bank == 0 && s.odd == 1 )
	{
		s.oam [index() - 1] = s.buf;
		s.oam [index() - 0] = data;
	}
	
	next();
}

// OAMDATAREAD
int read_2138()
{
	int data = s.oam [index()];
	next();
	return data;
}
