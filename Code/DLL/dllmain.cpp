//-----------------------------------------------------------------------------------------------
#include <map>
#include <vector>
#include <windows.h>
#include "Ant.hpp"
#include "Time.hpp"
#include <gl/gl.h>
#include <gl/glu.h>
#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library
#pragma comment( lib, "glu32" ) // Link in the GLU32.lib static library


//-----------------------------------------------------------------------------------------------
const float MILLISECONDS_FOR_UPDATE_PER_TURN = 0.9f;
const float SECONDS_FOR_UPDATE_PER_TURN = MILLISECONDS_FOR_UPDATE_PER_TURN * 0.001f;


//-----------------------------------------------------------------------------------------------
unsigned int		g_antIndex;
bool				g_isQuitting = false;
bool				g_areUpdatesReady = false;
bool				g_haveResultsReturned = false;
Orders				g_tentativeOrders;
AgentReports		g_agentReportsFromPrevOrders;
Ant*				g_queenAnt;
std::vector< Ant* >	g_ants;


//-----------------------------------------------------------------------------------------------
_declspec( dllexport ) int GetInterfaceVersion()
{
	return COMMON_INTERFACE_VERSION_NUMBER;
}


//-----------------------------------------------------------------------------------------------
_declspec( dllexport ) const char* GetPlayerName()
{
	return "Our New Insect Overlords";
}


//-----------------------------------------------------------------------------------------------
_declspec( dllexport ) const char* GetAuthorName()
{
	return "JD Tomlinson";
}


//-----------------------------------------------------------------------------------------------
_declspec( dllexport ) void PrepareForBattle( int, const ArenaInfo& arenaInfo )
{
	g_arenaDimension = ShortVector2( arenaInfo.width, arenaInfo.height );
	g_antIndex = 0;
	g_numScouts = 3;
	g_numWorkers = 3;
	g_numSoldiers = 3;
	InitializeTime();

	for( short arenaColumns = 0; arenaColumns < g_arenaDimension.x; ++arenaColumns )
	{
		for( short arenaRows = 0; arenaRows < g_arenaDimension.y; ++arenaRows )
		{
			g_arenaSquaresMemory.SetSquareTypeAtCoords( arenaColumns, arenaRows, ARENA_SQUARE_TYPE_UNKNOWN );
		}
	}

	InitializeCriticalSection( &g_cs );
}


