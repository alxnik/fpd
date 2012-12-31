#include "fpd.h"

string int2string(uint8_t num)
{
	stringstream strstm;
	unsigned int NumCast = num;
	strstm <<  NumCast;
	return strstm.str();
}
string int2string(uint16_t num)
{
	stringstream strstm;
	unsigned int NumCast = num;
	strstm <<  NumCast;
	return strstm.str();
}
string int2string(uint32_t num)
{
	stringstream strstm;
	unsigned int NumCast = num;
	strstm <<  NumCast;
	return strstm.str();
}
string int2string(uint64_t num)
{
	stringstream strstm;
	strstm << num;
	return strstm.str();
}
string int2string(int8_t num)
{
	stringstream strstm;
	int NumCast = num;
	strstm <<  NumCast;
	return strstm.str();
}
string int2string(int16_t num)
{
	stringstream strstm;
	int NumCast = num;
	strstm <<  NumCast;
	return strstm.str();
}
string int2string(int32_t num)
{
	stringstream strstm;
	strstm << num;
	return strstm.str();
}
string int2string(int64_t num)
{
	stringstream strstm;
	strstm << num;
	return strstm.str();
}

#if __WORDSIZE != 64
string int2string(time_t num)
{
	stringstream strstm;
	strstm << num;
	return strstm.str();
}
#endif

string float2string(float num)
{
	stringstream strstm;
	strstm << num;
	return strstm.str();
}
