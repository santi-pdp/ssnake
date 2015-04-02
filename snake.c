#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define GAME_TITLE "THE SNAKE GAME!"
#define INSTRUCTIONS "Press ENTER to continue. Use the arrow keys to move the snake. Press P to pause the game."
#define LICENCE "Author: Santi PdP"
#define GAME_OVER "GAME OVER"
#define PAUSE_TEXT "PAUSED. PRESS P TO CONTINUE"
#define SNAKE_BODY_PIECE 'o'
#define FOOD_PIECE '+'
#define MAX_LENGTH 1024000
#define DOES_COLLIDE 1
#define DOES_NOT_COLLIDE 0
#define SCORE_UNIT 10
#define OFF 0
#define PLAYING 1
#define GAME_OVER_MENU 2
#define PAUSED 3
#define KEY_SPACE 32
#define KEY_Q 113
#define KEY_P 112

//display structure 
typedef struct{
	unsigned int rows;
	unsigned int columns;
}display_t;

//store the vector to know where to create next point
typedef struct{
	unsigned int vx, vy;
}vector_t;

//point structure
typedef struct{
	unsigned int x,y;
}point_t;

//snake structure that has a vector of change and many positions
typedef struct{
	vector_t *vector;
	point_t **positions;
	unsigned int length;
}snake_t;

typedef struct{
	snake_t *snake;
	display_t *window;
	point_t *food_piece;
	unsigned int score;
	int in_game;
}game_status_t;

/* WRITE LOG
 *************/
void write_log_panel(const char *log_message, display_t *window){
		mvprintw(window->rows-2,2,"%s\n",log_message);
		refresh();
}

/* PRINT TITLE
 **************/
void print_title(display_t *window){
	clear();
	mvprintw(window->rows/2,(window->columns-strlen(GAME_TITLE))/2,"%s",GAME_TITLE);
	mvprintw(window->rows/2+2,(window->columns-strlen(INSTRUCTIONS))/2, "%s", INSTRUCTIONS);
	mvprintw(window->rows-3, (window->columns-strlen(LICENCE))/2, "%s", LICENCE);
	mvprintw(window->rows-2,2,"%dx%d\n",window->rows,window->columns);
	box(stdscr, '#', '#');
	refresh();
    	getch();
}

/* ADD FOOD PIECE
 *****************/
//Generate a random point inside the window
void add_food_piece(point_t *f_point, display_t *window){
	unsigned int max_x = window->columns-2;
	unsigned int min_x = 2;
	unsigned int max_y = window->rows-2;
	unsigned int min_y = 2;
	f_point->x = (rand()%(max_x-min_x))+min_x;
	f_point->y = (rand()%(max_y-min_y))+min_y;
}

/* COLLISION WITH WALLS
 ***********************/
//check whether the snake hits a wall or not
unsigned int collision_with_walls(snake_t *snake, display_t *window){
	return (snake->positions[0]->x > window->columns-1 || snake->positions[0]->x < 1 ||
		snake->positions[0]->y > window->rows-1 || snake->positions[0]->y < 1);
}

/* HITS FOOD
 ************/
//check whether the snake hits the food or not
unsigned int hit_food(snake_t *snake, point_t *food_piece){
	return (snake->positions[0]->x == food_piece->x)&&(snake->positions[0]->y == food_piece->y);
}

/* HITS ITSELF
 ***********************/
unsigned int hits_itself(snake_t *snake){
	unsigned int k;
	for(k=1;k<snake->length;k++){
		if(snake->positions[k]){
			if(snake->positions[k]->x == snake->positions[0]->x && snake->positions[k]->y == snake->positions[0]->y){
				//hits head with tail!
				return DOES_COLLIDE;
			}
		}
	}
	return DOES_NOT_COLLIDE;	
}

/* MOVES VERTICAL
 *****************/
unsigned int snake_moves_vertical(snake_t *snake){
	return snake->vector->vy;
}

/* MOVES HORIZONTAL
 *******************/
unsigned int snake_moves_horizontal(snake_t *snake){
	return snake->vector->vx;
}

void init_snake(snake_t *snake, display_t *window){
	//set up the vector
	vector_t *v = (vector_t*)calloc(1,sizeof(vector_t));
	v->vx = 1;
	v->vy = 0;
	
	//set up the initial snake point
	point_t *init_p = (point_t*)calloc(1,sizeof(point_t));
	init_p->x = (unsigned int)round(window->columns/2 - 1);
	init_p->y = (unsigned int)round(window->rows/2);
	
	//set up the snake
	snake->vector = v;
	snake->positions = (point_t **)calloc(1,MAX_LENGTH*sizeof(point_t));
	snake->positions[0] = init_p;
	snake->length = 1;
}

point_t * create_next_point(snake_t *snake){
	point_t *new_point = (point_t *)calloc(1,sizeof(point_t));
	new_point->x = snake->positions[0]->x + snake->vector->vx;
	new_point->y = snake->positions[0]->y + snake->vector->vy;
	return new_point;
}

