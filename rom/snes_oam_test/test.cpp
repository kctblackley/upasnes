// Tests oam.c using conversion of test suite from asm

#include "oam.c"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int test_code;
static const char* test_text;

void set_test( int code, const char text [] )
{
	test_code = code;
	test_text = text;
}

void test_failed()
{
	printf( "\n" );
	printf( "%s\n", test_text );
	printf( "Failed %d\n", test_code );
	exit( EXIT_FAILURE );
}

void clear_oam()
{
	memset( &s, 0, sizeof s );
}

void fill_oam()
{
	clear_oam();
	
	for ( int i = 0; i < 544; i++ )
		s.oam [i] = i;
}

void check_oam( int addr, const char data [], int size )
{
	for ( int i = 0; i < size; i++ )
		if ( s.oam [addr*2 + i] != (unsigned char) data [i] )
			goto different;
	return;

different:
	printf( "Wrong: " );
	for ( int i = 0; i < size; i++ )
		printf( "%02X ", s.oam [addr*2 + i] );
	
	printf( "\nRight: " );
	for ( int i = 0; i < size; i++ )
		printf( "%02X ", (unsigned char) data [i] );
	
	test_failed();
}

#define write_OAMADDL       write_2102
#define write_OAMADDH       write_2103
#define write_OAMDATA       write_2104
#define read_OAMDATAREAD    read_2138

void writew_OAMADDL( int addr )
{
	write_OAMADDL( addr & 0xFF );
	write_OAMADDH( addr >> 8 );
}

void test_low()
{
	printf( "test_low\n\n" );
	
	int a;
	{
	set_test( 2,"Organized as words, not bytes" );
	clear_oam();
	writew_OAMADDL( 1 );
	write_OAMDATA( 0x12 );
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x00,0x00,0x12,0x34,0x00,0x00}; check_oam( 0, data, sizeof data );}
	
	set_test( 3,"Read of even byte toggles flag" );
	clear_oam();
	writew_OAMADDL( 1 );
	read_OAMDATAREAD();
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x00,0x00,0x00,0x34,0x00,0x00}; check_oam( 0, data, sizeof data );}
	
	set_test( 4,"First write doesn't affect OAM" );
	clear_oam();
	writew_OAMADDL( 1 );
	write_OAMDATA( 0x12 );
	{ char data [] =  {0x00,0x00,0x00,0x00,0x00,0x00}; check_oam( 0, data, sizeof data );}
	
	set_test( 5,"Setting address clears flag" );
	clear_oam();
	writew_OAMADDL( 1 );
	write_OAMDATA( 0x12 );
	writew_OAMADDL( 1 );
	write_OAMDATA( 0x34 );
	write_OAMDATA( 0x56 );
	{ char data [] =  {0x00,0x00,0x34,0x56,0x00,0x00}; check_oam( 0, data, sizeof data );}
	
	set_test( 6,"First write sets buffer, second writes word" );
	clear_oam();
	writew_OAMADDL( 0 );
	write_OAMDATA( 0x12 );
	writew_OAMADDL( 1 );
	read_OAMDATAREAD();
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x00,0x00,0x12,0x34,0x00,0x00}; check_oam( 0, data, sizeof data );}
	
	set_test( 7,"Buffer keeps contents after write" );
	clear_oam();
	writew_OAMADDL( 0 );
	write_OAMDATA( 0x12 );
	write_OAMDATA( 0x34 );
	read_OAMDATAREAD();
	write_OAMDATA( 0x56 );
	read_OAMDATAREAD();
	write_OAMDATA( 0x78 );
	{ char data [] =  {0x12,0x34,0x12,0x56,0x12,0x78}; check_oam( 0, data, sizeof data );}
	
	set_test( 8,"Read of odd byte toggles flag and increments addr" );
	clear_oam();
	writew_OAMADDL( 0 );
	write_OAMDATA( 0x12 );
	read_OAMDATAREAD();
	write_OAMDATA( 0x34 );
	write_OAMDATA( 0x56 );
	{ char data [] =  {0x00,0x00,0x34,0x56,0x00,0x00}; check_oam( 0, data, sizeof data );}
	
	set_test( 9,"Reading doesn't affect buffer" );
	clear_oam();
	writew_OAMADDL( 0 );
	write_OAMDATA( 0x12 );
	read_OAMDATAREAD();
	read_OAMDATAREAD();
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x00,0x00,0x12,0x34,0x00,0x00}; check_oam( 0, data, sizeof data );}
	
	set_test( 10,"Low byte read is unbuffered" );
	fill_oam();
	writew_OAMADDL( 1 );
	a = read_OAMDATAREAD();
	a ^= 0x02;
	if ( a ) test_failed();
	
	set_test( 11,"High byte read is unbuffered" );
	fill_oam();
	writew_OAMADDL( 1 );
	read_OAMDATAREAD();
	a = read_OAMDATAREAD();
	a ^= 0x03;
	if ( a ) test_failed();
	
	set_test( 12,"Writing low byte makes next read get high byte" );
	fill_oam();
	writew_OAMADDL( 1 );
	write_OAMDATA( 0x12 );
	a = read_OAMDATAREAD();
	a ^= 0x03;
	if ( a ) test_failed();
	
	set_test( 13,"Writing low byte of addr resets flag" );
	clear_oam();
	writew_OAMADDL( 1 );
	write_OAMDATA( 0x12 );
	write_OAMADDL( 1 );
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x00,0x00,0x00,0x00,0x00,0x00}; check_oam( 0, data, sizeof data );}
	
	set_test( 14,"Writing high byte of addr resets flag" );
	clear_oam();
	writew_OAMADDL( 1 );
	write_OAMDATA( 0x12 );
	write_OAMADDH( 0 );
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x00,0x00,0x00,0x00,0x00,0x00}; check_oam( 0, data, sizeof data );}
	
	}
}

