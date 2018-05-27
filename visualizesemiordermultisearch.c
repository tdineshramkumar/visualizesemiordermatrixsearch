#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <omp.h>
int MAXRANDNUM;
#define MATRIXELEMENTSHOWFORMAT "%3lu"

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

int numthreads =1 ;
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
			printf(" "MATRIXELEMENTSHOWFORMAT, matrix[i][j] );
		printf("\n");
	}
}

void showmatrix( uint64_t ** matrix, int size,int x, int y, bool found, int x1, int x2, int y1, int y2, bool downwards,bool binary) {
	printf("\033[s");
	printf("\n\n");
	if ( binary )
		usleep(200000);
	else 
		usleep(50000);

	for ( int i = 0; i < size; i ++) {
		for ( int j = 0 ; j < size ; j ++)

			if ( i == x && j == y )
				if ( found )
					printf( BG_GREEN " "MATRIXELEMENTSHOWFORMAT RESET, matrix[i][j] );
				else 
					printf( BG_YELLOW " "MATRIXELEMENTSHOWFORMAT RESET, matrix[i][j] );
			else
				if ( i < x1 || i > x2 || j < y1 || j > y2 ) 
					printf(BG_RED " "MATRIXELEMENTSHOWFORMAT RESET, matrix[i][j] );
				else 
					if ( i < x && j == y && downwards)
						printf( BG_CYAN " "MATRIXELEMENTSHOWFORMAT RESET , matrix[i][j] );
					else if ( i == x && j < y && !downwards) 
						printf( BG_CYAN " "MATRIXELEMENTSHOWFORMAT RESET , matrix[i][j] );
					else 
						printf(" "MATRIXELEMENTSHOWFORMAT, matrix[i][j] );
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
				showmatrix(matrix,size,x1,y2,true,x1, x2, y1, tmp, false, false);
				return true;
			}
			else if ( matrix[x1][y2] > element ){
				showmatrix(matrix,size,x1,y2,false, x1, x2, y1, tmp, false, false);
				break ;
			}
			else 
				showmatrix(matrix,size,x1,y2,false, x1, x2, y1, tmp ,false, false);
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
				showmatrix(matrix,size,x1,y2,true, tmp, x2, y1, y2, true, false);
				return true;
			}
			else if ( matrix[x1][y2] > element ){
				showmatrix(matrix,size,x1,y2,false,tmp, x2, y1, y2, true, false);
				break ;
			}
			else 
				showmatrix(matrix,size,x1,y2,false,tmp, x2, y1, y2, true, false);
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
				showmatrix(matrix,size,x2,y1,true, x1, x2, tmp, y2, false, false);
				return true;
			}
			else if ( matrix[x2][y1] > element ){
				showmatrix(matrix,size,x2,y1,false, x1, x2, tmp, y2, false, false);
				break ;
			}
			else 
				showmatrix(matrix,size,x2,y1,false, x1, x2, tmp, y2, false, false);
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
				showmatrix(matrix,size,x2,y1,true, x1, tmp, y1, y2, true, false);
				return true;
			}
			else if ( matrix[x2][y1] > element ){

				showmatrix(matrix,size,x2,y1,false, x1, tmp, y1, y2, true, false);
				break ;
			}
			else 
				showmatrix(matrix,size,x2,y1,false, x1, tmp, y1, y2, true, false);
		}
		x2 --; // this is because we are setting an upper bound ...
		y1 ++; // we already looked at first column
		if ( y1 > y2 || x1 > x2 )
			return false ;
	}
	return false;
}

