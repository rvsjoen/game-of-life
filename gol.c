#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CELL_DEAD  0
#define CELL_ALIVE 1

int** world[2];
int current = 0;

void exit_gracefully(){
	endwin();
}

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

void zero(int** cworld, int x, int y){
		int i;
		for(i=0;i<y;i++)
			memset(cworld[i], CELL_DEAD, sizeof(int)*x);
}

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

int** tick(int width, int height){
	int** cworld = world[current];
	current = ( current + 1 ) % 2;
	int** nworld = world[current];

	int x, y;
	for(x=0; x<width; x++){
		for(y=0; y<height; y++){
			int cnt = 0;

			if(y>0){
				if(x>0)
					if(cworld[y-1][x-1] == CELL_ALIVE) cnt++;
				if(cworld[y-1][x] == CELL_ALIVE) cnt++;
				if(x<width)
					if(cworld[y-1][x+1] == CELL_ALIVE) cnt++;
			}

			if(x>0)
				if(cworld[y][x-1] == CELL_ALIVE) cnt++;
			if(x<width)
				if(cworld[y][x+1] == CELL_ALIVE) cnt++;

			if(y<(height-1)){
				if(x>0)
					if(cworld[y+1][x-1] == CELL_ALIVE) cnt++;
				if(cworld[y+1][x] == CELL_ALIVE) cnt++;
				if(x<width)
					if(cworld[y+1][x+1] == CELL_ALIVE) cnt++;
			}

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

	int width, height;
	getmaxyx(w, height, width);
	width 	-= 1;
	height 	-= 1;

	init(width, height);
	readfile(argv[1], width, height);

	int** cworld = world[0]; 
	while(true){
		int x = 0;
		int y = 0;

		while(x<(width-1) && y<height)
		{
			move(y+1,x+1);
			if(cworld[y][x])
				waddch(w, ACS_BLOCK);
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
	exit_gracefully();
	return EXIT_SUCCESS;
}