//-----------------------------------------------------------------------------------------------
_declspec( dllexport ) void BeginWork()
{
	EnterCriticalSection( &g_cs );
	bool breakLoop = g_isQuitting;
	LeaveCriticalSection( &g_cs );

	while( !breakLoop )
	{
		EnterCriticalSection( &g_cs );
		bool haveRecentResults = g_haveResultsReturned;
		bool areTentativeOrdersEmpty = g_tentativeOrders.m_numberOfOrders == 0;
		bool areResultsEmpty = g_agentReportsFromPrevOrders.m_numberAgentReports == 0;
		LeaveCriticalSection( &g_cs );

		if( !haveRecentResults )
		{
			Sleep( 0 );
		}
		else if( !areTentativeOrdersEmpty || areResultsEmpty )
		{
			EnterCriticalSection( &g_cs );
			g_areUpdatesReady = true;
			LeaveCriticalSection( &g_cs );
			Sleep( 0 );
		}
		else
		{
			EnterCriticalSection( &g_cs );
			int numOrdersToGive = g_agentReportsFromPrevOrders.m_numberAgentReports;
			AgentReports tempReports = g_agentReportsFromPrevOrders;
			std::vector< Ant* > tempAnts = g_ants;
			unsigned int tempAntIndex = g_antIndex;
			g_areUpdatesReady = true;
			LeaveCriticalSection( &g_cs );

			double startTime = GetCurrentTimeSeconds();
			double timeElapsed = 0.0;
			Orders tempOrders;
			unsigned int ordersGiven = 0;

			for( int reportIndex = 0; reportIndex < numOrdersToGive; ++reportIndex )
			{
				const AgentReport& report = tempReports.m_agentReports[ reportIndex ];
				
				Ant* ant = nullptr;
				for( unsigned int antIndex = 0; antIndex < tempAnts.size(); ++antIndex )
				{
					Ant* checkAnt = tempAnts[ antIndex ];
					if( checkAnt->m_id == report.m_entityID )
					{
						if( report.m_specialStatus == ENTITY_SPECIAL_STATUS_DEAD )
						{
							DecrementAntTypeCountByOne( (EntityType) report.m_agentType );

							delete checkAnt;
							tempAnts.erase( tempAnts.begin() + antIndex );
						}
						else
						{
							ant = checkAnt;
							ant->m_reportCode = report.m_reportCode;
							ant->m_specialStatus = report.m_specialStatus;
							ant->m_position = ShortVector2( report.m_newPositionX, report.m_newPositionY );
						}
						
						break;
					}
				}

				if( ant == nullptr && report.m_specialStatus != ENTITY_SPECIAL_STATUS_DEAD )
				{
					ant = new Ant( report.m_entityID, report.m_agentType, ShortVector2( report.m_newPositionX, report.m_newPositionY ) );
					tempAnts.push_back( ant );
					if( ant->m_type == ENTITY_TYPE_QUEEN )
						g_queenAnt = ant;
				}

				if( ant )
				{
					ant->UpdateCurrentPath();
					tempOrders.AddOrder( ant->m_id, ant->GetCurrentPathOrderCode(), true );
				}
			}

			tempOrders.AddOrder( g_queenAnt->m_id, g_queenAnt->CalculateAndGetOrderCode(), true );
			++ordersGiven;

			while( timeElapsed < SECONDS_FOR_UPDATE_PER_TURN )
			{
				if( ordersGiven == tempAnts.size() )
					break;

				++tempAntIndex;
				if( tempAntIndex >= tempAnts.size() )
					tempAntIndex = 0;	

				Ant* ant = tempAnts[ tempAntIndex ];
				if( ant == nullptr )
					continue;
				if( ant == g_queenAnt )
					continue;

				tempOrders.AddOrder( ant->m_id, ant->CalculateAndGetOrderCode(), true );
				++ordersGiven;

				double currentTime = GetCurrentTimeSeconds();
				timeElapsed = currentTime - startTime;
			}

			EnterCriticalSection( &g_cs );

			g_ants = tempAnts;
			g_antIndex = tempAntIndex;
			g_tentativeOrders = tempOrders;

			LeaveCriticalSection( &g_cs );
		}

		EnterCriticalSection( &g_cs );
		breakLoop = g_isQuitting;
		LeaveCriticalSection( &g_cs );
	}
}


//-----------------------------------------------------------------------------------------------
_declspec( dllexport ) void FetchNewOrders( int, Orders& newOrders_out )
{
	EnterCriticalSection( &g_cs );
	bool areNewUpdatesReady = g_areUpdatesReady;
	LeaveCriticalSection( &g_cs );

	while( !areNewUpdatesReady )
	{
		Sleep( 0 );
		EnterCriticalSection( &g_cs );
		areNewUpdatesReady = g_areUpdatesReady;
		LeaveCriticalSection( &g_cs );
	}

	EnterCriticalSection( &g_cs );
	newOrders_out = g_tentativeOrders;
	g_tentativeOrders.m_numberOfOrders = 0;
	g_haveResultsReturned = false;
	LeaveCriticalSection( &g_cs );
}