void *draw(void *data){
	//cast the game status structure
	game_status_t *game_status = (game_status_t *)data;
	//cast the snake in the game
	snake_t *snake = game_status->snake;
	//cast the display of the game
	display_t *window = game_status->window;
	//cast the food piece in the game
	point_t *food_piece = game_status->food_piece;
	unsigned int k,n;
	char score_panel[5];
	//char positions[20];
	unsigned int resulting_score = 0;
	unsigned int snake_size;
	//loop whilst the in_game flag is set on
	while(game_status->in_game){
		clear();
		if(game_status->in_game == PLAYING){
				//draw box
				box(stdscr, 0, 0);
				//DRAW SCORE PANEL**********************************************************************
				mvprintw(1, 1, "Score: ");
				sprintf(score_panel, "%d", game_status->score);
				mvprintw(1, 10, score_panel);
				//DRAW SNAKE**************************************************************************
				snake_size = snake->length;
				
				point_t *new_point = create_next_point(snake);
				
				//free last position
				if(snake->positions[snake_size-1]){
					//write_log_panel("Have to delete this cell\n", window);
					free(snake->positions[snake_size-1]);
				}
				
				for(k=snake_size-1;k>0;k--){					
					//shift positions
					snake->positions[k] = snake->positions[k-1];
				}
				snake->positions[0] = new_point;
				for(n=0;n<snake_size;n++){
					//sprintf(positions, "%d,,%d", snake->positions[n]->y,snake->positions[n]->x);
					//write_log_panel(positions, window);
					mvaddch(snake->positions[n]->y, snake->positions[n]->x, SNAKE_BODY_PIECE);
				}
				
				//DRAW FOOD PIECE***********************************************************************
				mvaddch(food_piece->y, food_piece->x, FOOD_PIECE);
				if(hit_food(snake, food_piece) == DOES_COLLIDE){
					game_status->score+=SCORE_UNIT;
					snake->length++;
					add_food_piece(food_piece, window);
				}
				if(collision_with_walls(snake, window) == DOES_COLLIDE || hits_itself(snake) == DOES_COLLIDE){
					//get the score
					resulting_score = game_status->score;
					//set the in_game flag to go to the game over menu
					game_status->in_game = GAME_OVER_MENU;
					//RESET game
					init_snake(snake,window);
					game_status->score = 0;
					add_food_piece(food_piece, window);
				}
		}
		else if(game_status->in_game == GAME_OVER_MENU){
			//draw game over panel
			mvprintw(window->rows/2 -2, (window->columns - strlen(GAME_OVER))/2, GAME_OVER);
			mvprintw(window->rows/2, (window->columns-21)/2, "You scored: %d points!", resulting_score);
			mvprintw(window->rows/2 + 2, (window->columns - strlen(INSTRUCTIONS))/2, INSTRUCTIONS);
		}
		else if(game_status->in_game == PAUSED){
			mvprintw(window->rows/2-2, (window->columns - strlen(PAUSE_TEXT))/2, PAUSE_TEXT);
			refresh();
			usleep(50000);
			continue;
		}
		refresh();
		usleep(50000);
	}
	
	return NULL;
}
	
int main(int argc, char **argv){
	//set up random seed
	srand(time(NULL));
	
	//draw thread id
	pthread_t drawing_thread;	
	
	//create the snake
	snake_t snake;

	int command;
	
	//keep track of the window size
	display_t window;
	
	//create the food piece
	point_t food_piece;
	
	//init game status
	game_status_t game_status;
	game_status.snake = &snake;
	game_status.window = &window;
	game_status.food_piece = &food_piece;
	game_status.score = 0;
	game_status.in_game = PLAYING;
	
	//init ncurses mode
	initscr();
	//get keys
	keypad(stdscr, TRUE);
	//don't wait until EOL to get char
	noecho();
	//don't show cursor
	curs_set(0);
	//get window size and store values in the window struct
	getmaxyx(stdscr, window.rows, window.columns);
	
	//print game title
	print_title(&window);
	
	//set up the first food piece
	add_food_piece(&food_piece, &window);
	
	//init snake
	init_snake(&snake, &window);
	
	//start drawing thread
	pthread_create(&drawing_thread, NULL, draw, (void*)&game_status);
	
	while((command = getch())!=KEY_Q){
		switch(command){
			case KEY_LEFT:
				if(snake_moves_horizontal(&snake)) break;
				snake.vector->vx = -1;
				snake.vector->vy = 0;
				break;
			case KEY_RIGHT:
				if(snake_moves_horizontal(&snake)) break;
				snake.vector->vx = 1;
				snake.vector->vy = 0;
				break;
			case KEY_UP:
				if(snake_moves_vertical(&snake)) break;
				snake.vector->vx = 0;
				snake.vector->vy = -1;
				break;
			case KEY_DOWN:
				if(snake_moves_vertical(&snake)) break;
				snake.vector->vx = 0;
				snake.vector->vy = 1;
				break;
			case KEY_SPACE:
				game_status.in_game = PLAYING;
				break;
			case KEY_P:
				if(game_status.in_game == PAUSED)
					game_status.in_game = PLAYING;
				else
					game_status.in_game = PAUSED;
				break;
			
		}
	}
	game_status.in_game = OFF;
	pthread_join(drawing_thread, NULL);
	//end the window
	endwin();
	return 0;
}
