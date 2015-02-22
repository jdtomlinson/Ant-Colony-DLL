#ifndef include_ShortVector2
#define include_ShortVector2

//-----------------------------------------------------------------------------------------------
struct ShortVector2
{
	ShortVector2() : x( 0 ), y( 0 ) {}
	ShortVector2( short initialX, short initialY ) : x( initialX ), y( initialY ) {}

	short	x;
	short	y;
};


#endif // include_ShortVector2