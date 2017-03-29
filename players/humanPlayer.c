#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "battleship.h"

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

/* Initialize a board.
 */
static void clear(char (*board)[SIZE][SIZE]){
   int i = 0, j;
   for(; i < SIZE; i++) {
      j = 0;
      for(; j < SIZE; j++) 
         (*board)[i][j] = 0;
   }
}

/* Clears stdin to help invalid responses not interfere with the next prompts.
 */
static void clearIn() {
   char c;
   while((c = getchar()) != '\n' && c != EOF) {}
}

/* Prompts for the orientation of the ship.
 */
static int orientation() {
   char in;
   while(1) {
      printf("\tWill this ship be horizontal? [Y/N]: ");
      scanf("%c", &in);
      if(in != 'Y' && in != 'N') {
         printf("\tInvalid response, please try again.\n");
         clearIn();
      }
      else if(in == 'Y')
         return 1;
      else
         return 0;
   }
}

/* Interprets the board and returns the proper character to display.
 */
static char symbol(char c) {
   switch(c) {
      case OPEN_WATER :
         return '-';
      case AIRCRAFT_CARRIER :
         return 'A';
      case BATTLESHIP :
         return 'B';
      case DESTROYER :
         return 'D';
      case SUBMARINE :
         return 'S';
      case PATROL_BOAT :
         return 'P';
      case 'O' :
      case 'X' :
         return c;
      default : 
         fprintf(stderr, "Bad board. Terminating\n");
         exit(EXIT_FAILURE);
   }
}

/* Display the board based on the previous function.
 */
static void displayBoard(char board[SIZE][SIZE]) {
   int i, j;
   i = j = 0;
   printf("\n\t  ");
   for(; i < SIZE; i++) 
      printf("%d ", i);
   i = 0;
   printf("\n");
   for(; i < SIZE; i++) {
      j = 0;
      printf("\t%d ", i);
      for(; j < SIZE; j++) 
         printf("%c ", symbol(board[i][j]));
      printf("\n");
   }
   printf("\n");
}

/* Updates the board by updating the board position with the shot result.
 */
static char updateShot(int result, Shot last) {
   switch(result) {
      case MISS :
         printf("Miss\n");
         return 'O';
      case SINK :
         printf("Enemy ship sunk\n");
         return 'X';
      case HIT :
         printf("Enemy ship hit\n");
         return 'X';
      default :
         fprintf(stderr, "Bad shot result. Terminating\n");
         exit(EXIT_FAILURE);
   }
}

/* Overwrites the ship with hits and open water with misses, based
 * on the shot result.
 */
static char updateOppShot(Shot opp, char board[SIZE][SIZE]) {
   if(board[opp.row][opp.col] != OPEN_WATER && board[opp.row][opp.col] != 'O')
      return 'X';
   else
      return 'O';
}

/* Place a shot horizontally using the row and column numbers given
 * in the prompts. Also checks to see if there is a collision with an existing
 * ship.
 */
static void placeHoriz(int size, char ship, char (*board)[SIZE][SIZE]) {
   int row, col, i = 0;
   while(1) {  // loops until a suitable position is found
      printf("\tEnter the leftmost column number [0-%d]: ", SIZE - size);
      scanf("%d", &col);
      if(col < 0 || col > SIZE - size) {
         printf("\tInvalid column number, please choose from given range.\n");
         clearIn();
      }
      else {
         printf("\tEnter the row number [0-%d]: ", SIZE - 1);
         scanf("%d", &row);
         clearIn();
         if(row < 0 || row > SIZE - 1)
            printf("\tInvalid row number, please choose from given range.\n");
         else {
            for(; i < size; i++) {
               if((*board)[row][i+col]) {
                  printf("\tInvalid position, something is already there.\n");
                  i = -1;
                  break;
               }
            }
            if(i != -1) {
               i = 0;
               for(; i < size; i++)
                  (*board)[row][i+col] = ship;
               return;
            } 
         }
      }
   }
}

/* Place a shot vertically using the row and column numbers given
 * in the prompts. Also checks to see if there is a collision with an existing
 * ship.
 */
static void placeVert(int size, char ship, char (*board)[SIZE][SIZE]) {
   int row, col, i = 0;
   while(1) {     // loops until a suitable position is found
      printf("\tEnter the topmost row number [0-%d]: ", SIZE - size);
      scanf("%d", &row);
      if(row < 0 || row > SIZE - size) {
         printf("\tInvalid row number, please choose from given range.\n");
         clearIn();
      }
      else {
         printf("\tEnter the column number [0-%d]: ", SIZE - 1);
         scanf("%d", &col);
         clearIn();
         if(col < 0 || col > SIZE - 1) 
            printf("\tInvalid column number, please choose from given range.\n");
         else {
            for(; i < size; i++) {
               if((*board)[i+row][col]) {
                  printf("\tInvalid position, something is already there.\n");
                  i = -1;
                  break;
               }
            }
            if(i != -1) {
               i = 0;
               for(; i < size; i++)
                  (*board)[i+row][col] = ship;
               return;
            } 
         }
      }
   }
}

