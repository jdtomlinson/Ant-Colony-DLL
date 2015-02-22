#include "Ant.hpp"


//-----------------------------------------------------------------------------------------------
Ant::Ant( int antID, char antType, const ShortVector2& currentPosition )
	: m_id( antID )
	, m_type( antType )
	, m_position( currentPosition )
{

}


//-----------------------------------------------------------------------------------------------
void Ant::UpdateCurrentPath()
{
	if( m_path.size() > 0 && m_reportCode == REPORT_MOVE_SUCCESSFUL )
	{
		m_path.erase( m_path.begin() );
	}
}


//-----------------------------------------------------------------------------------------------
OrderCode Ant::GetCurrentPathOrderCode()
{
	if( m_path.size() > 0 )
	{
		return GetMovementOrderFromNextNode( m_path[0] );
	}

	if( m_type == ENTITY_TYPE_WORKER && m_path.size() == 0 )
	{
		EnterCriticalSection( &g_cs );
		ArenaSquares tempSquaresMemory = g_arenaSquaresMemory;
		LeaveCriticalSection( &g_cs );

		if( m_specialStatus == ENTITY_SPECIAL_STATUS_CARRYING_FOOD && m_reportCode == REPORT_MOVE_SUCCESSFUL )
		{
			return ORDER_DROP_FOOD;
		}

		ArenaSquareType squareType = tempSquaresMemory.GetSquareTypeAtCoords( m_position.x, m_position.y );
		if( squareType == ARENA_SQUARE_TYPE_FOOD )
		{
			return ORDER_TAKE_FOOD;
		}
	}

	return ORDER_HOLD;
}


//-----------------------------------------------------------------------------------------------
OrderCode Ant::CalculateAndGetOrderCode()
{
	if( m_type == ENTITY_TYPE_QUEEN )
		return GetQueenAntOrder();

	if( m_type == ENTITY_TYPE_SCOUT )
		return GetScoutAntOrder();
	
	if( m_type == ENTITY_TYPE_WORKER )
		return GetWorkerAntOrder();
	
	if( m_type == ENTITY_TYPE_SOLDIER )
		return GetSoldierAntOrder();

	return ORDER_HOLD;
}


//-----------------------------------------------------------------------------------------------
OrderCode Ant::GetQueenAntOrder()
{
	EnterCriticalSection( &g_cs );
	g_queenLastKnownPosX = m_position.x;
	g_queenLastKnownPosY = m_position.y;
	int currentNutrients = g_currentNutrients;
	int turnNumber = g_currentTurnNumber;
	LeaveCriticalSection( &g_cs );

	if( m_specialStatus == ENTITY_SPECIAL_STATUS_EXHAUSTED || turnNumber % NUM_TURNS_TO_WAIT_BEFORE_QUEEN_MAKES_ANT != 0 )
		return ORDER_HOLD;

	int totalNumAnts = GetTotalAntCount();
	if( totalNumAnts < 100 && currentNutrients > MIN_NUTRIENTS_TO_MAKE_ANTS )
	{
		if( turnNumber < 1000 )
		{
			if( g_arenaDimension.x > MIN_ARENA_SIZE_TO_MAKE_SCOUTS && GetAntCountByType( ENTITY_TYPE_SCOUT ) < NUM_SCOUT_ANTS_DESIRED_BEFORE_SUDDEN_DEATH )
			{
				IncrementAntTypeCountByOne( ENTITY_TYPE_SCOUT );
				return ORDER_CREATE_SCOUT;
			}

			if( GetAntCountByType( ENTITY_TYPE_WORKER ) < NUM_WORKER_ANTS_DESIRED_BEFORE_SUDDEN_DEATH )
			{
				IncrementAntTypeCountByOne( ENTITY_TYPE_WORKER );
				return ORDER_CREATE_WORKER;
			}

			if( GetAntCountByType( ENTITY_TYPE_SOLDIER ) < NUM_SOLDIER_ANTS_DESIRED_BEFORE_SUDDEN_DEATH )
			{
				IncrementAntTypeCountByOne( ENTITY_TYPE_SOLDIER );
				return ORDER_CREATE_SOLDIER;
			}
		}
		else
		{
			if( GetAntCountByType( ENTITY_TYPE_SOLDIER ) < NUM_SOLDIER_ANTS_DESIRED_AFTER_SUDDEN_DEATH )
			{
				IncrementAntTypeCountByOne( ENTITY_TYPE_SOLDIER );
				return ORDER_CREATE_SOLDIER;
			}

			if( GetAntCountByType( ENTITY_TYPE_WORKER ) < NUM_WORKER_ANTS_DESIRED_AFTER_SUDDEN_DEATH )
			{
				IncrementAntTypeCountByOne( ENTITY_TYPE_WORKER );
				return ORDER_CREATE_WORKER;
			}

			if( GetAntCountByType( ENTITY_TYPE_SCOUT ) < NUM_SCOUT_ANTS_DESIRED_AFTER_SUDDEN_DEATH )
			{
				IncrementAntTypeCountByOne( ENTITY_TYPE_SCOUT );
				return ORDER_CREATE_SCOUT;
			}
		}
	}

	return ORDER_HOLD;
}


