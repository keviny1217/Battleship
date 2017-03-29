#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "battleship.h"

/* Structure used to keep track of the board and the last shot.
 */
typedef struct{
   char board[SIZE][SIZE];
   char index[2];
} Array;

static int getFD(const char *arg)
{
   int fd;
   if (1 != sscanf(arg, "%d", &fd)) {
      fprintf(stderr, "Bad fd argument\n");
      exit(EXIT_FAILURE);
   }

   return fd;
}

static int readMsg(int fd) {
   int msg;
   if(sizeof(int) != read(fd, &msg, sizeof(int))) {
      fprintf(stderr, "read failure in %s at line %d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }
   return msg;
}

/* Initializes the Array structure.
 */
static Array clear(Array arr) {
   int i = 0, j;
   for(; i < SIZE; i++) {
      j = 0;
      for(; j < SIZE; j++) 
         arr.board[i][j] = 0;
   }
   arr.index[0] = 0;
   arr.index[1] = 0;
   return arr;
}

static Array placeAC(Array arr) {
   int i = 0;
   for(; i < SIZE_AIRCRAFT_CARRIER; i++) 
      arr.board[i][3] = AIRCRAFT_CARRIER;
   return arr;
}

static Array placeB(Array arr) {
   int i = 0;
   for(; i < SIZE_BATTLESHIP; i++)
      arr.board[i][2] = BATTLESHIP;
   return arr;
}

static Array placeD(Array arr) {
   int i = 0;
   for(; i < SIZE_DESTROYER; i++)
      arr.board[SIZE-1][i] = DESTROYER;   
   return arr;
}

static Array placeS(Array arr) {
   int i = 0;
   for(; i < SIZE_SUBMARINE; i++)
      arr.board[6][i] = SUBMARINE;
   return arr;
}

static Array placePB(Array arr) {
   int i = 0;
   for(; i < SIZE_PATROL_BOAT; i++)
      arr.board[SIZE-1][i+6] = PATROL_BOAT;
   return arr;
}

static Array placeShips(Array arr) {
   arr = placeAC(arr);
   arr = placeB(arr);
   arr = placeD(arr);
   arr = placeS(arr);
   arr = placePB(arr);
   return arr;
}

static void sendBoard(int fd) {
   Array out;
   out = clear(out);
   out = placeShips(out);
   if(sizeof(out.board) != write(fd, out.board, sizeof(out.board))) {
      fprintf(stderr, "write failure in %s at line %d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }
}

/* Shoots in columns from left to right.
 */
static Array sendShot(int fd, Array arr) {
   Shot out;
   out.row = arr.index[0];
   out.col = arr.index[1];
   if(sizeof(Shot) != write(fd, &out, sizeof(Shot))) {
      fprintf(stderr, "write failure in %s at line %d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }
   if(arr.index[0] < (SIZE - 1))
      arr.index[0]++;
   else {
      arr.index[1]++;
      arr.index[0] = 0;
   }
   return arr;
}

int main(int argc, char **argv) {
   int readFD, writeFD, in = 0;
   Array shots;
   if (argc != 3) {
      fprintf(stderr, "Usage: player readFD writeFD\n");
      exit(EXIT_FAILURE);
   }
   readFD = getFD(argv[1]);
   writeFD = getFD(argv[2]);
   while(1) {
      in = readMsg(readFD);
      if(in == NEW_GAME) {
         shots = clear(shots); 
         sendBoard(writeFD);
      }
      else if(in == SHOT_REQUEST) 
         shots = sendShot(writeFD, shots);
      else if(in == MATCH_OVER)
         break;
      else
         readMsg(readFD);
   }
   return EXIT_SUCCESS;
}
