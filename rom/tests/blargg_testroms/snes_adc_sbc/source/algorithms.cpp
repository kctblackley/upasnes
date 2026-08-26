// These assume that host machine uses two's complement

int const n80 = 0x80;
int const v40 = 0x40;
int const d08 = 0x08;
int const z02 = 0x02;
int const c01 = 0x01;

int adc8( int a, int& p, int operand )
{
	int carry = (p & c01) != 0;
	
	p &= ~(n80 | v40 | z02 | c01);
	
	int result;
	
	if ( !(p & d08) )
	{
		result = a + operand + carry;
	}
	else
	{
		result = (a & 0x0F) + (operand & 0x0F) + carry;
		if ( result > 9 )
			result += 6;
		
		carry = (result > 0x0F);
		result = (a & 0xF0) + (operand & 0xF0) + (result & 0x0F) + (carry * 0x10);
	}
	
	// signs of a and operand match, and sign of result doesn't
	if ( (a & 0x80) == (operand & 0x80) && (a & 0x80) != (result & 0x80) )
		p |= v40;
	
	if ( p & d08 && result > 0x9F )
		result += 0x60;
	
	if ( result > 0xFF )
		p |= c01;
	
	if ( result & 0x80 )
		p |= n80;
	
	if ( (result & 0xFF) == 0 )
		p |= z02;
	
	return result & 0xFF;
}

int sbc8( int a, int& p, int operand )
{
	int carry = (p & c01) != 0;
	
	p &= ~(n80 | v40 | z02 | c01);
	
	int result;
	
	operand ^= 0xFF;
	
	if ( !(p & d08) )
	{
		result = a + operand + carry;
	}
	else
	{
		result = (a & 0x0F) + (operand & 0x0F) + carry;
		if ( result < 0x10 )
			result -= 6;
		
		carry = (result > 0x0F);
		result = (a & 0xF0) + (operand & 0xF0) + (result & 0x0F) + (carry * 0x10);
	}
	
	// signs of a and operand match, and sign of result doesn't
	if ( (a & 0x80) == (operand & 0x80) && (a & 0x80) != (result & 0x80) )
		p |= v40;
	
	if ( p & d08 && result < 0x100 )
		result -= 0x60;
	
	if ( result > 0xFF )
		p |= c01;
	
	if ( result & 0x80 )
		p |= n80;
	
	if ( (result & 0xFF) == 0 )
		p |= z02;
	
	return result & 0xFF;
}

int adc16( int a, int& p, int operand )
{
	int carry = (p & c01) != 0;
	
	p &= ~(n80 | v40 | z02 | c01);
	
	int result;
	
	if ( !(p & d08) )
	{
		result = a + operand + carry;
	}
	else
	{
		result = (a & 0x000F) + (operand & 0x000F) + carry;
		if ( result > 0x0009 )
			result += 0x0006;
		
		carry = (result > 0x000F);
		
		result = (a & 0x00F0) + (operand & 0x00F0) + (result & 0x000F) + carry * 0x10;
		if ( result > 0x009F )
			result += 0x0060;

		carry = (result > 0x00FF);
	
		result = (a & 0x0F00) + (operand & 0x0F00) + (result & 0x00FF) + carry * 0x100;
		if ( result > 0x09FF )
			result += 0x0600;
		
		carry = (result > 0x0FFF);
		
		result = (a & 0xF000) + (operand & 0xF000) + (result & 0x0FFF) + carry * 0x1000;
	}
	
	// signs of a and operand match, and sign of result doesn't
	if ( (a & 0x8000) == (operand & 0x8000) && (a & 0x8000) != (result & 0x8000) )
		p |= v40;
	
	if ( p & d08 && result > 0x9FFF )
		result += 0x6000;
	
	if ( result > 0xFFFF )
		p |= c01;
	
	if ( result & 0x8000 )
		p |= n80;
	
	if ( (result & 0xFFFF) == 0 )
		p |= z02;
	
	return result & 0xFFFF;
}

int sbc16( int a, int& p, int operand )
{
	int carry = (p & c01) != 0;
	
	p &= ~(n80 | v40 | z02 | c01);
	
	int result;
	
	operand ^= 0xFFFF;
	
	if ( !(p & d08) )
	{
		result = a + operand + carry;
	}
	else
	{
		result = (a & 0x000F) + (operand & 0x000F) + carry;
		if ( result < 0x0010 )
			result -= 0x0006;
		
		carry = (result > 0x000F);
		
		result = (a & 0x00F0) + (operand & 0x00F0) + (result & 0x000F) + carry * 0x10;
		if ( result < 0x0100 )
			result -= 0x0060;

		carry = (result > 0x00FF);
	
		result = (a & 0x0F00) + (operand & 0x0F00) + (result & 0x00FF) + carry * 0x100;
		if ( result < 0x1000 )
			result -= 0x0600;
		
		carry = (result > 0x0FFF);
		
		result = (a & 0xF000) + (operand & 0xF000) + (result & 0x0FFF) + carry * 0x1000;
	}
	
	// signs of addends match, and sign of result doesn't
	if ( ((a ^ operand) & 0x8000) == 0 && ((a ^ result) & 0x8000) )
		p |= v40;
	
	if ( p & d08 && result < 0x10000 )
		result -= 0x6000;
	
	if ( result > 0xFFFF )
		p |= c01;
	
	if ( result & 0x8000 )
		p |= n80;
	
	if ( (result & 0xFFFF) == 0 )
		p |= z02;
	
	return result & 0xFFFF;
}
