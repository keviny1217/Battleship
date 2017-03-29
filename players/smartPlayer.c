#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "battleship.h"

#define UNSET -1

#define UP 1
#define RIGHT 2
#define DOWN 3
#define LEFT 4

#define BLOB_STYLE 6
#define EDGE_STYLE 7
#define PREMADE_STYLE 8


/* Structure used in the logic of all the AI's decisions.
 */
typedef struct{
   char placeStyle;
   char board[SIZE][SIZE];
   int lastShot[2], lastHit[2], lastPShot[2];
   int sunk, result, games, dir;
} PLogic;

static int getFD(const char *arg) {
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

/* Initialize the PLogic structure.
 */
static PLogic clear(PLogic pl){
   int i = 0, j;
   for(; i < SIZE; i++) {
      j = 0;
      for(; j < SIZE; j++)
         pl.board[i][j] = 0;
   }
   pl.lastShot[0] = pl.lastShot[1] = UNSET;
   pl.lastHit[0] = pl.lastHit[1] = UNSET;
   pl.lastPShot[0] = pl.lastPShot[1] = UNSET;
   pl.sunk = 0;
   pl.result = MISS;
   pl.dir = UNSET;
   return pl;
}

static PLogic placeAC(PLogic pl) {
   int i = 0;
   if(pl.placeStyle == BLOB_STYLE) {
      if(pl.games%2) {
         for(; i < SIZE_AIRCRAFT_CARRIER; i++)
            pl.board[i+5][6] = AIRCRAFT_CARRIER;
      }
      else {
         for(; i < SIZE_AIRCRAFT_CARRIER; i++)
            pl.board[i+5][0] = AIRCRAFT_CARRIER;
      }
   }
   else if(pl.placeStyle == EDGE_STYLE) {
      if(pl.games%2) {
         for(; i < SIZE_AIRCRAFT_CARRIER; i++)
            pl.board[9][i+4] = AIRCRAFT_CARRIER;
      }
      else {
         for(; i < SIZE_AIRCRAFT_CARRIER; i++)
            pl.board[9][i] = AIRCRAFT_CARRIER;
      }
   }
   else {
      if(pl.games%2) {
         for(; i < SIZE_AIRCRAFT_CARRIER; i++)
            pl.board[i+5][2] = AIRCRAFT_CARRIER;
      }
      else {
         for(; i < SIZE_AIRCRAFT_CARRIER; i++)
            pl.board[0][i] = AIRCRAFT_CARRIER;
      }
   }
   return pl;
}

static PLogic placeB(PLogic pl) {
   int i = 0;
   if(pl.placeStyle == BLOB_STYLE) {
      if(pl.games%2) {
         for(; i < SIZE_BATTLESHIP; i++)
            pl.board[i+6][5] = BATTLESHIP;
      }
      else {
         for(; i < SIZE_BATTLESHIP; i++)
            pl.board[9][i+1] = BATTLESHIP;
      }
   }
   else if(pl.placeStyle == EDGE_STYLE) {
      if(pl.games%2) {
         for(; i < SIZE_BATTLESHIP; i++)
            pl.board[0][i+3] = BATTLESHIP;
      }
      else {
         for(; i < SIZE_BATTLESHIP; i++)
            pl.board[i+1][0] = BATTLESHIP;
      }
   }
   else {
      if(pl.games%2) {
         for(; i < SIZE_BATTLESHIP; i++)
            pl.board[i+6][7] = BATTLESHIP;
      }
      else {
         for(; i < SIZE_BATTLESHIP; i++)
            pl.board[i+2][5] = BATTLESHIP;
      }
   }
   return pl;
}

static PLogic placeD(PLogic pl) {
   int i = 0;
   if(pl.placeStyle == BLOB_STYLE) {
      if(pl.games%2) {
         for(; i < SIZE_DESTROYER; i++)
            pl.board[i+5][4] = DESTROYER;
      }
      else {
         for(; i < SIZE_DESTROYER; i++)
            pl.board[7][i+1] = DESTROYER;
      }
   }
   else if(pl.placeStyle == EDGE_STYLE) {
      if(pl.games%2) {
         for(; i < SIZE_DESTROYER; i++)
            pl.board[0][i+7] = DESTROYER;
      }
      else {
         for(; i < SIZE_DESTROYER; i++)
            pl.board[0][i+1] = DESTROYER;
      }
   }
   else {
      if(pl.games%2) {
         for(; i < SIZE_DESTROYER; i++)
            pl.board[i+4][6] = DESTROYER;
      }
      else {
         for(; i < SIZE_DESTROYER; i++)
            pl.board[i][9] = DESTROYER;
      }
   }
   return pl;
}

static PLogic placeS(PLogic pl) {
   int i = 0;
   if(pl.placeStyle == BLOB_STYLE) {
      if(pl.games%2) {
         for(; i < SIZE_SUBMARINE; i++)
            pl.board[i+3][5] = SUBMARINE;
      }
      else {
         for(; i < SIZE_SUBMARINE; i++)
            pl.board[8][i+1] = SUBMARINE;
      }
   }
   else if(pl.placeStyle == EDGE_STYLE) {
      if(pl.games%2) {
         for(; i < SIZE_SUBMARINE; i++)
            pl.board[i+5][0] = SUBMARINE;
      }
      else {
         for(; i < SIZE_SUBMARINE; i++)
            pl.board[0][i+5] = SUBMARINE;
      }
   }
   else {
      if(pl.games%2) {
         for(; i < SIZE_SUBMARINE; i++)
            pl.board[i+1][9] = SUBMARINE;
      }
      else {
         for(; i < SIZE_SUBMARINE; i++)
            pl.board[4][i+7] = SUBMARINE;
      }
   }
   return pl;
}

static PLogic placePB(PLogic pl) {
   int i = 0;
   if(pl.placeStyle == BLOB_STYLE) {
      if(pl.games%2) {
         for(; i < SIZE_PATROL_BOAT; i++)
            pl.board[i+8][4] = PATROL_BOAT;
      }
      else {
         for(; i < SIZE_PATROL_BOAT; i++)
            pl.board[6][i+1] = PATROL_BOAT;
      }
   }
   else if(pl.placeStyle == EDGE_STYLE) {
      if(pl.games%2) {
         for(; i < SIZE_PATROL_BOAT; i++)
            pl.board[i+1][0] = PATROL_BOAT;
      }
      else {
         for(; i < SIZE_PATROL_BOAT; i++)
            pl.board[9][i+2] = PATROL_BOAT;
      }
   }
   else {
      if(pl.games%2) {
         for(; i < SIZE_PATROL_BOAT; i++)
            pl.board[3][i+1] = PATROL_BOAT;
      }
      else {
         for(; i < SIZE_PATROL_BOAT; i++)
            pl.board[i+6][3] = PATROL_BOAT;
      }
   }
   return pl;
}

static PLogic placeShips(PLogic pl) {
   pl = placeAC(pl);
   pl = placeB(pl);
   pl = placeD(pl);
   pl = placeS(pl);
   pl = placePB(pl);
   return pl;
}

static void sendBoard(int fd, PLogic pl) {
   PLogic out;
   out = clear(out);
   out.games = pl.games;
   out.placeStyle = pl.placeStyle;
   out = placeShips(out);
   if(sizeof(out.board) != write(fd, out.board, sizeof(out.board))) {
      fprintf(stderr, "write failure in %s at line %d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }
}

static int shot(Shot shot, PLogic pl) {
   return pl.board[shot.row][shot.col];
}

static int outOfBounds(Shot shot) {
   if(shot.row >= SIZE || shot.row < 0)
      return 1;
   if(shot.col >= SIZE || shot.col < 0)
      return 1;
   return 0;
}

/* Change the direction when hunting for a ship.
 */
static PLogic changeDir(PLogic pl) {
   if(pl.dir == UP)
      pl.dir = DOWN;
   else if(pl.dir == RIGHT)
      pl.dir = LEFT;
   else if(pl.dir == DOWN)
      pl.dir = RIGHT;
   else 
      pl.dir = UNSET;
   return pl;
}

static PLogic sendShot(int fd, Shot shot, PLogic pl) {
   if(sizeof(Shot) != write(fd, &shot, sizeof(Shot))) {
      fprintf(stderr, "write failure in %s at line %d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }
   pl.lastShot[0] = shot.row;
   pl.lastShot[1] = shot.col;
   return pl;
}

/* Sends a shot that is searching for a ship after a hit.
 */
static PLogic sendSearch(int fd, PLogic pl) {
   Shot out;
   int dx, dy;
   dx = dy = 0;
   out.row = pl.lastHit[0];
   out.col = pl.lastHit[1];
   while(1) {
      if(pl.dir == UP) 
         dy = -1;
      else if(pl.dir == RIGHT)
         dx = 1;
      else if(pl.dir == DOWN)
         dy = 1;
      else
         dx = -1;
      out.row += dy;
      out.col += dx;
      if(outOfBounds(out)) {
         pl = changeDir(pl);
         dx = dy = 0;
         out.row = pl.lastHit[0];
         out.col = pl.lastHit[1];
      }
      else if(!shot(out, pl)) {
         pl = sendShot(fd, out, pl);
         break;
      }
      else if(shot(out, pl) != HIT) {
         pl = changeDir(pl);
         dx = dy = 0;
         out.row = pl.lastHit[0];
         out.col = pl.lastHit[1];
      }
   }
   return pl;
}

/* Sends a normal patterned shot that zigzags from the top right corner.
 */
static PLogic sendStandard(int fd, PLogic pl) {
   Shot out;
   if(pl.lastPShot[0] == UNSET && pl.lastPShot[1] == UNSET) {
      out.row = 0;
      out.col = SIZE - 2;
   }
   else {
      out.row = pl.lastPShot[0];
      out.col = pl.lastPShot[1];
   }
   while(1) {
      if(!shot(out, pl)) {
         pl = sendShot(fd, out, pl);
         pl.lastPShot[0] = out.row;    // keeps track of last patterned shot to resume
         pl.lastPShot[1] = out.col;    // after sinking a ship with search
         break;
      }
      if(out.col > 1)
         out.col -= 2;
      else {
         out.col = SIZE - (out.row % 2) - 1;
         out.row++;
      }
   }
   return pl;
}

static PLogic selectShot(int fd, PLogic pl) {
   if(pl.lastHit[0] != UNSET && pl.lastHit[0] != UNSET)
      pl = sendSearch(fd, pl);
   else
      pl = sendStandard(fd, pl);
   return pl;
}

/* Store the shot's results to be used in determining future decisions. 
 */
static PLogic storeResult(int fd, PLogic pl) {
   pl.result = readMsg(fd);
   pl.board[(int)pl.lastShot[0]][(int)pl.lastShot[1]] = pl.result;
   if(pl.result == SINK) {
      pl.sunk++;
      pl.lastHit[0] = pl.lastHit[1] = UNSET;
      pl.dir = UNSET;
   }
   else if(pl.result == HIT) {
      if(pl.lastHit[0] == UNSET && pl.lastHit[1] == UNSET) {
         pl.lastHit[0] = pl.lastShot[0];
         pl.lastHit[1] = pl.lastShot[1];
         pl.dir = UP;
      }
   }
   else if(pl.dir != UNSET) {
      pl = changeDir(pl);
   }
   return pl;
}

/* Change styles on each loss. 
 */
static PLogic setStyle(PLogic pl) {
   if(pl.games == 0) 
      pl.placeStyle = BLOB_STYLE;      // places ships together
   else if(pl.placeStyle == BLOB_STYLE)
      pl.placeStyle = EDGE_STYLE;      // places ships on the edge
   else if(pl.placeStyle == EDGE_STYLE)
      pl.placeStyle = PREMADE_STYLE;   // places ships based on a preset with no pattern
   else
      pl.placeStyle = PREMADE_STYLE;
   return pl;
}

int main(int argc, char **argv) {
   int readFD, writeFD, in = 0;
   PLogic pl;
   pl.sunk = pl.games = 0;
   if (argc != 3) {
      fprintf(stderr, "Usage: player readFD writeFD\n");
      exit(EXIT_FAILURE);
   }
   readFD = getFD(argv[1]);
   writeFD = getFD(argv[2]);
   while(1) {
      in = readMsg(readFD);
      if(in == NEW_GAME) {
         if(pl.sunk != NUMBER_OF_SHIPS)
            pl = setStyle(pl);
         pl = clear(pl);
         pl.games++;
         sendBoard(writeFD, pl);
      }
      else if(in == SHOT_REQUEST)
         pl = selectShot(writeFD, pl);
      else if(in == SHOT_RESULT)
         pl = storeResult(readFD, pl);
      else if(in == OPPONENTS_SHOT)
         readMsg(readFD);
      else if(in == MATCH_OVER)
         break;
   }
   return EXIT_SUCCESS;
}
