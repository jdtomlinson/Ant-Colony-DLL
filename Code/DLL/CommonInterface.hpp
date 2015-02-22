//-----------------------------------------------------------------------------------------------
// CommonInterface.hpp
//
// This header is meant to be used both by the Arena/server code and the Player/client code.
// Defines data types and utilities facilitating cross-module data transfer and versioning.
//-----------------------------------------------------------------------------------------------
#ifndef __include_CommonInterface__
#define __include_CommonInterface__
#include <memory.h>
#include <windows.h>
#include <crtdbg.h>
#include <stdio.h>
#include <vadefs.h>


/////////////////////////////////////////////////////////////////////////////////////////////////
// Global constants
const int COMMON_INTERFACE_VERSION_NUMBER = 15;
const short MAX_ARENA_WIDTH = 200;
const short MAX_ARENA_HEIGHT = 200;
const int MAX_ARENA_SQUARES = (MAX_ARENA_WIDTH * MAX_ARENA_HEIGHT);
const short MAX_PLAYERS = 10;
const short MAX_AGENTS_PER_PLAYER = 100;
const short MAX_ORDERS_PER_PLAYER = MAX_AGENTS_PER_PLAYER;
const short MAX_REPORTS_PER_PLAYER = 2 * MAX_AGENTS_PER_PLAYER;
const short MAX_AGENTS_TOTAL = (MAX_PLAYERS * MAX_AGENTS_PER_PLAYER);
const double MAXIMUM_SECONDS_PER_DLL_CALL = 0.015;


//-----------------------------------------------------------------------------------------------
inline void DebuggerPrintf( const char* messageFormat, ... )
{
	const int MESSAGE_MAX_LENGTH = 2048;
	char messageLiteral[ MESSAGE_MAX_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, messageFormat );
	vsnprintf_s( messageLiteral, MESSAGE_MAX_LENGTH, _TRUNCATE, messageFormat, variableArgumentList );
	va_end( variableArgumentList );
	messageLiteral[ MESSAGE_MAX_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)
	OutputDebugStringA( messageLiteral );
}


/////////////////////////////////////////////////////////////////////////////////////////////////
enum CardinalDirection
{
	CARDINAL_DIRECTION_NONE			= 0,
	CARDINAL_DIRECTION_EAST			= 1,
	CARDINAL_DIRECTION_NORTH		= 2,
	CARDINAL_DIRECTION_WEST			= 3,
	CARDINAL_DIRECTION_SOUTH		= 4,
	NUM_CARDINAL_DIRECTIONS
};


/////////////////////////////////////////////////////////////////////////////////////////////////
enum ArenaSquareType
{
	ARENA_SQUARE_TYPE_AIR			= 0,
	ARENA_SQUARE_TYPE_DIRT			= 1,
	ARENA_SQUARE_TYPE_STONE			= 2,
	ARENA_SQUARE_TYPE_FOOD			= 3,
	ARENA_SQUARE_TYPE_UNKNOWN		= 4,
	NUM_ARENA_SQUARE_TYPES
};


/////////////////////////////////////////////////////////////////////////////////////////////////
enum EntityType
{
	// Listed in order of starvation (soldiers starve first, workers and queens last)
	ENTITY_TYPE_SOLDIER,
	ENTITY_TYPE_SCOUT,
	ENTITY_TYPE_WORKER,
	ENTITY_TYPE_QUEEN,
	NUM_AGENT_TYPES,
	NUM_ENTITY_TYPES = NUM_AGENT_TYPES
};


/////////////////////////////////////////////////////////////////////////////////////////////////
enum EntitySpecialStatus
{
	ENTITY_SPECIAL_STATUS_NONE,
	ENTITY_SPECIAL_STATUS_CARRYING_FOOD,
	ENTITY_SPECIAL_STATUS_EXHAUSTED,
	ENTITY_SPECIAL_STATUS_DEAD,
	NUM_ENTITY_SPECIAL_STATUSES
};