void test_high()
{
	printf( "test_high\n\n" );
	
	int a;
	{
	set_test( 2,"Organized as words, not bytes" );
	clear_oam();
	writew_OAMADDL( 0x101 );
	write_OAMDATA( 0x12 );
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x00,0x00,0x12,0x34,0x00,0x00}; check_oam( 0x100, data, sizeof data );}
	
	set_test( 3,"Read of even byte toggles flag" );
	clear_oam();
	writew_OAMADDL( 0x101 );
	read_OAMDATAREAD();
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x00,0x00,0x00,0x34,0x00,0x00}; check_oam( 0x100, data, sizeof data );}
	
	set_test( 4,"First write affects OAM" );
	clear_oam();
	writew_OAMADDL( 0x101 );
	write_OAMDATA( 0x12 );
	{ char data [] =  {0x00,0x00,0x12,0x00,0x00,0x00}; check_oam( 0x100, data, sizeof data );}
	
	set_test( 5,"First write sets buffer" );
	clear_oam();
	writew_OAMADDL( 0x101 );
	write_OAMDATA( 0x12 );
	writew_OAMADDL( 0 );
	read_OAMDATAREAD();
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x12,0x34}; check_oam( 0, data, sizeof data );}
	
	set_test( 6,"Setting address clears flag" );
	clear_oam();
	writew_OAMADDL( 0x101 );
	write_OAMDATA( 0x12 );
	writew_OAMADDL( 0x101 );
	write_OAMDATA( 0x34 );
	write_OAMDATA( 0x56 );
	{ char data [] =  {0x00,0x00,0x34,0x56,0x00,0x00}; check_oam( 0x100, data, sizeof data );}
	
	set_test( 7,"Write of odd byte just writes byte, not word" );
	clear_oam();
	writew_OAMADDL( 0x100 );
	write_OAMDATA( 0x12 );
	writew_OAMADDL( 0x101 );
	read_OAMDATAREAD();
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x12,0x00,0x00,0x34,0x00,0x00}; check_oam( 0x100, data, sizeof data );}
	
	set_test( 8,"Buffer keeps contents after odd write" );
	clear_oam();
	writew_OAMADDL( 0x101 );
	write_OAMDATA( 0x12 );
	write_OAMDATA( 0x34 );
	writew_OAMADDL( 0 );
	read_OAMDATAREAD();
	write_OAMDATA( 0x56 );
	{ char data [] =  {0x12,0x56}; check_oam( 0, data, sizeof data );}

	set_test( 9,"Read of odd byte toggles flag and increments addr" );
	clear_oam();
	writew_OAMADDL( 0x100 );
	write_OAMDATA( 0x12 );
	read_OAMDATAREAD();
	write_OAMDATA( 0x34 );
	write_OAMDATA( 0x56 );
	{ char data [] =  {0x12,0x00,0x34,0x56,0x00,0x00}; check_oam( 0x100, data, sizeof data );}
	
	set_test( 10,"Reading doesn't affect buffer" );
	clear_oam();
	writew_OAMADDL( 0x100 );
	write_OAMDATA( 0x12 );
	read_OAMDATAREAD();
	read_OAMDATAREAD();
	writew_OAMADDL( 0 );
	read_OAMDATAREAD();
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x12,0x34}; check_oam( 0, data, sizeof data );}
	
	set_test( 11,"Low byte read is unbuffered" );
	fill_oam();
	writew_OAMADDL( 0x101 );
	a = read_OAMDATAREAD();
	a ^= 0x02;
	if ( a ) test_failed();
	
	set_test( 12,"High byte read is unbuffered" );
	fill_oam();
	writew_OAMADDL( 0x101 );
	read_OAMDATAREAD();
	a = read_OAMDATAREAD();
	a ^= 0x03;
	if ( a ) test_failed();
	
	set_test( 13,"Writing low byte makes next read get high byte" );
	fill_oam();
	writew_OAMADDL( 0x101 );
	write_OAMDATA( 0x12 );
	a = read_OAMDATAREAD();
	a ^= 0x03;
	if ( a ) test_failed();

	set_test( 14,"First write to low bank doesn't affect addr" );
	clear_oam();
	writew_OAMADDL( 1 );
	write_OAMDATA( 0x12 );
	write_OAMADDH( 1 );
	write_OAMDATA( 0x34 );
	{ char data [] =  {0x00,0x00,0x34,0x00,0x00,0x00}; check_oam( 0x100, data, sizeof data );}
	
	set_test( 15,"High byte of addr incremented when low wraps" );
	clear_oam();
	writew_OAMADDL( 0xFF );
	write_OAMDATA( 0x12 );
	write_OAMDATA( 0x34 );
	write_OAMDATA( 0x56 );
	{ char data [] =  {0x12,0x34,0x56,0x00}; check_oam( 0xFF, data, sizeof data );}
	
	set_test( 16,"Upper bits of OAMADDH ignored" );
	clear_oam();
	writew_OAMADDL( 0xFE00 );
	write_OAMDATA( 0x12 );
	write_OAMDATA( 0x34 );
	writew_OAMADDL( 0xFF00 );
	write_OAMDATA( 0x56 );
	{ char data [] =  {0x12,0x34}; check_oam( 0, data, sizeof data );}
	{ char data [] =  {0x56,0x00}; check_oam( 0x100, data, sizeof data );}
	}
}

int main()
{
	test_low();
	test_high();
	printf( "Passed\n" );
}