//-----------------------------------------------------------------------------------------------
OrderCode Ant::GetScoutAntOrder()
{
	UpdatePathForBlockingNodes();

	if( m_path.size() == 0 )
	{
		ShortVector2 randomPosition = GetRandomPositionFarthest();
		m_path = GetAStarPath( m_position.x, m_position.y, randomPosition.x, randomPosition.y );
	}

	if( m_path.size() > 0 )
	{
		return GetMovementOrderFromNextNode( m_path[0] );
	}

	return ORDER_HOLD;
}


//-----------------------------------------------------------------------------------------------
OrderCode Ant::GetWorkerAntOrder()
{
	EnterCriticalSection( &g_cs );
	ArenaSquares tempSquaresMemory = g_arenaSquaresMemory;
	short queenPosX = g_queenLastKnownPosX;
	short queenPosY = g_queenLastKnownPosY;
	LeaveCriticalSection( &g_cs );

	UpdatePathForBlockingNodes();

	if( m_path.size() > 0 )
	{
		ShortVector2 finalNode = m_path.back();
		if( m_specialStatus != ENTITY_SPECIAL_STATUS_CARRYING_FOOD && tempSquaresMemory.GetSquareTypeAtCoords( finalNode.x, finalNode.y ) != ARENA_SQUARE_TYPE_FOOD )
		{
			std::vector< ShortVector2 > foodPath = GetPathToClosestFood();
			if( foodPath.size() > 0 )
			{
				m_path = foodPath;
			}
		}
	}

	if( m_specialStatus == ENTITY_SPECIAL_STATUS_CARRYING_FOOD )
	{
		if( m_path.size() == 0 )
		{
			if( m_reportCode == REPORT_MOVE_SUCCESSFUL )
				return ORDER_DROP_FOOD;

			m_path = GetAStarPath( m_position.x, m_position.y, queenPosX, queenPosY );
		}	

		if( m_path.size() > 0 )
		{
			ShortVector2 nextNode = m_path[0];
			return GetMovementOrderFromNextNode( nextNode );
		}	
	}

	ArenaSquareType squareType = tempSquaresMemory.GetSquareTypeAtCoords( m_position.x, m_position.y );
	if( squareType == ARENA_SQUARE_TYPE_FOOD )
	{
		return ORDER_TAKE_FOOD;
	}

	if( m_path.size() == 0 )
	{
		std::vector< ShortVector2 > foodPath = GetPathToClosestFood();
		if( foodPath.size() > 0 )
		{
			m_path = foodPath;
		}
		else
		{
			ShortVector2 randomPosition = GetRandomPositionFarthest();
			m_path = GetAStarPath( m_position.x, m_position.y, randomPosition.x, randomPosition.y );
		}
	}

	if( m_path.size() > 0 )
	{
		return GetMovementOrderFromNextNode( m_path[0] );
	}

	return ORDER_HOLD;
}