//-----------------------------------------------------------------------------------------------
_declspec( dllexport ) void SendUpdate( int turnNumber, int currentNutrients, const AgentReports& agentReports, const ObservedEntities& observedEntities, const ArenaSquares& observedSquares )
{
	EnterCriticalSection( &g_cs );
	g_currentTurnNumber = turnNumber;
	g_currentNutrients = currentNutrients;
	g_agentReportsFromPrevOrders = agentReports;
	g_areUpdatesReady = false;
	g_haveResultsReturned = true;
	ArenaSquares squaresMemoryCopy = g_arenaSquaresMemory;
	std::map< int, ShortVector2 > queenPositionCopy = g_enemyQueenPositions;
	LeaveCriticalSection( &g_cs );

	for( short arenaColumns = 0; arenaColumns < g_arenaDimension.x; ++arenaColumns )
	{
		for( short arenaRows = 0; arenaRows < g_arenaDimension.y; ++arenaRows )
		{
			ArenaSquareType memorySquareType = squaresMemoryCopy.GetSquareTypeAtCoords( arenaColumns, arenaRows );
			ArenaSquareType currentObservedSquareType = observedSquares.GetSquareTypeAtCoords( arenaColumns, arenaRows );
			if( currentObservedSquareType != ARENA_SQUARE_TYPE_UNKNOWN && memorySquareType != ARENA_SQUARE_TYPE_STONE )
			{
				squaresMemoryCopy.SetSquareTypeAtCoords( arenaColumns, arenaRows, currentObservedSquareType );
			}
		}
	}

	std::map< int, ShortVector2 >::iterator queenIter = queenPositionCopy.begin();
	while( queenIter != queenPositionCopy.end() )
	{
		ShortVector2 queenPosition = queenIter->second;
		ArenaSquareType queenSquareType = observedSquares.GetSquareTypeAtCoords( queenPosition.x, queenPosition.y );
		if( queenSquareType != ARENA_SQUARE_TYPE_UNKNOWN )
		{
			queenIter = queenPositionCopy.erase( queenIter );
		}
		else
		{
			++queenIter;
		}
	}

	for( int entityIndex = 0; entityIndex < observedEntities.m_numberOfObservedEntities; ++entityIndex )
	{
		ObservedEntity entity = observedEntities.m_entities[ entityIndex ];
		if( entity.m_agentType == ENTITY_TYPE_QUEEN )
		{
			queenPositionCopy[ entity.m_entityID ] = ShortVector2( entity.m_positionX, entity.m_positionY );
		}
	}

	EnterCriticalSection( &g_cs );
	g_arenaSquaresMemory = squaresMemoryCopy;
	g_enemyQueenPositions = queenPositionCopy;
	LeaveCriticalSection( &g_cs );
}


//-----------------------------------------------------------------------------------------------
_declspec( dllexport ) void EndWork()
{
	g_isQuitting = true;
}


//-----------------------------------------------------------------------------------------------
_declspec( dllexport ) void DrawDebugOverlay( short, short )
{
	EnterCriticalSection( &g_cs );

	glColor4ub( 0, 255, 255, 255 );
	glBegin( GL_QUADS );
	{
		for( short arenaColumns = 0; arenaColumns < g_arenaDimension.x; ++arenaColumns )
		{
			for( short arenaRows = 0; arenaRows < g_arenaDimension.y; ++arenaRows )
			{
				ArenaSquareType squareType = g_arenaSquaresMemory.GetSquareTypeAtCoords( arenaColumns, arenaRows );
				if( squareType == ARENA_SQUARE_TYPE_UNKNOWN )
				{
					glVertex2f( arenaColumns - 0.5f, arenaRows - 0.5f );
					glVertex2f( arenaColumns + 0.5f, arenaRows - 0.5f );
					glVertex2f( arenaColumns + 0.5f, arenaRows + 0.5f );
					glVertex2f( arenaColumns - 0.5f, arenaRows + 0.5f );
				}
			}
		}
	}
	glEnd();

	/*glBegin( GL_LINES );
	{
		for( unsigned int antIndex = 0; antIndex < g_ants.size(); ++antIndex )
		{
			Ant* ant = g_ants[ antIndex ];
			if( ant->m_type == ENTITY_TYPE_WORKER )
				glColor4ub( 255, 0, 0, 255 );
			else if( ant->m_type == ENTITY_TYPE_SCOUT )
				glColor4ub( 0, 255, 0, 255 );
			else if( ant->m_type == ENTITY_TYPE_SOLDIER )
				glColor4ub( 0, 0, 255, 255 );
			else if( ant->m_type == ENTITY_TYPE_QUEEN )
				glColor4ub( 255, 255, 255, 255 );

			std::vector< ShortVector2 > path = ant->m_path;
			ShortVector2 nodeComingFrom = ant->m_position;
			for( unsigned int nodeIndex = 0; nodeIndex < path.size(); ++nodeIndex )
			{
				ShortVector2 node = ant->m_path[ nodeIndex ];

				glVertex2f( nodeComingFrom.x, nodeComingFrom.y );
				glVertex2f( node.x, node.y );

				nodeComingFrom = node;
			}
		}
	}
	glEnd();*/

	LeaveCriticalSection( &g_cs );
}