#include "GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
int g_numScouts;
int g_numWorkers;
int g_numSoldiers;
int g_currentTurnNumber;
int g_currentNutrients;
short g_queenLastKnownPosX;
short g_queenLastKnownPosY;
ShortVector2 g_arenaDimension;
ArenaSquares g_arenaSquaresMemory;
CRITICAL_SECTION g_cs;
std::map< int, ShortVector2 > g_enemyQueenPositions;


//-----------------------------------------------------------------------------------------------
short ShortClamp( short val, short min, short max )
{
	if( val < min )
		return min;

	if( val > max )
		return max;

	return val;
}


//-----------------------------------------------------------------------------------------------
int GetAntCountByType( EntityType antType )
{
	int numAnts = 0;

	EnterCriticalSection( &g_cs );

	if( antType == ENTITY_TYPE_SCOUT )
		numAnts = g_numScouts;
	else if( antType == ENTITY_TYPE_WORKER )
		numAnts = g_numWorkers;
	else if( antType == ENTITY_TYPE_SOLDIER )
		numAnts = g_numSoldiers;

	LeaveCriticalSection( &g_cs );

	return numAnts;
}


//-----------------------------------------------------------------------------------------------
int GetTotalAntCount()
{
	EnterCriticalSection( &g_cs );
	int scoutCount = g_numScouts;
	int workerCount = g_numWorkers;
	int soldierCount = g_numSoldiers;
	LeaveCriticalSection( &g_cs );

	return 1 + scoutCount + workerCount + soldierCount;
}


//-----------------------------------------------------------------------------------------------
void IncrementAntTypeCountByOne( EntityType antType )
{
	EnterCriticalSection( &g_cs );

	if( antType == ENTITY_TYPE_SCOUT )
		++g_numScouts;
	else if( antType == ENTITY_TYPE_WORKER )
		++g_numWorkers;
	else if( antType == ENTITY_TYPE_SOLDIER )
		++g_numSoldiers;

	LeaveCriticalSection( &g_cs );
}


//-----------------------------------------------------------------------------------------------
void DecrementAntTypeCountByOne( EntityType antType )
{
	EnterCriticalSection( &g_cs );

	if( antType == ENTITY_TYPE_SCOUT && g_numScouts > 0 )
		--g_numScouts;
	else if( antType == ENTITY_TYPE_WORKER && g_numWorkers > 0 )
		--g_numWorkers;
	else if( antType == ENTITY_TYPE_SOLDIER && g_numSoldiers > 0 )
		--g_numSoldiers;

	LeaveCriticalSection( &g_cs );
}