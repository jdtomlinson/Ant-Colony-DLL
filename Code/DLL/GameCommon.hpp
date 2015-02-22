#ifndef include_GameCommon
#define include_GameCommon

//-----------------------------------------------------------------------------------------------
#include <map>
#include "ShortVector2.hpp"
#include "CommonInterface.hpp"


//-----------------------------------------------------------------------------------------------
extern int g_numScouts;
extern int g_numWorkers;
extern int g_numSoldiers;
extern int g_currentTurnNumber;
extern int g_currentNutrients;
extern short g_queenLastKnownPosX;
extern short g_queenLastKnownPosY;
extern ShortVector2 g_arenaDimension;
extern ArenaSquares g_arenaSquaresMemory;
extern CRITICAL_SECTION g_cs;
extern std::map< int, ShortVector2 > g_enemyQueenPositions;


//-----------------------------------------------------------------------------------------------
short ShortClamp( short val, short min, short max );
int GetAntCountByType( EntityType antType );
int GetTotalAntCount();
void IncrementAntTypeCountByOne( EntityType antType );
void DecrementAntTypeCountByOne( EntityType antType );


#endif // include_GameCommon