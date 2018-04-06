#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

int MAXRANDNUM;
#define MATRIXELEMENTSHOWFORMAT "%4lu"

#define FG_BLACK "\033[30m"
#define FG_RED "\033[31m"
#define FG_GREEN "\033[32m"
#define FG_YELLOW "\033[33m"
#define FG_BLUE "\033[34m"
#define FG_MAGENTA "\033[35m"
#define FG_CYAN "\033[36m"
#define FG_WHITE "\033[37m"
#define FG_DEFAULT "\033[39m"

#define BG_BLACK "\033[40m"
#define BG_RED "\033[41m"
#define BG_GREEN "\033[42m"
#define BG_YELLOW "\033[43m"
#define BG_BLUE "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN "\033[46m"
#define BG_WHITE "\033[47m"
#define BG_DEFAULT "\033[49m"
#define RESET "\033[0m"

#define TOPLEFTCORNER "\033[H" 
#define CLEARSCREEN "\033[2J" 
#define REMOVETOENDLINE "\033[K"
#define HIDECURSOR "\033[?25l"
#define MOVENLINESDOWN "\033[%dB"
#define MOVEBEGINLINE "\r"

// Note: Not checking for overflow may result in error
uint64_t ** generatematrix(int size){
	uint64_t ** matrix = malloc(sizeof(uint64_t *) * size);
	srand(time(NULL));
	for ( int i = 0 ; i < size; i ++ )
		matrix[i] = malloc(sizeof(uint64_t) * size);
	for ( int i = 0 ; i < size ; i ++ ) 
		for ( int j = 0; j < size; j ++ ) 
			if ( i == 0 && j == 0 )
				matrix[i][j] = rand() % MAXRANDNUM;
			else if ( j == 0 )
				matrix[i][j] = matrix[i-1][j] + ( rand() % MAXRANDNUM);
			else if ( i == 0 )
				matrix[i][j] = matrix[i][j-1] + ( rand() % MAXRANDNUM);
			else
				matrix[i][j] = ( rand() % MAXRANDNUM) + (matrix[i-1][j] > matrix[i][j-1] ? matrix[i-1][j]: matrix[i][j-1] ) ; 
	return matrix;
}
void printmatrix( uint64_t ** matrix, int size ) {
	for ( int i = 0; i < size; i ++) {
		for ( int j = 0 ; j < size ; j ++)
			printf(" "MATRIXELEMENTSHOWFORMAT" ", matrix[i][j] );
		printf("\n");
	}
}

