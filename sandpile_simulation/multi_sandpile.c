#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _grid_t{
  int* row;
  int stability;
  pthread_mutex_t mutex;
}grid_t;

typedef struct _barrier_t{
  int counter;
  int stability;
  pthread_mutex_t mutex;
  pthread_cond_t condition;
}barrier_t;

//1 = stable, 0 = unstable

typedef struct _tinfo_t{
  pthread_t thread;
  barrier_t* barrier;
  grid_t* grid;
  int thread_id;
  int numthreads;
  int gridsize;
  int region_stability;
  int grid_stability;
  int initial_cell;
  int final_cell;
}tinfo_t;

void barrier_init(barrier_t* b, int numthreads) {
  b->counter = numthreads;
  b->stability = numthreads;
  pthread_cond_init(&b->condition,NULL);
  pthread_mutex_init(&b->mutex, NULL);
}

int barrier_wait(barrier_t* barrier,int numthreads, int stability) {
  pthread_mutex_lock(&barrier->mutex);
  barrier->counter--;
  barrier->stability -= stability;

  if (barrier->counter == 0) {
    pthread_cond_broadcast(&barrier->condition);
    if (barrier->stability == 0) {
      pthread_mutex_unlock(&barrier->mutex);
      return 1;
    }
    barrier->counter = numthreads;
    barrier->stability = numthreads;
  } else {
    pthread_cond_wait(&barrier->condition,&barrier->mutex);
    if (barrier->stability == 0) {
      pthread_mutex_unlock(&barrier->mutex);
      return 1;
    }
  }
  pthread_mutex_unlock(&barrier->mutex);
  return 0;
}


void print_grid(tinfo_t* tinfo) {
  for(int i = 0; i < tinfo->gridsize; i++) {
    printf("\n");
    for(int j = 0; j < tinfo->gridsize; j++) {
      printf("%d ",tinfo->grid[i].row[j]);
    }
  }
  printf("\n");
}


void lock_threads(tinfo_t* tinfo, int rownumber) {
  
  if(rownumber == tinfo->initial_cell) {
    pthread_mutex_lock(&tinfo->grid[rownumber].mutex);
  }
  if(rownumber - 1 <= tinfo->initial_cell && rownumber > 0) {
    pthread_mutex_lock(&tinfo->grid[rownumber-1].mutex);
  }
  if(rownumber + 1 >= tinfo->final_cell - 1 && rownumber < tinfo->gridsize - 1) {
    pthread_mutex_lock(&tinfo->grid[rownumber+1].mutex);
  }
  if(rownumber == tinfo->final_cell - 1) {
    pthread_mutex_lock(&tinfo->grid[rownumber].mutex);
  }
}

void unlock_threads(tinfo_t* tinfo, int rownumber) {
  if(rownumber == tinfo->initial_cell || rownumber == tinfo->final_cell - 1) {
    pthread_mutex_unlock(&tinfo->grid[rownumber].mutex);
  }
  if(rownumber + 1 >= tinfo->final_cell-1 && rownumber < tinfo->gridsize - 1) {
    pthread_mutex_unlock(&tinfo->grid[rownumber+1].mutex);
  }
  if(rownumber - 1 <= tinfo->initial_cell && rownumber > 0) {
    pthread_mutex_unlock(&tinfo->grid[rownumber-1].mutex);
  }
}


void compute(tinfo_t* tinfo) {
  tinfo->region_stability = 1;
  for(int i = tinfo->initial_cell; i < tinfo->final_cell; i++) {
    lock_threads(tinfo,i);
    while(tinfo->grid[i].stability == 0) {
      tinfo->grid[i].stability = 1;
      for(int j = 0; j < tinfo->gridsize; j++) {
      	if(tinfo->grid[i].row[j] >= 4) {
      	  tinfo->grid[i].row[j] -= 4;
      	  tinfo->grid[i].stability = 0;
      	  tinfo->region_stability = 0;
      	  if(i-1 < tinfo->initial_cell || i+1 == tinfo->final_cell) {
      	    tinfo->grid_stability = 0;
      	  }
      	  if (i > 0) {
      	    tinfo->grid[i-1].row[j] += 1;
      	    tinfo->grid[i-1].stability = 0;
      	  }
      	  if (i < tinfo->gridsize - 1) {
      	    tinfo->grid[i+1].row[j] += 1;
      	    tinfo->grid[i+1].stability = 0;
      	  }
      	  if (j > 0) {
      	    tinfo->grid[i].row[j-1] += 1;
      	  }
      	  if(j < tinfo->gridsize - 1) {
      	    tinfo->grid[i].row[j+1] += 1;
      	  }
	      } 
      }
    }
    print_grid(tinfo); 
    unlock_threads(tinfo,i);   
  } 
}


void sandpile_simulation(tinfo_t *tinfo) {
  while(1) { 
    compute(tinfo);
    if(tinfo->region_stability == 1) {
      int stability = tinfo->grid_stability & tinfo->region_stability;
      if(barrier_wait(tinfo->barrier,tinfo->numthreads,stability) == 1) {
	       break;
      }
      // print_grid(tinfo);
      tinfo->grid_stability = 1;
      barrier_wait(tinfo->barrier, tinfo->numthreads,stability);
    }
  }
}

int main(int argc, char **argv) {

  clock_t initial_cell = clock();

  int numthreads = atoi(argv[1]);
  int gridsize = atoi(argv[2]);
  int height = atoi(argv[3]);

  static grid_t* grid;
  grid = (grid_t*)malloc(gridsize*sizeof(grid_t));
  for (int i = 0; i < gridsize; i++) {
    grid[i].stability = 0;
    grid[i].row = (int*)malloc(gridsize*sizeof(int));
    pthread_mutex_init(&grid[i].mutex,NULL);
    for (int j = 0; j < gridsize; j++) {
      grid[i].row[j] = 0;
    }
  }

  grid[gridsize/2].row[gridsize/2] = height;
  tinfo_t tinfo[numthreads];
  static barrier_t barrier;
  barrier_init(&barrier,numthreads);
  for(int i = 0; i < numthreads; i++) {
    tinfo[i].numthreads = numthreads;
    tinfo[i].thread_id = i;
    tinfo[i].barrier = &barrier;
    tinfo[i].grid = grid;
    tinfo[i].initial_cell = (gridsize/(numthreads))*(i);
    tinfo[i].final_cell = (gridsize/(numthreads))*(i+1);
    tinfo[i].gridsize = gridsize;
    tinfo[i].region_stability = 1;
    tinfo[i].grid_stability = 1;
    if(i != 0) {
      pthread_create(&tinfo[i].thread, NULL, (void*(*)(void*))sandpile_simulation, &tinfo[i]);
    }
  }

  sandpile_simulation(&tinfo[0]);
  print_grid(&tinfo[0]);

  clock_t final_cell = clock();
  double time = ((double)(final_cell - initial_cell))/ CLOCKS_PER_SEC;
  printf("Time: %f secs\n",time);
}