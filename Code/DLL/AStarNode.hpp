#ifndef include_AStarNode
#define include_AStarNode

//-----------------------------------------------------------------------------------------------
struct AStarNode
{
	AStarNode( short x, short y ) : nodeCameFrom( nullptr ), coordX( x ), coordY( y ), fValue( 0.f ), gValue( 0.f ), hValue( 0.f ) {}

	AStarNode* nodeCameFrom;
	short coordX;
	short coordY;
	float fValue;
	float gValue;
	float hValue;
};


#endif // include_AStarNode