//-----------------------------------------------------------------------------------------------
OrderCode Ant::GetSoldierAntOrder()
{
	EnterCriticalSection( &g_cs );
	std::map< int, ShortVector2 > tempQueenPositions = g_enemyQueenPositions;
	LeaveCriticalSection( &g_cs );

	UpdatePathForBlockingNodes();

	ShortVector2 positionToAttack;
	float distanceToEntity = NUM_A_STAR_LOOPS_BEFORE_GIVE_UP;

	std::map< int, ShortVector2 >::iterator queenIter;
	for( queenIter = tempQueenPositions.begin(); queenIter != tempQueenPositions.end(); ++queenIter )
	{
		ShortVector2 entityPosition = queenIter->second;
		float dist = GetManhattanDistance( m_position, entityPosition );
		if( dist < distanceToEntity )
		{
			positionToAttack = entityPosition;
			distanceToEntity = dist;
		}
	}

	ShortVector2 goToPosition;
	if( distanceToEntity == NUM_A_STAR_LOOPS_BEFORE_GIVE_UP && m_path.size() == 0 )
	{
		ShortVector2 randomPosition = GetRandomPositionFarthest();
		m_path = GetAStarPath( m_position.x, m_position.y, randomPosition.x, randomPosition.y );
	}
	else if( distanceToEntity > 0.f )
	{
		if( m_path.size() > 0 )
		{
			ShortVector2 lastNode = m_path.back();
			if( lastNode.x != positionToAttack.x || lastNode.y != positionToAttack.y )
			{
				m_path = GetAStarPath( m_position.x, m_position.y, positionToAttack.x, positionToAttack.y );
			}
		}
		else
		{
			m_path = GetAStarPath( m_position.x, m_position.y, positionToAttack.x, positionToAttack.y );
		}
	}

	if( m_path.size() > 0 )
	{
		return GetMovementOrderFromNextNode( m_path[0] );
	}

	return ORDER_HOLD;
}


//-----------------------------------------------------------------------------------------------
OrderCode Ant::GetMovementOrderFromNextNode( const ShortVector2& nextNode )
{
	if( m_position.x == nextNode.x - 1 )
		return ORDER_MOVE_EAST;

	if( m_position.x == nextNode.x + 1 )
		return ORDER_MOVE_WEST ;

	if( m_position.y == nextNode.y - 1 )
		return ORDER_MOVE_SOUTH;

	if( m_position.y == nextNode.y + 1 )
		return ORDER_MOVE_NORTH;

	return ORDER_HOLD;
}


//-----------------------------------------------------------------------------------------------
float Ant::GetManhattanDistance( const ShortVector2& start, const ShortVector2& goal )
{
	return (float) ( abs( start.x - goal.x ) + abs( start.y - goal.y ) );
}


//-----------------------------------------------------------------------------------------------
ShortVector2 Ant::GetRandomPositionFarthest()
{
	ShortVector2 farthestPosition = m_position;
	float longestDistance = 0.f;
	for( int squareIndex = 0; squareIndex < NUM_RANDOM_DISTANCES; ++squareIndex )
	{
		ShortVector2 randomPosition;
		short arenaSubWidth = MAX_RANDOM_DISTANCE_X_AXIS;
		short arenaSubHeight = MAX_RANDOM_DISTANCE_Y_AXIS;

		if( g_arenaDimension.x < MAX_RANDOM_DISTANCE_X_AXIS )
			arenaSubWidth = g_arenaDimension.x;

		if( g_arenaDimension.y < MAX_RANDOM_DISTANCE_Y_AXIS )
			arenaSubWidth = g_arenaDimension.y;
		
		randomPosition.x = ShortClamp( ( rand() % arenaSubWidth ) - (short) ( arenaSubWidth * 0.5f ) + m_position.x, 1, g_arenaDimension.x - 1 );
		randomPosition.y = ShortClamp( ( rand() % arenaSubHeight ) - (short) ( arenaSubHeight * 0.5f ) + m_position.y, 1, g_arenaDimension.y - 1 );
		float dist = GetManhattanDistance( m_position, randomPosition );
		if( dist > longestDistance )
		{
			farthestPosition = randomPosition;
			longestDistance = dist;
		}
	}

	return farthestPosition;
}


