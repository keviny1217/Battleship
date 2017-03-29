#ifndef BATTLESHIP_H
#define BATTLESHIP_H

/* The Battleship board size. The board is a SIZExSIZE 2-dimensional char array.
 */
#define SIZE 10

/* The maximum number of shots it should take to sink every ship
 */
#define MAX_SHOTS SIZE*SIZE

/* The value each cell of the board contains when it is empty (no ship).
 */
 #define OPEN_WATER 0

/* Number of ships
 */
#define NUMBER_OF_SHIPS 5

/* Number of games
 */
#define GAMES 2 

/* Ship types - The values to put on the board when hiding the ships
 */
#define AIRCRAFT_CARRIER 11
#define BATTLESHIP 22
#define DESTROYER 33
#define SUBMARINE 44
#define PATROL_BOAT 55

/* Ship sizes - The number of position each ship takes on the board
 */
#define SIZE_AIRCRAFT_CARRIER 5
#define SIZE_BATTLESHIP 4
#define SIZE_DESTROYER 3
#define SIZE_SUBMARINE 3
#define SIZE_PATROL_BOAT 2

/* Possible shot outcomes - passed as an int.
 */
#define MISS 1
#define HIT  2
#define SINK 3

/* Battleship game host-to-player message types - passed as an int.
 */
#define NEW_GAME 100
#define SHOT_REQUEST 101
#define SHOT_RESULT 103
#define OPPONENTS_SHOT 104
#define MATCH_OVER 105

/* Structure representing the coordinates of a shot - this is the structure
 * the player will send to the game host.
 */
typedef struct {
   unsigned short row; /* The row index of a shot */
   unsigned short col; /* The column index of a shot */
} Shot;

#endif
