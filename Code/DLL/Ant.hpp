#ifndef include_Ant
#define include_Ant

//-----------------------------------------------------------------------------------------------
#include <vector>
#include "AStarNode.hpp"
#include "GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
const int NUM_RANDOM_DISTANCES = 20;
const int NUM_A_STAR_LOOPS_BEFORE_GIVE_UP = 2000;
const int NUM_TURNS_TO_WAIT_BEFORE_QUEEN_MAKES_ANT = 5;
const int MIN_ARENA_SIZE_TO_MAKE_SCOUTS = 50;
const int MIN_NUTRIENTS_TO_MAKE_ANTS = 5000;
const int NUM_SCOUT_ANTS_DESIRED_BEFORE_SUDDEN_DEATH = 5;
const int NUM_WORKER_ANTS_DESIRED_BEFORE_SUDDEN_DEATH = 60;
const int NUM_SOLDIER_ANTS_DESIRED_BEFORE_SUDDEN_DEATH = 34;
const int NUM_SCOUT_ANTS_DESIRED_AFTER_SUDDEN_DEATH = 0;
const int NUM_WORKER_ANTS_DESIRED_AFTER_SUDDEN_DEATH = 10;
const int NUM_SOLDIER_ANTS_DESIRED_AFTER_SUDDEN_DEATH = 30;
const short FOOD_SEARCH_RADIUS = 15;
const short MAX_RANDOM_DISTANCE_X_AXIS = 75;
const short MAX_RANDOM_DISTANCE_Y_AXIS = 75;


//-----------------------------------------------------------------------------------------------
class Ant
{
public:
	Ant( int antID, char antType, const ShortVector2& currentPosition );
	void UpdateCurrentPath();
	OrderCode GetCurrentPathOrderCode();
	OrderCode CalculateAndGetOrderCode();

	int								m_id;
	char							m_type;
	char							m_specialStatus;
	char							m_reportCode;
	ShortVector2					m_position;
	std::vector< ShortVector2 >		m_path;

private:
	OrderCode GetQueenAntOrder();
	OrderCode GetScoutAntOrder();
	OrderCode GetWorkerAntOrder();
	OrderCode GetSoldierAntOrder();
	OrderCode GetMovementOrderFromNextNode( const ShortVector2& nextNode );
	float GetManhattanDistance( const ShortVector2& start, const ShortVector2& goal );
	ShortVector2 GetRandomPositionFarthest();
	void UpdatePathForBlockingNodes();
	void DeleteNodes( std::vector< AStarNode* > nodeList );
	std::vector< ShortVector2 > GetPathToClosestFood();
	std::vector< ShortVector2 > ReconstructPath( AStarNode* node );
	std::vector< ShortVector2 > GetAStarPath( short startX, short startY, short goalX, short goalY );
};


#endif // include_Ant