static void placeAC(char (*board)[SIZE][SIZE]) {
   int horiz;
   printf("Placing aircraft carrier\n");
   horiz = orientation();
   if(horiz)
      placeHoriz(SIZE_AIRCRAFT_CARRIER, AIRCRAFT_CARRIER, board);
   else
      placeVert(SIZE_AIRCRAFT_CARRIER, AIRCRAFT_CARRIER, board);
   printf("Successfully placed aircraft carrier\n");
   displayBoard(*board);
}

static void placeB(char (*board)[SIZE][SIZE]) {
   int horiz;
   printf("Placing battleship\n");
   horiz = orientation();
   if(horiz)
      placeHoriz(SIZE_BATTLESHIP, BATTLESHIP, board);
   else
      placeVert(SIZE_BATTLESHIP, BATTLESHIP, board);
   printf("Successfully placed battleship\n");
   displayBoard(*board);
}

static void placeD(char (*board)[SIZE][SIZE]) {
   int horiz;
   printf("Placing destroyer\n");
   horiz = orientation();
   if(horiz)
      placeHoriz(SIZE_DESTROYER, DESTROYER, board);
   else
      placeVert(SIZE_DESTROYER, DESTROYER, board);
   printf("Successfully placed destroyer\n");
   displayBoard(*board);
}

static void placeS(char (*board)[SIZE][SIZE]) {
   int horiz;
   printf("Placing submarine\n");
   horiz = orientation();
   if(horiz)
      placeHoriz(SIZE_SUBMARINE, SUBMARINE, board);
   else
      placeVert(SIZE_SUBMARINE, SUBMARINE, board);
   printf("Successfully placed submarine\n");
   displayBoard(*board);
}

static void placePB(char (*board)[SIZE][SIZE]) {
   int horiz;
   printf("Placing patrol boat\n");
   horiz = orientation();
   if(horiz)
      placeHoriz(SIZE_PATROL_BOAT, PATROL_BOAT, board);
   else
      placeVert(SIZE_PATROL_BOAT, PATROL_BOAT, board);
   printf("Successfully placed patrol boat\n");
   displayBoard(*board);
}

static void placeShips(char (*board)[SIZE][SIZE]) {
   placeAC(board);
   placeB(board);
   placeD(board);
   placeS(board);
   placePB(board);
}

static void sendBoard(int fd, char (*board)[SIZE][SIZE]) {
placeShips(board);
   if(sizeof(*board) != write(fd, *board, sizeof(*board))) {
      fprintf(stderr, "write failure in %s at line %d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }
}

/* Prompts for a row and column number to shoot.
 * Includes error checking to only allow ships that are inside the board.
 */
static Shot sendShot(int fd) {
   Shot s;
   printf("Your turn to shoot\n");
   while(1) {     // loops until a suitable position is found
      printf("\tEnter the row number [0-%d]: ", SIZE - 1);
      scanf("%hu", &(s.row));
      if(s.row < 0 || s.row > SIZE - 1) {
         printf("\tInvalid row number, please choose from given range.\n");
         clearIn();
      }
      else {
         printf("\tEnter the column number [0-%d]: ", SIZE - 1);
         scanf("%hu", &(s.col));
         clearIn();
         if(s.col < 0 || s.col > SIZE - 1) 
            printf("\tInvalid column number, please choose from given range.\n");
         else {
            if(sizeof(Shot) != write(fd, &s, sizeof(Shot))) {
               fprintf(stderr, "write failure in %s at line %d\n", __FILE__, __LINE__);
               exit(EXIT_FAILURE);
            }
            return s;
         }
      }
   }
}

int main(int argc, char **argv) {
   int readFD, writeFD, in = 0;
   char c;
   Shot lastShot, oppShot;
   char board[SIZE][SIZE], oppBoard[SIZE][SIZE];
   if (argc != 3) {
      fprintf(stderr, "Usage: player readFD writeFD\n");
      exit(EXIT_FAILURE);
   }
   readFD = getFD(argv[1]);
   writeFD = getFD(argv[2]);
   while(1) {
      in = readMsg(readFD);
      if(in == NEW_GAME) {
         clear(&board);
         clear(&oppBoard);
         sendBoard(writeFD, &board);
      }
      else if(in == SHOT_REQUEST) 
         lastShot = sendShot(writeFD);
      else if(in == SHOT_RESULT) {
         in = readMsg(readFD); 
         oppBoard[lastShot.row][lastShot.col] = updateShot(in, lastShot);
         displayBoard(oppBoard);
      }
      else if(in == OPPONENTS_SHOT) {
         if(sizeof(Shot) != read(readFD, &oppShot, sizeof(Shot))) {
            fprintf(stderr, "read failure in %s at line %d\n", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
         }
         printf("Enemy shot [%hu][%hu]\n", oppShot.row, oppShot.col);
         board[oppShot.row][oppShot.col] = updateOppShot(oppShot, board);
         displayBoard(board);
      }
      else if(in == MATCH_OVER)
         break;
   }
   return EXIT_SUCCESS;
}