void showmatrix( uint64_t ** matrix, int size,int x, int y, bool found, int x1, int x2, int y1, int y2, bool downwards) {
	printf("\033[s");
	printf("\n");
	usleep(20000);
	for ( int i = 0; i < size; i ++) {
		for ( int j = 0 ; j < size ; j ++)

			if ( i == x && j == y )
				if ( found )
					printf( BG_GREEN " "MATRIXELEMENTSHOWFORMAT" " RESET, matrix[i][j] );
				else 
					printf( BG_YELLOW " "MATRIXELEMENTSHOWFORMAT" " RESET, matrix[i][j] );
			else
				if ( i < x1 || i > x2 || j < y1 || j > y2 ) 
					printf(BG_RED " "MATRIXELEMENTSHOWFORMAT" " RESET, matrix[i][j] );
				else 
					if ( i < x && j == y && downwards)
						printf( BG_CYAN " "MATRIXELEMENTSHOWFORMAT" " RESET , matrix[i][j] );
					else if ( i == x && j < y && !downwards) 
						printf( BG_CYAN " "MATRIXELEMENTSHOWFORMAT" " RESET , matrix[i][j] );
					else 
						printf(" "MATRIXELEMENTSHOWFORMAT" ", matrix[i][j] );
		printf("\n");
	}
	printf("\033[u");
}
// this is the sequential algorithm doing a linear search instead of binary to just check for correctness
// of algorithm
bool find(uint64_t **matrix, int size, uint64_t element, int *x, int *y, int *numcomparisons, int *depth){
	int x1=0, x2=size-1, y1=0, y2=size-1;
	int tmp;
	*numcomparisons = 0;
	(*depth) = 0;
	while ( y1 <= y2 && x1 <= x2 ) {
		(*depth) ++;
		tmp= y2 ;
		// First look at first row ...
		for ( y2 = y1 ; y2 <= tmp; y2 ++){
			(*numcomparisons) ++;
			if ( matrix[x1][y2] == element ){
				*x = x1 ;
				*y = y2 ;
				showmatrix(matrix,size,x1,y2,true,x1, x2, y1, tmp, false);
				return true;
			}
			else if ( matrix[x1][y2] > element ){
				showmatrix(matrix,size,x1,y2,false, x1, x2, y1, tmp, false);
				break ;
			}
			else 
				showmatrix(matrix,size,x1,y2,false, x1, x2, y1, tmp ,false);
		}
			
		y2 --; // move back .. (to an element less than element or to end)
		x1 ++; // move to next row ..
		if ( y1 > y2 || x1 > x2 ) 
			return false;
		// Next look at last column ...
		tmp = x1 ;
		for ( x1 = x1 ; x1 <= x2 ; x1 ++ ){
			(*numcomparisons) ++;
			if ( matrix[x1][y2] == element ){
				*x = x1 ;
				*y = y2 ;
				showmatrix(matrix,size,x1,y2,true, tmp, x2, y1, y2, true);
				return true;
			}
			else if ( matrix[x1][y2] > element ){
				showmatrix(matrix,size,x1,y2,false,tmp, x2, y1, y2, true);
				break ;
			}
			else 
				showmatrix(matrix,size,x1,y2,false,tmp, x2, y1, y2, true);
		}
		// No change in x1
		y2 --; // move one column to left (as we have already looked at the column)
		if ( y1 > y2 || x1 > x2 )
			return false;
		// Top row 
		tmp = y1 ;
		for ( y1 = y1 ; y1 <= y2 ; y1 ++ ){
			(*numcomparisons) ++;
			if ( matrix[x2][y1] == element ){
				*x = x2 ;
				*y = y1 ;
				showmatrix(matrix,size,x2,y1,true, x1, x2, tmp, y2, false);
				return true;
			}
			else if ( matrix[x2][y1] > element ){
				showmatrix(matrix,size,x2,y1,false, x1, x2, tmp, y2, false);
				break ;
			}
			else 
				showmatrix(matrix,size,x2,y1,false, x1, x2, tmp, y2, false);
		}
		// No change in y1 ..
		x2 --; // we looked at this row ..
		// First column
		tmp = x2 ;
		for ( x2 = x1 ; x2 <= tmp ; x2 ++ ){
			(*numcomparisons) ++;
			if ( matrix[x2][y1] == element ){
				*x = x2 ;
				*y = y1 ;
				showmatrix(matrix,size,x2,y1,true, x1, tmp, y1, y2, true);
				return true;
			}
			else if ( matrix[x2][y1] > element ){

				showmatrix(matrix,size,x2,y1,false, x1, tmp, y1, y2, true);
				break ;
			}
			else 
				showmatrix(matrix,size,x2,y1,false, x1, tmp, y1, y2, true);
		}
		x2 --; // this is because we are setting an upper bound ...
		y1 ++; // we already looked at first column
		if ( y1 > y2 || x1 > x2 )
			return false ;
	}
	return false;
}

void signalhandler(int signo){
	printf(CLEARSCREEN TOPLEFTCORNER);
	exit(EXIT_SUCCESS);
}
int main(int argc, char *argv[]) {
	if ( argc != 3 ) { printf("format: executable size maxrandom.\n"); exit(EXIT_SUCCESS); }
	int size = atoi(argv[1]);
	MAXRANDNUM = atoi(argv[2]);
	uint64_t **matrix = generatematrix(size);
	// printmatrix(matrix,size);
	uint64_t element ;
	int x, y ;
	signal(SIGINT, signalhandler);
	signal(SIGQUIT, signalhandler);
	signal(SIGTSTP, signalhandler);
	int comparisons, depth;
	printf(CLEARSCREEN TOPLEFTCORNER);
	printf("\n\n");
		printmatrix(matrix,size);
	while ( 1 ) {
		//printf(TOPLEFTCORNER );
		
		printf( TOPLEFTCORNER REMOVETOENDLINE "Enter element:" );
		scanf("%lu",&element);
		if ( find( matrix, size, element, &x, &y,&comparisons,&depth ) ){
			printf(REMOVETOENDLINE "Found at (%d, %d) from %d comparisons %d depth\n",x,y, comparisons, depth );
		}
		else {
			printf(REMOVETOENDLINE "Not found from %d comparisons %d depth.\n", comparisons, depth);
		}
 	}
 	return 0;
}
