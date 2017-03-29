#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "battleship.h"

#define MAX_FD 4
#define MAX_NAME 20

/* Struct used to keep track of various stats.
 */
typedef struct{
   unsigned int wins, losses, draws, hits, misses, sinks;
} Score;

/* Struct used in determining which parts of a ship is hit and when it is sunk.
 */
typedef struct{
   int shotAC, shotB, shotD, shotS, shotPB;
   Shot AC[SIZE_AIRCRAFT_CARRIER];
   Shot B[SIZE_AIRCRAFT_CARRIER];
   Shot D[SIZE_AIRCRAFT_CARRIER];
   Shot S[SIZE_AIRCRAFT_CARRIER];
   Shot PB[SIZE_AIRCRAFT_CARRIER];
} HitCounter;

static void printFileUsage() {
   fprintf(stderr, "Usage: battleship player1 player2\n");
   exit(EXIT_FAILURE);
}

static void closeEnd(int fd) {
   if(close(fd) == -1) {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
}

void setupPipe(int *fd, int write, char *arg) {
   if(pipe(fd)) {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
   if(!write)
      sprintf(arg, "%d", fd[0]);
   else 
      sprintf(arg, "%d", fd[1]);
}

/* Set up the host by closing the pipe ends that aren't used.
 */
void setupHost(int close1, int close2, int close3, int close4) {
   closeEnd(close1);
   closeEnd(close2);
   closeEnd(close3);
   closeEnd(close4);
}

/* Set up a player by closing pipe ends that aren't used and then executing the player.
 */
void setupPlayer(char **args, int index, char *rfd, char *wfd,
   int close1, int close2, int close34[2], int close56[2]) {
   pid_t pid;
   if((pid = fork()) < 0)
      perror(NULL);
   else if(pid == 0) {
      closeEnd(close1);
      closeEnd(close2);
      closeEnd(close34[0]);
      closeEnd(close34[1]);
      closeEnd(close56[0]);
      closeEnd(close56[1]);
      execl(args[index], args[index], rfd, wfd, (char *)0);
      perror(NULL);
      exit(EXIT_FAILURE);
   }
}


static Score setupScore() {
   Score temp;
   temp.wins = temp.losses = temp.draws = 0;
   temp.hits = temp.misses = temp.sinks = 0;
   return temp;
}

static HitCounter setupHit() {
   HitCounter temp;
   temp.shotAC = temp.shotB = temp.shotD = temp.shotS = temp.shotPB = 0;
   return temp;
}

static void writeTo(int fd, int msg) {
   if(sizeof(int) != write(fd, &msg, sizeof(int))) {
      fprintf(stderr, "write failure in %s at line %d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }
}

/* Extract the names of the players from the arguments provided.
 */
static void getName(char (*name)[MAX_NAME], char *arg) {
   int index = strlen(arg) - 1, i = 0;
   if(strchr(arg, '/') == NULL) {
      for(; i < strlen(arg); i++)
         (*name)[i] = arg[i];
   }
   else {
      while(arg[index] != '/')
         index--;
      index++;
      for(; index < strlen(arg); index++) {
         (*name)[i] = arg[index];
         i++;
      }
   }
   (*name)[i] = '\0';
}

static void writeShot(int fd, Shot shot) {
   if(sizeof(Shot) != write(fd, &shot, sizeof(Shot))) {
      fprintf(stderr, "write failure in %s at line %d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }
}

/* Read in a board and store it.
 */
static void readBoard(int fd, char (*board)[SIZE][SIZE]) {
   int bytesRead = 0, i = 0, j = 0;
   char temp;
   for(; bytesRead < (SIZE*SIZE); bytesRead++) {
      if(1 != read(fd, &temp, 1)) {
         fprintf(stderr, "read failure in %s at line %d\n", __FILE__, __LINE__);
         exit(EXIT_FAILURE);
      }      
      (*board)[i][j] = temp;
      if(j < (SIZE-1))
         j++;
      else {
         i++;
         j = 0;
      }
   }
}

static int outOfBounds(Shot s) {
   return (s.row >= SIZE || s.row < 0 || s.col >= SIZE || s.col < 0);
}

/* Comparator for shots based on their row and col values.
 */      
static int compareShots(Shot a, Shot b) {
   if(a.row == b.row && a.col == b.col)
      return 0;
   return 1;
}  

/* Check to see if the shot sunk a particular ship.
 */
static int shipCheck(Shot s, int size, int shot, Shot (*shotShip)[5]) {
   int i = 0;
   if(shot == size)     // if the ship is already sunk, return a hit
      return HIT;
   for(; i <= shot; i++) {
      if(compareShots(s, (*shotShip)[i])) {
         (*shotShip)[shot] = s;
         shot++;
         if(shot == size) 
            return SINK;
         return HIT;
      }
   }
   return HIT;
}


/* Redirects into the previous function with the proper parameters.
 */
static int checkSink(HitCounter *h, Shot s, int hit) {
   int i;
   if(hit == AIRCRAFT_CARRIER) {
      i = shipCheck(s, SIZE_AIRCRAFT_CARRIER, (*h).shotAC, &((*h).AC));
      if(i == SINK || (i == HIT && (*h).shotAC != SIZE_AIRCRAFT_CARRIER))
         (*h).shotAC++;
   }
   else if(hit == BATTLESHIP) {
      i = shipCheck(s, SIZE_BATTLESHIP, (*h).shotB, &((*h).B));
      if(i == SINK || (i == HIT && (*h).shotB != SIZE_BATTLESHIP))
         (*h).shotB++;
   }
   else if(hit == DESTROYER) {
      i = shipCheck(s, SIZE_DESTROYER, (*h).shotD, &((*h).D));
      if(i == SINK || (i == HIT && (*h).shotD != SIZE_DESTROYER))
         (*h).shotD++;
   }
   else if(hit == SUBMARINE) {
      i = shipCheck(s, SIZE_SUBMARINE, (*h).shotS, &((*h).S));
      if(i == SINK || (i == HIT && (*h).shotS != SIZE_SUBMARINE))
         (*h).shotS++;
   }
   else{
      i = shipCheck(s, SIZE_PATROL_BOAT, (*h).shotPB, &((*h).PB));
      if(i == SINK || (i == HIT && (*h).shotPB != SIZE_PATROL_BOAT))
         (*h).shotPB++;
   }
   return i;
}

/* Checks for a win based on the amount of sinks.
 */
static void checkWin(int (*wins)[2], Score *a, Score *b) {
   (*wins)[0] = (*wins)[1] = 0;
   if(a->sinks == 5 && b->sinks == 5) {   // draw when both players win on their last shot
      a->draws++;
      b->draws++;
      (*wins)[0] = (*wins)[1] = 1;
   }
   else if(a->sinks == 5) {
      a->wins++;
      b->losses++;
      (*wins)[0] = 1;
   }
   else if(b->sinks == 5) {
      a->losses++;
      b->wins++;
      (*wins)[1] = 1;
   }
   else {      // both lose if no one sinks all the ships
      a->losses++;
      b->losses++;
   }
}

static void printGameResults(int game, int aWin, int bWin, 
   Score *a, Score *b, char *nA, char *nB) {
   int shots = a->hits + a->misses;
   sleep(0.5);
   printf("\nGame %d Results: ", game);
   if(aWin && bWin)
      printf("Draw\n");
   else if(aWin)
      printf("%s won!\n", nA);
   else if(bWin)
      printf("%s won!\n", nB);
   else
      printf("No winner within %d shots\n", MAX_SHOTS);
   printf("%16s: %d shots, %d hits, and ", nA, shots, a->hits);
   printf("%d sinks\n", a->sinks);
   printf("%16s: %d shots, %d hits, and ", nB, shots, b->hits);
   printf("%d sinks\n", b->sinks);
}

static void printMatchResults(Score a, Score b, char *nA, char *nB) {
   printf("\nMatch Results: ");
   if(a.wins > b.wins)
      printf("%s won!\n", nA);
   else
      printf("%s won!\n", nB);
   printf("%16s: %d wins, %d draws, and ", nA, a.wins, a.draws);
   printf("%d losses\n", a.losses);
   printf("%16s: %d wins, %d draws, and ", nB, b.wins, b.draws);
   printf("%d losses\n", b.losses);
}

/* Determines the shot's results and increments the correct stats.
 * Also writes the results and proper signals to players.
 */
static void processShot(int rfd, int wfd1, int wfd2, Score *score,
   HitCounter *h, char board[SIZE][SIZE], char *name) {
   Shot shot;
   int result;
   if(sizeof(Shot) != read(rfd, &shot, sizeof(Shot))) {
      fprintf(stderr, "read failure in %s at line %d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }
   if(outOfBounds(shot)) {
      result = MISS;
      score->misses++;
   }
   else if(board[shot.row][shot.col] == OPEN_WATER) {
      result = MISS;
      score->misses++;
   }
   else {
      result = checkSink(h, shot, board[shot.row][shot.col]);
      if(result == HIT)
         score->hits++;
      else {
         score->hits++;
         score->sinks++;
      }
   }
   writeTo(wfd1, SHOT_RESULT);
   writeTo(wfd1, result);
   writeTo(wfd2, OPPONENTS_SHOT);
   writeShot(wfd2, shot);
}
/* Main game logic: sends out signals and reads in the responses.
 */
static void gameLoop(int ar, int aw, int br, int bw, Score *sa,
   Score *sb, char *nameA, char *nameB) {
   char boardA[SIZE][SIZE], boardB[SIZE][SIZE];
   HitCounter hitA, hitB;
   int i = 0, shots, winAB[2];
   for(; i < GAMES; i++) {    // plays the amount of games specified in battleship.h
      shots = 0;
      hitA = setupHit();
      hitB = setupHit();
      writeTo(aw, NEW_GAME);
      readBoard(ar, &boardA);
      writeTo(bw, NEW_GAME);
      readBoard(br, &boardB);
      printf("\nGame %d:\n", i+1);
      while(shots < MAX_SHOTS) {
         writeTo(aw, SHOT_REQUEST);
         processShot(ar, aw, bw, sa, &hitA, boardB, nameA);
         writeTo(bw, SHOT_REQUEST);
         processShot(br, bw, aw, sb, &hitB, boardA, nameB);
         shots++;
         if((*sa).sinks == 5 || (*sb).sinks == 5)
            break;
      }
      checkWin(&winAB, sa, sb);
      printGameResults(i+1, winAB[0], winAB[1], sa, sb, nameA, nameB);
      sa->hits = sa->misses = sa->sinks = sb->hits = sb->misses = sb->sinks = 0;
      winAB[0] = winAB[1] = 0;
   }
   writeTo(aw, MATCH_OVER);
   writeTo(bw, MATCH_OVER);
}

/* Calls most of the setup for pipes, players and data structures.
 * There is 2 sets of pipes per player: read and write.
 * The unused ends for the host and players need to be closed. 
 */
int main(int argc, char **argv) {
   Score sA, sB;
   int fdA1[2], fdA2[2], fdB1[2], fdB2[2];
   char rfdA[MAX_FD], wfdA[MAX_FD], nA[MAX_NAME];
   char rfdB[MAX_FD], wfdB[MAX_FD], nB[MAX_NAME];
   if(argc != 3)
      printFileUsage();
   getName(&nA, argv[1]);
   getName(&nB, argv[2]);
   setupPipe(fdA1, 0, rfdA);
   setupPipe(fdA2, 1, wfdA);
   setupPipe(fdB1, 0, rfdB);
   setupPipe(fdB2, 1, wfdB);
   setupPlayer(argv, 1, rfdA, wfdA, fdA1[1], fdA2[0], fdB1, fdB2);
   setupPlayer(argv, 2, rfdB, wfdB, fdB1[1], fdB2[0], fdA1, fdA2);
   setupHost(fdA1[0], fdA2[1], fdB1[0], fdB2[1]);
   sA = setupScore();
   sB = setupScore();
   gameLoop(fdA2[0], fdA1[1], fdB2[0], fdB1[1], &sA, &sB, nA, nB);
   printMatchResults(sA, sB, nA, nB);
   exit(EXIT_SUCCESS);
}