// bool findindex(uint64_t** matrix, int size, uint64_t element, int x1, int x2, int y1, int y2, int *x, int *y, int ox1, int ox2, int oy1, int oy2){
// 	if ( x1 == x2 ){
// 		for ( int i = y1 ; i <= y2 ; i ++ ) {
// 			if ( matrix[x1][i] == element ) {
// 				*x = x1; *y = i ;
// 				showmatrix(matrix,size,*x,*y,true,ox1, ox2, oy1, oy2, false);
// 				return true;
// 			}
// 			else if ( matrix[x1][i] > element ){
// 				*x = x1 ; *y = i -1;
// 				showmatrix(matrix,size,*x,*y,false,ox1, ox2, oy1, oy2, false);
// 				return false;
// 			}
// 			showmatrix(matrix,size,x1,i,false,ox1, ox2, oy1, oy2, false);	
// 			*x = x1 ; *y = i ;
// 		}
// 	} 
// 	else if ( y1 == y2 ){
// 		for ( int i = x1 ; i <= x2 ; i ++ ) {
// 			if ( matrix[i][y1] == element ) {
// 				*x = i; *y = y1 ;
// 				showmatrix(matrix,size,*x,*y,true,ox1, ox2, oy1, oy2, true);
// 				return true;
// 			}
// 			else if ( matrix[i][y1] > element ){
// 				*x = i -1 ; *y = y1;
// 				showmatrix(matrix,size,*x,*y,false,ox1, ox2, oy1, oy2, true);
// 				return false;
// 			}
// 			showmatrix(matrix,size,i,y1,false,ox1, ox2, oy1, oy2, true);
// 			*x = i ; *y = y1 ;	
// 		}
// 	} 
	
// 	return false;
// }
bool findindex(uint64_t** matrix, int size, uint64_t element, int x1, int x2, int y1, int y2, int *x, int *y, int ox1, int ox2, int oy1, int oy2, int *comparisons){
	if ( x1 == x2 ){
		int left= y1, right=y2, middle;
		bool leftmoved = false;
		while ( left <= right ){
			(*comparisons) ++;
			middle = (left + right)/2 ;
			showmatrix(matrix,size, x1, middle,false, ox1, ox2, oy1, oy2, false, true);
			if ( matrix[x1][middle] == element) {
				*x = x1; *y = middle ;
				showmatrix(matrix,size,*x,*y,true,ox1, ox2, oy1, oy2, false, true);
				return true;
			}
			else if ( matrix[x1][middle] < element ){
				left = middle + 1;
				leftmoved = true;
			}
			else {
				right = middle - 1;
				leftmoved = false;
			}
			if ( left > right ){
				// if left move to right look for greater element
				// if right moved we are looking for smaller element
				if ( leftmoved ) {
					*x = x1 ; *y = right; 
				}
				else {
					*x = x1 ; *y = right;
				}
			}

		} 
		// for ( int i = y1 ; i <= y2 ; i ++ ) {

		// 	if ( matrix[x1][i] == element ) {
		// 		*x = x1; *y = i ;
		// 		showmatrix(matrix,size,*x,*y,true,ox1, ox2, oy1, oy2, false);
		// 		return true;
		// 	}
		// 	else if ( matrix[x1][i] > element ){
		// 		*x = x1 ; *y = i -1;
		// 		showmatrix(matrix,size,*x,*y,false,ox1, ox2, oy1, oy2, false);
		// 		return false;
		// 	}
		// 	showmatrix(matrix,size,x1,i,false,ox1, ox2, oy1, oy2, false);	
		// 	*x = x1 ; *y = i ;
		// }
	} 
	else if ( y1 == y2 ){
		int left= x1, right= x2, middle;
		bool leftmoved = false;
		while ( left <= right ){
			(*comparisons) ++;
			middle = (left + right)/2 ;
			showmatrix(matrix,size, middle, y1, false, ox1, ox2, oy1, oy2, true, true);
			if ( matrix[middle][y1] == element) {
				*x = middle; *y = y1 ;
				showmatrix(matrix,size,*x,*y,true,ox1, ox2, oy1, oy2, true, true);
				return true;
			}
			else if ( matrix[middle][y1] < element ){
				left = middle + 1;
				leftmoved = true;
			}
			else {
				right = middle - 1;
				leftmoved = false;
			}
			if ( left > right ){
				// if left move to right look for greater element
				// if right moved we are looking for smaller element
				if ( leftmoved ) {
					*x = right ; *y = y1; 
				}
				else {
					*x = right ; *y = y1;
				}
			}

		} 
		// for ( int i = x1 ; i <= x2 ; i ++ ) {
		// 	if ( matrix[i][y1] == element ) {
		// 		*x = i; *y = y1 ;
		// 		showmatrix(matrix,size,*x,*y,true,ox1, ox2, oy1, oy2, true);
		// 		return true;
		// 	}
		// 	else if ( matrix[i][y1] > element ){
		// 		*x = i -1 ; *y = y1;
		// 		showmatrix(matrix,size,*x,*y,false,ox1, ox2, oy1, oy2, true);
		// 		return false;
		// 	}
		// 	showmatrix(matrix,size,i,y1,false,ox1, ox2, oy1, oy2, true);
		// 	*x = i ; *y = y1 ;	
		// }
	} 
	
	return false;
}