//-----------------------------------------------------------------------------------------------
void Ant::UpdatePathForBlockingNodes()
{
	EnterCriticalSection( &g_cs );
	ArenaSquares tempSquaresMemory = g_arenaSquaresMemory;
	LeaveCriticalSection( &g_cs );

	for( unsigned int nodeIndex = 0; nodeIndex < m_path.size(); ++nodeIndex )
	{
		ShortVector2 node = m_path[ nodeIndex ];
		ArenaSquareType squareType = tempSquaresMemory.GetSquareTypeAtCoords( node.x, node.y );
		if( squareType == ARENA_SQUARE_TYPE_STONE )
		{
			ShortVector2 finalNode = m_path.back();
			if( node.x == finalNode.x && node.y == finalNode.y )
				m_path.clear();
			else
				m_path = GetAStarPath( m_position.x, m_position.y, finalNode.x, finalNode.y );

			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Ant::DeleteNodes( std::vector< AStarNode* > nodeList )
{
	while( nodeList.size() != 0 )
	{
		AStarNode* node = nodeList.back();
		nodeList.pop_back();
		delete node;
	}
}


//-----------------------------------------------------------------------------------------------
std::vector< ShortVector2 > Ant::GetPathToClosestFood()
{
	EnterCriticalSection( &g_cs );
	ArenaSquares tempSquaresMemory = g_arenaSquaresMemory;
	LeaveCriticalSection( &g_cs );

	bool hasFoundFood = false;
	ShortVector2 foodPosition;
	for( short radius = 1; radius < FOOD_SEARCH_RADIUS; ++radius )
	{
		for( short coordX = -radius; coordX < radius; ++coordX )
		{
			short posCoordY = radius - (short) abs( coordX );
			short negCoordY = -posCoordY;

			ArenaSquareType squareType = tempSquaresMemory.GetSquareTypeAtCoords( m_position.x + coordX, m_position.y + posCoordY );
			if( squareType == ARENA_SQUARE_TYPE_FOOD )
			{
				foodPosition = ShortVector2( m_position.x + coordX, m_position.y + posCoordY );
				hasFoundFood = true;
				break;
			}

			squareType = tempSquaresMemory.GetSquareTypeAtCoords( m_position.x + coordX, m_position.y + negCoordY );
			if( squareType == ARENA_SQUARE_TYPE_FOOD )
			{
				foodPosition = ShortVector2( m_position.x + coordX, m_position.y + negCoordY );
				hasFoundFood = true;
				break;
			}
		}

		if( hasFoundFood )
			break;
	}

	if( !hasFoundFood )
	{
		std::vector< ShortVector2 > emptyVector;
		return emptyVector;
	}

	std::vector< ShortVector2 > path = GetAStarPath( m_position.x, m_position.y, foodPosition.x, foodPosition.y );
	if( path.size() == 0 )
		tempSquaresMemory.SetSquareTypeAtCoords( foodPosition.x, foodPosition.y, ARENA_SQUARE_TYPE_STONE );

	EnterCriticalSection( &g_cs );
	g_arenaSquaresMemory = tempSquaresMemory;
	LeaveCriticalSection( &g_cs );

	return path;
}


//-----------------------------------------------------------------------------------------------
std::vector< ShortVector2 > Ant::ReconstructPath( AStarNode* node )
{
	std::vector< ShortVector2 > path;

	if( node->nodeCameFrom == nullptr )
	{
		return path;
	}

	path = ReconstructPath( node->nodeCameFrom );
	path.push_back( ShortVector2( node->coordX, node->coordY ) );

	return path;
}


//-----------------------------------------------------------------------------------------------
std::vector< ShortVector2 > Ant::GetAStarPath( short startX, short startY, short goalX, short goalY )
{
	EnterCriticalSection( &g_cs );
	ArenaSquares tempSquaresMemory = g_arenaSquaresMemory;
	LeaveCriticalSection( &g_cs );

	std::vector< AStarNode* > openList;
	std::vector< AStarNode* > closeList;

	if( startX == goalX && startY == goalY )
	{
		std::vector< ShortVector2 > emptyVector;
		return emptyVector;
	}

	AStarNode* startNode = new AStarNode( startX, startY );
	startNode->gValue = 0.f;
	startNode->hValue = GetManhattanDistance( ShortVector2( startX, startY ), ShortVector2( goalX, goalY ) );
	startNode->fValue = startNode->gValue + startNode->hValue;
	openList.push_back( startNode );

	int numLoops = 0;

	while( openList.size() != 0 )
	{
		if( numLoops > NUM_A_STAR_LOOPS_BEFORE_GIVE_UP )
		{
			DeleteNodes( openList );
			DeleteNodes( closeList );
			std::vector< ShortVector2 > emptyVector;
			return emptyVector;
		}

		AStarNode* currentNode = openList.front();
		int currentNodeIndex = 0;
		for( unsigned int nodeIndex = 0; nodeIndex < openList.size(); ++nodeIndex )
		{
			AStarNode* node = openList[ nodeIndex ];
			if( node->fValue < currentNode->fValue )
			{
				currentNode = node;
				currentNodeIndex = nodeIndex;
			}
		}

		openList.erase( openList.begin() + currentNodeIndex );
		closeList.push_back( currentNode );

		std::vector< AStarNode > neighbors;
		neighbors.push_back( AStarNode( currentNode->coordX + 1, currentNode->coordY ) );
		neighbors.push_back( AStarNode( currentNode->coordX - 1, currentNode->coordY ) );
		neighbors.push_back( AStarNode( currentNode->coordX, currentNode->coordY + 1 ) );
		neighbors.push_back( AStarNode( currentNode->coordX, currentNode->coordY - 1 ) );

		for( unsigned int nodeIndex = 0; nodeIndex < neighbors.size(); ++nodeIndex )
		{
			AStarNode node = neighbors[ nodeIndex ];

			if( node.coordX == goalX && node.coordY == goalY )
			{
				AStarNode* finalNode = new AStarNode( node.coordX, node.coordY );
				finalNode->nodeCameFrom = currentNode;

				std::vector< ShortVector2 > path = ReconstructPath( finalNode );

				DeleteNodes( openList );
				DeleteNodes( closeList );
				delete finalNode;

				return path;
			}

			ArenaSquareType squareType = tempSquaresMemory.GetSquareTypeAtCoords( node.coordX, node.coordY );
			if( squareType == ARENA_SQUARE_TYPE_STONE )
				continue;

			node.gValue = currentNode->gValue + 1;
			node.hValue = GetManhattanDistance( ShortVector2( node.coordX, node.coordY ), ShortVector2( goalX, goalY ) );
			node.fValue = node.gValue + node.hValue;
			if( squareType == ARENA_SQUARE_TYPE_DIRT && m_type != ENTITY_TYPE_SCOUT )
				node.gValue += 1;

			bool nodeFoundInList = false;
			for ( unsigned int openIndex = 0; openIndex < openList.size(); ++openIndex )
			{
				AStarNode* openNode = openList[ openIndex ];
				if( openNode->coordX == node.coordX && openNode->coordY == node.coordY )
				{
					nodeFoundInList = true;
					break;
				}
			}

			if( !nodeFoundInList )
			{
				for ( unsigned int closeIndex = 0; closeIndex < closeList.size(); ++closeIndex )
				{
					AStarNode* closeNode = closeList[ closeIndex ];
					if( closeNode->coordX == node.coordX && closeNode->coordY == node.coordY )
					{
						nodeFoundInList = true;
						break;
					}
				}
			}

			if( !nodeFoundInList )
			{
				AStarNode* nextNode = new AStarNode( node.coordX, node.coordY );
				nextNode->nodeCameFrom = currentNode;
				nextNode->gValue = node.gValue;
				nextNode->hValue = node.hValue;
				nextNode->fValue = node.fValue;
				openList.push_back( nextNode );
			}
		}

		++numLoops;
	}

	DeleteNodes( closeList );
	std::vector< ShortVector2 > emptyVector;
	return emptyVector;
}