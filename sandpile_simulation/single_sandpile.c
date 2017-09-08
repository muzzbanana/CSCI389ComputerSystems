#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>


typedef struct _grid_t {
	int *row;
}grid_t;


grid_t *sandpile(int i, int j, int h) {
	static grid_t *grid;
	int m, n;
	m = (j/2);
	n = (i/2);
	grid = (grid_t *)malloc(i*sizeof(grid_t));
	int x, y;
	for (x = 0; x < i; x++) {
		printf("\n");
		grid[x].row = (int *)malloc(i*sizeof(int));
		for (y = 0; y < j; y++){
			if (x == n && y == m) {
				grid[x].row[y] = h;
				printf(" ");
			} else {
				grid[x].row[y] = 0;
				printf(" ");
			}
			printf("%d", grid[x].row[y]);
		}
	}
	printf("\n\n\n\n");
	return grid;
}

bool stability(grid_t *grid, int row, int col){
    int x, y;
    for (x = 0; x < row; x++) {
		for (y = 0; y < col; y++) {
        	if (grid[x].row[y] >= 4)
            	return false;
    	}
   }
   return true;
}

void main (int argc, char** argv) {
	clock_t start = clock();
	int rows = atoi(argv[1]);
	int cols = atoi(argv[2]);
	int height = atoi(argv[3]);
	int x, y;
	grid_t *grid = sandpile(rows, cols, height);
	grid_t *next_grid =sandpile(rows, cols, height);
	int *row = grid->row;
	do {
		for (x = 0; x < rows; x++) {
				for (y = 0; y < cols; y++) {
					if (grid[x].row[y] >= 4) {
						next_grid[x].row[y] = grid[x].row[y] - 4;
					} else {
						next_grid[x].row[y] = grid[x].row[y];
					}
					if (x > 0 && grid[x-1].row[y] >= 4) {
						next_grid[x].row[y] += 1;
					}
					if (x < rows-1 && grid[x+1].row[y] >= 4) {
						next_grid[x].row[y] += 1;
					}
					if (y > 0 && grid[x].row[y-1] >= 4) {
						next_grid[x].row[y] += 1;
					}
					if (y < cols-1 && grid[x].row[y+1] >= 4) {
						next_grid[x].row[y] += 1;
					}
				printf("%d", next_grid[x].row[y]);
				printf(" ");
				}
			printf("\n");
			grid_t *tmp = grid;
			grid = next_grid;
			next_grid = grid;
		}
	printf("\n\n\n");
	} while (stability(next_grid, rows, cols) == false);
	clock_t end = clock();
	double time = ((double)(end - start))/ CLOCKS_PER_SEC;
  	printf("Time: %f secs\n",time);
}