bool findbs(uint64_t **matrix, int size, uint64_t element, int *x, int *y, int *comparisons, int *depth){
	int x1 = 0, y1 = 0, x2 = size -1 , y2 = size -1 ;
	*depth = 0; *comparisons = 0;
	while ( x1 <= x2 && y1 <= y2 )  {
		(*depth)++;
		// Find an element just greater than element in first row and remove that entire row
		if ( findindex(matrix, size, element, x1, x1, y1, y2, x, y, x1, x2, y1, y2, comparisons) ) return true;// return if found along first row...
		// else need to update the search space 
		// since first row already searched remove it..
		x1 ++;
		// also if (x,y) indicate element less than it ...
		y2 = *y;
		if ( y2 < y1 || x2 < x1 ) return false;
		if ( findindex(matrix, size, element, x1, x2, y2, y2, x, y, x1, x2, y1, y2, comparisons) ) return true; // last column ...
		x1 = *x + 1 ; y2 --; // get the next element ...
		if ( y2 < y1 || x2 < x1 ) return false;
		if ( findindex(matrix, size,element, x2, x2, y1, y2, x, y, x1, x2, y1, y2, comparisons) ) return true; // last column ...
		x2 -- ; y1 = *y + 1;
		if ( y2 < y1 || x2 < x1 ) return false;
		if ( findindex(matrix, size, element, x1, x2, y1, y1, x, y, x1, x2, y1, y2, comparisons) ) return true; // last column ...
		y1 ++; x2 = *x ; 
		if ( y2 < y1 || x2 < x1 ) return false;
	}
	return false;
}
omp_lock_t lock;
// currently it supports only vertical striping of data ...
void showmatrix_p( uint64_t ** matrix, int size,int x, int y, bool found, int x1, int x2, int y1, int y2,
	bool downwards,bool binary,int ox1, int ox2 ) {
	if ( binary )
		usleep(50000);
	else 
		usleep(25000);
	omp_set_lock(&lock);
	printf("\033[s");
	printf("\n");
	printf( MOVEBEGINLINE REMOVETOENDLINE "TID:%d OX1:%d OX2:%d\n", omp_get_thread_num(),ox1,ox2);
	
/*
	 Move the cursor forward N columns:
  \033[<N>C
- Move the cursor backward N columns:
  \033[<N>D
*/
	for ( int i = 0; i < size; i ++) {
		if ( !( i < ox1 || i > ox2 ) )
			for ( int j = 0 ; j < size ; j ++)

					if ( i == x && j == y )
						if ( found )
							printf( BG_GREEN " "MATRIXELEMENTSHOWFORMAT RESET, matrix[i][j] );
						else 
							printf( BG_YELLOW " "MATRIXELEMENTSHOWFORMAT RESET, matrix[i][j] );
					else
						if ( i < x1 || i > x2 || j < y1 || j > y2 ) 
							printf(BG_RED " "MATRIXELEMENTSHOWFORMAT RESET, matrix[i][j] );
						else 
							if ( i < x && j == y && downwards)
								printf( BG_CYAN " "MATRIXELEMENTSHOWFORMAT RESET , matrix[i][j] );
							else if ( i == x && j < y && !downwards) 
								printf( BG_CYAN " "MATRIXELEMENTSHOWFORMAT RESET , matrix[i][j] );
							else 
								printf(" "MATRIXELEMENTSHOWFORMAT, matrix[i][j] );
		printf("\n");
	}
	printf("\033[u");
	omp_unset_lock(&lock);
}
// sx1 sx2  -> For parallel regions for printing...
bool findindex_p(uint64_t** matrix, int size, uint64_t element, int x1, int x2, int y1, int y2, 
		int *x, int *y, int ox1, int ox2, int oy1, int oy2, int sx1, int sx2){
	if ( x1 == x2 ){
		int left= y1, right=y2, middle;
		bool leftmoved = false;
		while ( left <= right ){
			middle = (left + right)/2 ;
			showmatrix_p(matrix,size, x1, middle,false, ox1, ox2, oy1, oy2, false, true, sx1, sx2);
			if ( matrix[x1][middle] == element) {
				*x = x1; *y = middle ;
				showmatrix_p(matrix,size,*x,*y,true,ox1, ox2, oy1, oy2, false, true, sx1, sx2);
				return true;
			}
			else if ( matrix[x1][middle] < element ){
				left = middle + 1;
				leftmoved = true;
			}
			else {
				right = middle - 1;
				leftmoved = false;
			}
			if ( left > right ){
				// if left move to right look for greater element
				// if right moved we are looking for smaller element
				*x = x1 ; *y = right; 
			}

		} 
	} 
	else if ( y1 == y2 ){
		int left= x1, right= x2, middle;
		bool leftmoved = false;
		while ( left <= right ){
			middle = (left + right)/2 ;
			showmatrix_p(matrix,size, middle, y1, false, ox1, ox2, oy1, oy2, true, true, sx1, sx2);
			if ( matrix[middle][y1] == element) {
				*x = middle; *y = y1 ;
				showmatrix_p(matrix,size,*x,*y,true,ox1, ox2, oy1, oy2, true, true, sx1, sx2);
				return true;
			}
			else if ( matrix[middle][y1] < element ){
				left = middle + 1;
				leftmoved = true;
			}
			else {
				right = middle - 1;
				leftmoved = false;
			}
			if ( left > right ){
				// if left move to right look for greater element
				// if right moved we are looking for smaller element
				*x = right ; *y = y1; 
				
			}

		} 
	} 
	
	return false;
}