/////////////////////////////////////////////////////////////////////////////////////////////////
enum OrderCode
{
	ORDER_HOLD,
	ORDER_MOVE_EAST,
	ORDER_MOVE_NORTH,
	ORDER_MOVE_WEST,
	ORDER_MOVE_SOUTH,
	ORDER_TAKE_FOOD,
	ORDER_DROP_FOOD,
	ORDER_CREATE_WORKER,
	ORDER_CREATE_SOLDIER,
	ORDER_CREATE_SCOUT,
	ORDER_SUICIDE,
	NUM_ORDERS
};


/////////////////////////////////////////////////////////////////////////////////////////////////
enum ReportCode
{
	REPORT_HOLD_SUCCESSFUL,
	REPORT_MOVE_SUCCESSFUL,
	REPORT_DIG_SUCCESSFUL,
	REPORT_ATTACK_SUCCESSFUL,
	REPORT_TAKE_SUCCESSFUL,
	REPORT_DROP_SUCCESSFUL,
	REPORT_CREATE_SUCCESSFUL,
	REPORT_SUICIDE_SUCCESSFUL,
	REPORT_WAS_KILLED_IN_COMBAT,
	REPORT_STARVED_TO_DEATH,
	REPORT_QUEEN_WAS_ASSAULTED,
	REPORT_QUEEN_WAS_FED,
	REPORT_WAS_CREATED,
	REPORT_ERROR_BAD_ANT_ID,
	REPORT_ERROR_ANT_MOVED_TWICE,
	REPORT_ERROR_WRONG_ANT_TYPE,
	REPORT_ERROR_BLOCKED_BY_ROCK,
	REPORT_ERROR_OUT_OF_BOUNDS,
	REPORT_ERROR_NO_FOOD_PRESENT,
	REPORT_ERROR_ALREADY_CARRYING_FOOD,
	REPORT_ERROR_SQUARE_NOT_EMPTY,
	REPORT_ERROR_NOT_CARRYING_FOOD,
	REPORT_ERROR_INSUFFICIENT_FOOD,
	REPORT_ERROR_QUEEN_WAS_EXHAUSTED,
	REPORT_ERROR_MAXIMUM_POPULATION_REACHED,
	NUM_RESULTS
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct ArenaInfo
{
	char numPlayers;
	short width;
	short height;
	int startingNutrients;
	int nutrientsEarnedPerFoodEatenByQueen;
	int upkeepNutrientsPerTurnForQueen;
	int upkeepNutrientsPerTurnForWorker;
	int upkeepNutrientsPerTurnForScout;
	int upkeepNutrientsPerTurnForSoldier;
	int nutrientCostToBirthWorker;
	int nutrientCostToBirthScout;
	int nutrientCostToBirthSoldier;
	int visibilityRangeQueen;
	int visibilityRangeWorker;
	int visibilityRangeScout;
	int visibilityRangeSoldier;
	int combatStrengthWorker;
	int combatStrengthScout;
	int combatStrengthSoldier;
	int combatStrengthBonusFromNearbyQueen;
	int combatStrengthBonusMaximumDistance;
	int numTurnsBeforeSuddenDeath;
	int nutrientLossPerAttackerStrength;
	int colonyMaxPopulation;
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct AgentReport
{
	int		m_entityID;
	short	m_newPositionX;
	short	m_newPositionY;
	char	m_agentType; // is actually an EntityType
	char	m_reportCode; // is actually a ReportCode
	char	m_specialStatus; // is actually an EntitySpecialStatus
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct AgentReports
{
	AgentReports()
		: m_numberAgentReports( 0 )
	{}

	bool AddReport( int entityID, EntityType agentType, ReportCode reportCode, short newX, short newY, EntitySpecialStatus status, bool overrideExistingReportForThisAgentIfExists=false )
	{
		if( overrideExistingReportForThisAgentIfExists )
		{
			for( int agentReportIndex = 0; agentReportIndex < m_numberAgentReports; ++ agentReportIndex )
			{
				if( m_agentReports[ agentReportIndex ].m_entityID == entityID )
				{
					if( agentReportIndex != m_numberAgentReports-1 )
						m_agentReports[ agentReportIndex ] = m_agentReports[ m_numberAgentReports-1 ];

					-- m_numberAgentReports;
					break;
				}
			}
		}

		if( m_numberAgentReports >= MAX_REPORTS_PER_PLAYER )
			return false;

		_ASSERT( m_numberAgentReports < MAX_REPORTS_PER_PLAYER );
		m_agentReports[ m_numberAgentReports ].m_entityID = entityID;
		m_agentReports[ m_numberAgentReports ].m_agentType = (char) agentType;
		m_agentReports[ m_numberAgentReports ].m_reportCode = (char) reportCode;
		m_agentReports[ m_numberAgentReports ].m_newPositionX = newX;
		m_agentReports[ m_numberAgentReports ].m_newPositionY = newY;
		m_agentReports[ m_numberAgentReports ].m_specialStatus = (char) status;
		++ m_numberAgentReports;

		return true;
	}

	void AddReport( const AgentReport& report )
	{
		AddReport( report.m_entityID, (EntityType) report.m_agentType, (ReportCode) report.m_reportCode, report.m_newPositionX, report.m_newPositionY, (EntitySpecialStatus) report.m_specialStatus );
	}

	AgentReport				m_agentReports[ MAX_REPORTS_PER_PLAYER ];
	int						m_numberAgentReports;
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct AgentOrder
{
	int						m_entityID;
	OrderCode				m_orderCode;
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct Orders
{
	Orders()
		: m_numberOfOrders( 0 )
	{}

	bool AddOrder( int entityID, OrderCode orderCode, bool overrideExistingOrderForThisAgentIfExists=false )
	{
		if( overrideExistingOrderForThisAgentIfExists )
		{
			for( int orderIndex = 0; orderIndex < m_numberOfOrders; ++ orderIndex )
			{
				if( m_orders[ orderIndex ].m_entityID == entityID )
				{
					if( orderIndex != m_numberOfOrders-1 )
						m_orders[ orderIndex ] = m_orders[ m_numberOfOrders-1 ];

					-- m_numberOfOrders;
					break;
				}
			}
		}

		if( m_numberOfOrders >= MAX_ORDERS_PER_PLAYER )
			return false;

		_ASSERT( m_numberOfOrders < MAX_ORDERS_PER_PLAYER );
		m_orders[ m_numberOfOrders ].m_entityID = entityID;
		m_orders[ m_numberOfOrders ].m_orderCode = orderCode;
		++ m_numberOfOrders;

		return true;
	}

	AgentOrder				m_orders[ MAX_ORDERS_PER_PLAYER ];
	int						m_numberOfOrders;
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct ObservedEntity
{
	int		m_entityID;
	char	m_playerID;
	char	m_agentType; // is actually an EntityType
	char	m_specialStatus; // is actually an EntitySpecialStatus
	char	m_lastAction; // is actually an OrderCode
	short	m_positionX;
	short	m_positionY;
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct ObservedEntities
{
	ObservedEntities()
		: m_numberOfObservedEntities( 0 )
	{}

	void AddObservedEntity( int entityID, char playerID, EntityType entityType, EntitySpecialStatus status, OrderCode lastAction, short positionX, short positionY )
	{
		_ASSERT( m_numberOfObservedEntities < MAX_AGENTS_TOTAL );
		m_entities[ m_numberOfObservedEntities ].m_entityID = entityID;
		m_entities[ m_numberOfObservedEntities ].m_playerID = playerID;
		m_entities[ m_numberOfObservedEntities ].m_agentType = (char) entityType;
		m_entities[ m_numberOfObservedEntities ].m_specialStatus = (char) status;
		m_entities[ m_numberOfObservedEntities ].m_lastAction = (char) lastAction;
		m_entities[ m_numberOfObservedEntities ].m_positionX = positionX;
		m_entities[ m_numberOfObservedEntities ].m_positionY = positionY;
		++ m_numberOfObservedEntities;
	}

	ObservedEntity			m_entities[ MAX_AGENTS_TOTAL ];
	int						m_numberOfObservedEntities;
};


/////////////////////////////////////////////////////////////////////////////////////////////////
struct ArenaSquares
{
	ArenaSquares()
		: m_width( MAX_ARENA_WIDTH )
		, m_height( MAX_ARENA_HEIGHT )
	{
		memset( m_arenaSquareMaterials, 0, sizeof( m_arenaSquareMaterials ) );
	}

	ArenaSquares( short width, short height )
		: m_width( width )
		, m_height( height )
	{
		memset( m_arenaSquareMaterials, 0, sizeof( m_arenaSquareMaterials ) );
	}

	ArenaSquares( const ArenaSquares& copy )
		: m_width( copy.m_width )
		, m_height( copy.m_height )
	{
		memcpy( m_arenaSquareMaterials, copy.m_arenaSquareMaterials, sizeof( m_arenaSquareMaterials ) );
	}

	ArenaSquareType GetSquareTypeAtCoords( short x, short y ) const
	{
		int index = x + (y * m_width);
		if( index < 0 || index >= (m_width * m_height) )
			return ARENA_SQUARE_TYPE_STONE;
		else
			return (ArenaSquareType) m_arenaSquareMaterials[ index ];
	}

	void SetSquareTypeAtCoords( short x, short y, ArenaSquareType newSquareType )
	{
		int index = x + (y * m_width);
		if( index >= 0 && index < (m_width * m_height) )
		{
			m_arenaSquareMaterials[ index ] = (char) newSquareType;
		}
	}

	void DebugDumpContents() const
	{
		int numOfEachArenaSquareType[ NUM_ARENA_SQUARE_TYPES + 1 ] = { 0, 0, 0, 0, 0, 0 };

		for( int x = 0; x < m_width; ++ x )
		for( int y = 0; y < m_height; ++ y )
		{
			int index = x + (y * m_width);
			ArenaSquareType squareType = (ArenaSquareType) m_arenaSquareMaterials[ index ];
			if( squareType < 0 || squareType >= NUM_ARENA_SQUARE_TYPES )
				squareType = NUM_ARENA_SQUARE_TYPES; // used internally here only as "illegal enum"
			++ numOfEachArenaSquareType[ squareType ];
		}

		DebuggerPrintf( "ArenaDebugDump: %dx%d with %d air, %d dirt, %d stone, %d food, %d unknown, %d invalid.\n",
			m_width, m_height,
			numOfEachArenaSquareType[ ARENA_SQUARE_TYPE_AIR ],
			numOfEachArenaSquareType[ ARENA_SQUARE_TYPE_DIRT ],
			numOfEachArenaSquareType[ ARENA_SQUARE_TYPE_STONE ],
			numOfEachArenaSquareType[ ARENA_SQUARE_TYPE_FOOD ],
			numOfEachArenaSquareType[ ARENA_SQUARE_TYPE_UNKNOWN ],
			numOfEachArenaSquareType[ NUM_ARENA_SQUARE_TYPES ] ); // any illegal type #
	}


	char	m_arenaSquareMaterials[ MAX_ARENA_SQUARES ]; // are each actually ArenaSquareType
	short	m_width;
	short	m_height;
};


//-----------------------------------------------------------------------------------------------
inline const char* GetNameForEntityType( EntityType agentType )
{
	switch( agentType )
	{
		case ENTITY_TYPE_WORKER:	return "Worker";
		case ENTITY_TYPE_SOLDIER:	return "Soldier";
		case ENTITY_TYPE_SCOUT:		return "Scout";
		case ENTITY_TYPE_QUEEN:		return "Queen";
		default:					return "Unknown";
	}
}


//-----------------------------------------------------------------------------------------------
inline const char* GetNameForOrderCode( OrderCode orderCode )
{
	switch( orderCode )
	{
		case ORDER_HOLD:			return "HOLD";
		case ORDER_MOVE_EAST:		return "MOVE_EAST";
		case ORDER_MOVE_NORTH:		return "MOVE_NORTH";
		case ORDER_MOVE_WEST:		return "MOVE_WEST";
		case ORDER_MOVE_SOUTH:		return "MOVE_SOUTH";
		case ORDER_TAKE_FOOD:		return "TAKE_FOOD";
		case ORDER_DROP_FOOD:		return "DROP_FOOD";
		case ORDER_CREATE_WORKER:	return "CREATE_WORKER";
		case ORDER_CREATE_SOLDIER:	return "CREATE_SOLDIER";
		case ORDER_CREATE_SCOUT:	return "CREATE_SCOUT";
		case ORDER_SUICIDE:			return "SUICIDE";
		default:					return "Unknown order code!";
	}
}


//-----------------------------------------------------------------------------------------------
inline const char* GetNameForReportCode( ReportCode reportCode )
{
	switch( reportCode )
	{
		case REPORT_HOLD_SUCCESSFUL:					return "HOLD_SUCCESSFUL";
		case REPORT_MOVE_SUCCESSFUL:					return "MOVE_SUCCESSFUL";
		case REPORT_DIG_SUCCESSFUL:						return "DIG_SUCCESSFUL";
		case REPORT_ATTACK_SUCCESSFUL:					return "ATTACK_SUCCESSFUL";
		case REPORT_TAKE_SUCCESSFUL:					return "TAKE_SUCCESSFUL";
		case REPORT_DROP_SUCCESSFUL:					return "DROP_SUCCESSFUL";
		case REPORT_CREATE_SUCCESSFUL:					return "CREATE_SUCCESSFUL";
		case REPORT_SUICIDE_SUCCESSFUL:					return "SUICIDE_SUCCESSFUL";
		case REPORT_WAS_KILLED_IN_COMBAT:				return "WAS_KILLED_IN_COMBAT";
		case REPORT_STARVED_TO_DEATH:					return "STARVED_TO_DEATH";
		case REPORT_QUEEN_WAS_ASSAULTED:				return "QUEEN_WAS_ASSAULTED";
		case REPORT_QUEEN_WAS_FED:						return "QUEEN_WAS_FED";
		case REPORT_WAS_CREATED:						return "WAS_CREATED";
		case REPORT_ERROR_BAD_ANT_ID:					return "ERROR_BAD_ANT_ID";
		case REPORT_ERROR_ANT_MOVED_TWICE:				return "ERROR_ANT_MOVED_TWICE";
		case REPORT_ERROR_WRONG_ANT_TYPE:				return "ERROR_WRONG_ANT_TYPE";
		case REPORT_ERROR_BLOCKED_BY_ROCK:				return "ERROR_BLOCKED_BY_ROCK";
		case REPORT_ERROR_OUT_OF_BOUNDS:				return "ERROR_OUT_OF_BOUNDS";
		case REPORT_ERROR_NO_FOOD_PRESENT:				return "ERROR_NO_FOOD_PRESENT";
		case REPORT_ERROR_ALREADY_CARRYING_FOOD:		return "ERROR_ALREADY_CARRYING_FOOD";
		case REPORT_ERROR_SQUARE_NOT_EMPTY:				return "ERROR_SQUARE_NOT_EMPTY";
		case REPORT_ERROR_NOT_CARRYING_FOOD:			return "ERROR_NOT_CARRYING_FOOD";
		case REPORT_ERROR_INSUFFICIENT_FOOD:			return "ERROR_INSUFFICIENT_FOOD";
		case REPORT_ERROR_QUEEN_WAS_EXHAUSTED:			return "ERROR_QUEEN_WAS_EXHAUSTED";
		case REPORT_ERROR_MAXIMUM_POPULATION_REACHED:	return "ERROR_MAXIMUM_POPULATION_REACHED";
		default:										return "Unknown report code!";
	}
}


//-----------------------------------------------------------------------------------------------
// List of functions each AI DLL must export to the outside world
//
#ifndef ARENA
extern "C"
{
	__declspec( dllexport ) int GetInterfaceVersion();
	__declspec( dllexport ) const char* GetPlayerName();
	__declspec( dllexport ) const char* GetAuthorName();
	__declspec( dllexport ) void GiveColors( unsigned int playerColorRGBABytes );
	__declspec( dllexport ) void PrepareForBattle( int playerID, const ArenaInfo& arenaInfo );
	__declspec( dllexport ) void BeginWork(); // called from the player's dedicated thread
	__declspec( dllexport ) void SendUpdate( int turnNumber, int currentNutrients, const AgentReports& agentReports, const ObservedEntities& observedEntities, const ArenaSquares& observedSquares );
	__declspec( dllexport ) void FetchNewOrders( int turnNumber, Orders& newOrders_out );
	__declspec( dllexport ) void EndWork();
	__declspec( dllexport ) void DrawDebugOverlay( short inspectSquareX, short inspectSquareY ); // optional; drawn when your DLL # is pressed twice on the keyboard
}
#endif


#endif // __include_CommonInterface__
