About:
Ant Colony DLL is a DLL plugin for a given ant colony AI game
where the purpose is to have the ant colony that survives the
longest by gathering food and fighting other colonies. Given
a set of functions that are called each turn, the DLL returns
the actions of each of the colony's ants to the program.
The call for returning the ant's actions must not take longer
than 1 ms or else the colony is penalized. The DLL uses
A* pathfinding to set the ants' paths toward food, enemy ants,
and the queen ant, where food is deposited. Ant Colony DLL was
written in C++ and was completed over a 2 week period.
