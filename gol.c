#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Cell states
#define CELL_DEAD  0
#define CELL_ALIVE 1

//Define for compiling in color support
#define ENABLE_COLOR  

#ifdef  ENABLE_COLOR  
//Cell Colors
#define UNDERPOPULATION COLOR_RED
#define SUSTAIN         COLOR_BLUE
#define OVERCROWDING    COLOR_CYAN
#define REPRODUCTION    COLOR_GREEN

//Ncurses pair numbers
#define UND_PAIR  1
#define SUS_PAIR  2
#define OVER_PAIR 3
#define REP_PAIR  4
#endif

//Structure to hold Cell data
typedef struct cell_state {
  int neighbors;
  int status;
} CellState;

int** world[2];
int current = 0;

#ifdef ENABLE_COLOR
void set_color(int** cworld, CellState* state) {
  int pair;

  if ((state->status == CELL_DEAD) 
   && (state->neighbors == 3))
    pair = REP_PAIR;

  else {
    if (state->neighbors < 2)
      pair = UND_PAIR;

    else if ((state->neighbors >= 2) 
          && (state->neighbors <= 3)) 
      pair = SUS_PAIR;

    else if (state->neighbors > 3)
      pair = OVER_PAIR;
  }

  attron(COLOR_PAIR(pair));
}

void init_colors() {
  if (has_colors()) {
    start_color();
    init_pair(UND_PAIR,  UNDERPOPULATION, COLOR_BLACK);
    init_pair(SUS_PAIR,  SUSTAIN,         COLOR_BLACK);
    init_pair(REP_PAIR,  REPRODUCTION,    COLOR_BLACK);
    init_pair(OVER_PAIR, OVERCROWDING,    COLOR_BLACK);
  }
}
#endif

void exit_gracefully(){
	endwin();
}

//Read world from external files
void readfile(char* name, int scr_w, int scr_h){
	int fd;
	if((fd = open(name, O_RDONLY)) == -1 )
		exit_gracefully(), exit(1);

	int istate_w = 0;
	int istate_h = 0;

	char buf;
	while(read(fd, &buf, 1)){
		if(buf == '\n')
			istate_h++, istate_w = 0;
		else
			istate_w++;
	}
	lseek(fd, 0, SEEK_SET);

	int x = 0, y = 0;
	while(read(fd, &buf, 1)){
		if(buf == '\n'){
			y++;
			x=0;
		} else {
			x++;
			if(buf == '1')
				world[0][y+scr_h/2-istate_h/2][x+scr_w/2-istate_w/2] = 1;
		}
	}
	close(fd);
}

//Reset world to blank
void zero(int** cworld, int x, int y){
		int i;
		for(i=0;i<y;i++)
			memset(cworld[i], CELL_DEAD, sizeof(int)*x);
}

//Retreive info for a given cell
CellState* get_cell_state(int **cworld, 
                          int x, 
                          int y, 
                          int width, 
                          int height) 
{
  CellState *state = malloc(sizeof(CellState));
  int cnt = 0;
  
  //Loop through world counting cells
  if(y > 0){
    if(x > 0)
      if(cworld[y-1][x-1] == CELL_ALIVE) cnt++;
    if(cworld[y-1][x] == CELL_ALIVE) cnt++;
    if(x<(width-1))
      if(cworld[y-1][x+1] == CELL_ALIVE) cnt++;
  }

  if((x > 0) && (x < (width-1)))
    if(cworld[y][x-1] == CELL_ALIVE) cnt++;
  if(x < (width-1))
    if(cworld[y][x+1] == CELL_ALIVE) cnt++;

  if(y<(height-1)){
    if(x>0)
      if(cworld[y+1][x-1] == CELL_ALIVE) cnt++;
    if(cworld[y+1][x] == CELL_ALIVE) cnt++;
    if(x<(width-1))
      if(cworld[y+1][x+1] == CELL_ALIVE) cnt++;
  }

  state->neighbors = cnt;
  state->status    = cworld[y][x];

  return state;
}

//Create world
void init(int x, int y){
	int i;
	for(i=0;i<2;i++){
		world[i] = malloc(sizeof(int*)*y);
		int j;
		for(j=0;j<y;j++){
			world[i][j] = malloc(sizeof(int)*x);
		}
		zero(world[i], x, y);
	}
}

//Free world
void freeMem(int x, int y) {
	int i;
	for(i=0;i<2;i++){
      free(world[i]);
	}
}

//Update world
int** tick(int width, int height){
	int** cworld = world[current];
	current = ( current + 1 ) % 2;
	int** nworld = world[current];

	int x, y;
	for(x=0; x<width; x++){
		for(y=0; y<height; y++){
			CellState* state = get_cell_state(cworld, x, y, width, height);
      int cnt = state->neighbors;

			switch(cnt){
				case 0: case 1:
					nworld[y][x] = CELL_DEAD; break;
				case 2: case 3:
					if(cworld[y][x] == CELL_DEAD && cnt == 3)
						nworld[y][x] = CELL_ALIVE;
					else
						nworld[y][x] = cworld[y][x];
					break;
				case 4: case 5: case 6: case 7: case 8:
					nworld[y][x] = CELL_DEAD; break;
			}

      free(state);
		}
	}
	return nworld;
}

int main(int argc, char** argv){
	WINDOW* w = NULL;
	if((w = initscr()) == NULL){
		exit(EXIT_FAILURE);
	}
	cbreak();
	noecho();
	nonl();

  #ifdef ENABLE_COLOR
  init_colors();
  #endif 

	int width, height;
	getmaxyx(w, height, width);
	width 	-= 1;
	height 	-= 1;

	init(width, height);
	readfile(argv[1], width, height);

	int** cworld = world[0]; 
  int frames = 0;
	while(1){
		int x = 0;
		int y = 0;

		while(x<(width-1) && y<height)
		{
			move(y+1,x+1);
			if(cworld[y][x]) {
        #ifdef ENABLE_COLOR
        if (has_colors()) {
          CellState* state = get_cell_state(cworld, x, y, width, height);
          set_color(cworld, state);
          free(state);
        }
        #endif
				waddch(w, ACS_BLOCK);
      }
			else {
				delch();
				waddch(w, ' ');
			}
			x++;
			if(x==(width-1)){
				x = 0;
				y++;
			}
		}
		wrefresh(w);
		usleep(50000);
		cworld = tick(width, height);
	}
  freeMem(width, height);
	exit_gracefully();
	return EXIT_SUCCESS;
}