bool findbs_p(uint64_t **matrix, int size, uint64_t element, int *xfinal, int *yfinal){
	bool found = false;
	//int numthreads = 4;// set number of threads here
	omp_init_lock(&lock);
	#pragma omp parallel num_threads(numthreads) shared(found)
	{
		int x, y;
		int index = omp_get_thread_num();
		int x1 = (size*index/numthreads);
		int y1 =0, y2 = size -1; 
		int	x2 = ( index == numthreads -1  ?  size - 1 : (size*(index+1)/numthreads) -1 );
		int X1 = x1, X2 = x2 ;
		//printf("Thread index: %d x1:%d x2:%d y1:%d y2:%d\n",index, x1, x2, y2, y2 );
		while ( x1 <=  x2 && y1 <= y2 ){
			// Find an element just greater than element in first row and remove that entire row
			if ( findindex_p(matrix, size, element, x1, x1, y1, y2, &x, &y, x1, x2, y1, y2, X1, X2) ) {
				#pragma omp atomic write
				found = true;// return if found along first row...
				*xfinal = x;
				*yfinal = y;
				break;
			}
			// else need to update the search space 
			// since first row already searched remove it..
			x1 ++;
			// also if (x,y) indicate element less than it ...
			y2 = y;
			if ( y2 < y1 || x2 < x1 ) 
				break;
			if ( findindex_p(matrix, size, element, x1, x2, y2, y2, &x, &y, x1, x2, y1, y2, X1, X2) ) {
				#pragma omp atomic write
				found =  true; // last column ...
				*xfinal = x;
				*yfinal = y;
				break;
			}
			x1 = x + 1 ; y2 --; // get the next element ...
			if ( y2 < y1 || x2 < x1 ) 
				break;
			if ( findindex_p(matrix, size,element, x2, x2, y1, y2, &x, &y, x1, x2, y1, y2, X1, X2) ) {
				#pragma omp atomic write
				found = true; // last column ...
				*xfinal = x;
				*yfinal = y;
				break;
			}
			x2 -- ; y1 = y + 1;
			if ( y2 < y1 || x2 < x1 ) 
				break;
			if ( findindex_p(matrix, size, element, x1, x2, y1, y1, &x, &y, x1, x2, y1, y2, X1, X2) ) {
				#pragma omp atomic write
				found = true; // last column ...
				*xfinal = x;
				*yfinal = y;
				break;
			}
			y1 ++; x2 = x ; 
			if ( y2 < y1 || x2 < x1 ) 
				break;
		}
	}
	omp_destroy_lock(&lock);
	return found;
}
bool findbs_p2(uint64_t **matrix, int size, uint64_t element, int *xfinal, int *yfinal){
	bool found = false;
	// int numthreads = 2;
	omp_init_lock(&lock);
	// Do first iteration outsize... (to reduce search space....)
	// Find an element just greater than element in first row and remove that entire row
	int ix1=0, ix2=size-1, iy1=0, iy2=size-1;
	if ( findindex_p(matrix, size, element, ix1, ix1, iy1, iy2, xfinal, yfinal, ix1, ix2, iy1, iy2, 0, size-1) ) return true;// return if found along first row...
	ix1 ++;
	iy2 = *yfinal;
	if ( iy2 < iy1 || ix2 < ix1 ) return false;
	if ( findindex_p(matrix, size, element, ix1, ix2, iy2, iy2, xfinal, yfinal, ix1, ix2, iy1, iy2,  0, size-1) ) return true; // last column ...
	ix1 = *xfinal + 1 ; iy2 --; // get the next element ...
	if ( iy2 < iy1 || ix2 < ix1 ) return false;
	if ( findindex_p(matrix, size,element, ix2, ix2, iy1, iy2, xfinal, yfinal, ix1, ix2, iy1, iy2,  0, size-1) ) return true; // last column ...
	ix2 -- ; iy1 = *yfinal + 1;
	if ( iy2 < iy1 || ix2 < ix1 ) return false;
	if ( findindex_p(matrix, size, element, ix1, ix2, iy1, iy1, xfinal, yfinal, ix1, ix2, iy1, iy2,  0,size-1) ) return true; // last column ...
	iy1 ++; ix2 = *xfinal ; 
	if ( iy2 < iy1 || ix2 < ix1 ) return false;
	// Just for  showing
	showmatrix_p(matrix,size,-1,-1,false, ix1, ix2, iy1, iy2, false, true, 0, size-1);
	#pragma omp parallel num_threads(numthreads) shared(found)
	{
		int x, y;
		int asize = ix2 - ix1 + 1;
		int index = omp_get_thread_num();
		int x1 = ix1 + (asize*index/numthreads);
		int y1 = iy1, y2 = iy2 ; 
		int	x2 = ( index == numthreads -1  ?  ix2 : ix1 + (asize*(index+1)/numthreads) -1 );
		int X1 = x1, X2 = x2 ;
		//printf("Thread index: %d x1:%d x2:%d y1:%d y2:%d\n",index, x1, x2, y2, y2 );
		while ( x1 <=  x2 && y1 <= y2 && !found ){
			// Find an element just greater than element in first row and remove that entire row
			if ( findindex_p(matrix, size, element, x1, x1, y1, y2, &x, &y, x1, x2, y1, y2, X1, X2) ) {
				#pragma omp atomic write
				found = true;// return if found along first row...
				*xfinal = x;
				*yfinal = y;
				break;
			}
			// else need to update the search space 
			// since first row already searched remove it..
			x1 ++;
			// also if (x,y) indicate element less than it ...
			y2 = y;
			if ( y2 < y1 || x2 < x1 ) 
				break;
			if ( found ) break ; // in case some thread finishes
			if ( findindex_p(matrix, size, element, x1, x2, y2, y2, &x, &y, x1, x2, y1, y2, X1, X2) ) {
				#pragma omp atomic write
				found =  true; // last column ...
				*xfinal = x;
				*yfinal = y;
				break;
			}
			x1 = x + 1 ; y2 --; // get the next element ...
			if ( y2 < y1 || x2 < x1 ) 
				break;
			if ( found ) break ; // in case some thread finishes
			if ( findindex_p(matrix, size,element, x2, x2, y1, y2, &x, &y, x1, x2, y1, y2, X1, X2) ) {
				#pragma omp atomic write
				found = true; // last column ...
				*xfinal = x;
				*yfinal = y;
				break;
			}
			x2 -- ; y1 = y + 1;
			if ( y2 < y1 || x2 < x1 ) 
				break;
			if ( found ) break ; // in case some thread finishes
			if ( findindex_p(matrix, size, element, x1, x2, y1, y1, &x, &y, x1, x2, y1, y2, X1, X2) ) {
				#pragma omp atomic write
				found = true; // last column ...
				*xfinal = x;
				*yfinal = y;
				break;
			}
			y1 ++; x2 = x ; 
			if ( y2 < y1 || x2 < x1 ) 
				break;
		}
	}
	omp_destroy_lock(&lock);
	return found;
}
void signalhandler(int signo){
	printf(CLEARSCREEN TOPLEFTCORNER);
	exit(EXIT_SUCCESS);
}
int main(int argc, char *argv[]) {
	if ( argc != 5 ){
		fprintf(stderr,"Format: executable size maxrandnum mode numthreads\nmode:1 SEQUENTIAL LINEAR \nmode:2 SEQUENTIAL BINANY\nmode:3 PARALLEL BINARY \nmode:4 PARALLEL BINARY\n");
		exit(EXIT_SUCCESS);
	}
	numthreads = atoi(argv[4]);
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
	printf("\n\n\n");
		printmatrix(matrix,size);
	while ( 1 ) {
		//printf(TOPLEFTCORNER );
		printf( TOPLEFTCORNER REMOVETOENDLINE "Enter element:" );
		scanf("%lu",&element);
		switch ( atoi(argv[3]) ) {
			case 1:
				if ( find ( matrix, size, element, &x, &y, &comparisons, &depth ) ){
					printf("\033[B" MOVEBEGINLINE  REMOVETOENDLINE "Found at (%d, %d). comparisons: %d depth: %d " "\033[A",x,y,comparisons, depth );
				}
				else {
					printf("\033[B" MOVEBEGINLINE  REMOVETOENDLINE "Not found. comparisons: %d depth: %d" "\033[A",comparisons, depth );
				}
				break;
			case 2:
				if ( findbs( matrix, size, element, &x, &y, &comparisons, &depth ) ){
					printf(REMOVETOENDLINE "Found at (%d, %d). comparisons: %d depth: %d ",x,y,comparisons, depth );
				}
				else {
					printf(REMOVETOENDLINE "Not found. comparisons: %d depth: %d",comparisons, depth );
				}
				break;
			case 3:
				if ( findbs_p( matrix, size, element, &x, &y ) ){
					printf(REMOVETOENDLINE "Found at (%d, %d).",x,y);
				}
				else {
					printf(REMOVETOENDLINE "Not found.");
				}
				break;
			case 4: 
				// Use this to parallelize after first iteration
				if ( findbs_p2( matrix, size, element, &x, &y ) ){
					printf(REMOVETOENDLINE "Found at (%d, %d).",x,y);
				}
				else {
					printf(REMOVETOENDLINE "Not found.");
				}
				break;
			default:
				return 0;
		}
		
 	}
 	return 0;
}
