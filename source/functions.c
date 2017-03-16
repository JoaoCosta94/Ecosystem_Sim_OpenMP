#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "../include/functions.h"

/// Basic types for easier comparisons
const char types_in_char[4] = {'*', 'F', 'R', ' '};
const char types_in_string[4][8] = {"ROCK", "FOX", "RABBIT", "EMPTY"};

/// lock_matrix is the grid to spin-lock each ecosystem position
omp_lock_t *lock_matrix;

/*
Function that creates a spin-lock for
each position of the ecosystem grid

@param int		size		Dimension of the square grid
*/
void instanciate_locks(int size) {
	lock_matrix = (omp_lock_t*)malloc(size * sizeof(omp_lock_t));
	for (int i = 0; i < size; i++) {
		omp_init_lock(&lock_matrix[i]);
	}
}

/*
Function that creates frees the
spin-lock grid

@param int	size	Number of elements in the grid
*/
void destroy_locks(int size) {
	for (int i = 0; i < size; i++) {
		omp_destroy_lock(&lock_matrix[i]);
	}
	free(lock_matrix);
}

/*
Function that reads the iformation and rules 
for the ecosystem evolution from a file

@param FILE*	file	Input file
*/
ECO_SETTINGS read_settings(FILE *file){
	ECO_SETTINGS settings;

	fscanf(file, "%d %d %d %d %d %d %d",  &settings.GEN_PROC_RABBITS,
										  &settings.GEN_PROC_FOXES,
										  &settings.GEN_FOOD_FOXES,
										  &settings.N_GEN,
										  &settings.R,
										  &settings.C,
										  &settings.N);

	settings.size = settings.R * settings.C;
	return settings;
}

/*
Function that reads the input file 
and places the ecosystem elements in their
respective positions.

@param FILE*	file	Input file
@param int		R		Number of Columns
@param int		C		Number of Rows
@param int		N		Number of non-empty positions in the initial ecosystem
*/
ECO_ELEMENT* read_gen0(FILE *file, int R, int C, int N){

  /// The ecosystem is represented with an inline matrix of ECO_ELEMENTs
  ECO_ELEMENT* eco_system = malloc(R*C*sizeof(ECO_ELEMENT)); 
  int idx;
  /// Temp container for each new element
  ECO_ELEMENT new_element;

  /// Create inline matrix for the ecosystem. Begin with everything set as empty
  for(int I = 0; I < R; I++){
    for(int J = 0; J < C; J++){
      idx = I*C + J;
      new_element.type = EMPTY;
	  new_element.temp_type = EMPTY;
	  new_element.gen_food = 0;
	  new_element.gen_proc = 0;
      eco_system[idx] = new_element;
    }
  }

  /// Read the file and place the non-empty elements
  char string[10];
  int X, Y;
  for(int I = 0; I < N; I++){
    fscanf(file, "%s %d %d", string, &X, &Y);
    idx = X*C + Y;

	/// Set the food and procreation to 0
	new_element.gen_food = 0;
	new_element.gen_proc = 0;
    if(strcmp(string,"ROCK") == 0){
      new_element.type = ROCK;
	  new_element.temp_type = ROCK;
    }
    else if(strcmp(string,"FOX") == 0){
      new_element.type = FOX;
	  new_element.temp_type = FOX;
    }
    else if(strcmp(string,"RABBIT") == 0){
      new_element.type = RABBIT;
	  new_element.temp_type = RABBIT;
    }
    else{
      new_element.type = EMPTY;
	  new_element.temp_type = EMPTY;
    }
    eco_system[idx] = new_element;
  }

  return eco_system;
}

/*
Auxiliary function to print the 
matrix of a given generation

@param ECO_ELEMENT*		eco_system	State of the ecosystem
@param int				R			Number of Columns
@param int				C			Number of Rows
@param int				flag		Flag to specify the print type of empty spaces
*/
void print_gen(ECO_ELEMENT *eco_system, int R, int C, int gen, int flag){
  char *bar = malloc((R+1)*sizeof(char));
  strcpy(bar, "-");
  for (int I = 0; I <= R; I++){
    strcat(bar, "-");
  }

  printf("Generation %d\n", gen);
  printf("%s\n", bar);
  int idx;
  for(int I = 0; I < R; I++){
    printf("|");
    for(int J = 0; J < C-1; J++){
		idx = I*C + J;
		if (flag == 1)
			if (eco_system[idx].type == EMPTY) {
				printf("    ");
			}
			else if (eco_system[idx].type == ROCK) {
				printf("*** ");
			}
			else {
				printf("%c%d%d ", types_in_char[eco_system[idx].type], eco_system[idx].gen_proc, eco_system[idx].gen_food);
			}
		else
			printf("%c", types_in_char[eco_system[idx].type]);
    }
	if (flag == 1) {
		if (eco_system[idx + 1].type == EMPTY) {
			printf("   |\n");
		}
		else if (eco_system[idx + 1].type == ROCK) {
			printf("***|\n");
		}
		else {
			printf("%c%d%d|\n", types_in_char[eco_system[idx + 1].type], eco_system[idx + 1].gen_proc, eco_system[idx + 1].gen_food);
		}
	}
	else
		printf("%c|\n", types_in_char[eco_system[idx + 1].type]);
  }
  printf("%s\n\n", bar);
}

/*
Function to the save the sate of 
the ecosystem to a file

@param ECO_SETTINGS		settings	Settings and rules of the ecosystem
@param ECO_ELEMENT*		eco			State of the ecosystem
*/
void save_result(ECO_SETTINGS settings, ECO_ELEMENT* eco) {
	FILE *f = fopen("output.txt", "w+");

	int i, j, current_idx;
	int N = 0;
	for (i = 0; i < settings.size; i++) {
		if (eco[i].type != EMPTY) {
			N++;
		}
	}

	fprintf(f, "%d %d %d %d %d %d %d\n", settings.GEN_PROC_RABBITS, 
									settings.GEN_PROC_FOXES, 
									settings.GEN_FOOD_FOXES,
									0,
									settings.R,
									settings.C,
									N);

	for (i = 0; i < settings.R; i++) {
		for (j = 0; j < settings.C; j++) {
			current_idx = i*settings.C + j;
			if (eco[current_idx].type != EMPTY) {
				fprintf(f, "%s %d %d\n", types_in_string[eco[current_idx].type], i, j);
			}
		}
	}

	fclose(f);
}

/*
Function that calculates the new position
of an element of the ecosystem.Takes into 
consideration the type of the element,
the limits of the grid
and also the number
of the current generation

@param int				gen			Current generation's number
@param ECO_ELEMENT*		eco			State of the ecosystem
@param int				i			X position of the element
@param int				j			Y position of the element
@param int				R			Number of rows
@param int				C			Number of columns
@param int				type		Type of the element to move (as defined above)
*/
POSITION new_position(int gen, ECO_ELEMENT *ecosystem, int i, int j, int R, int C, int type) {
	int direction[4] = { 0, 0, 0, 0 }; // north, east, south, west
	ECO_ELEMENT elem;
	int idx;
	int size = R*C;

	
	if (i - 1 > -1 && i - 1 < R && j > -1 && j < C) {
		idx = (i - 1)*C + j;
		elem = ecosystem[idx];
		if (elem.type == type)
			direction[0]++;
	}
	if (i > -1 && i < R && j + 1 > -1 && j + 1 < C) {
		idx = i*C + j + 1;
		elem = ecosystem[idx];
		if (elem.type == type)
			direction[1]++;
	}
	if (i + 1 > -1 && i + 1 < R && j > -1 && j < C) {
		idx = (i + 1)*C + j;
		elem = ecosystem[idx];
		if (elem.type == type)
			direction[2]++;
	}
	if (i > -1 && i < R && j - 1 > -1 && j - 1 < C) {
		idx = i*C + j - 1;
		elem = ecosystem[idx];
		if (elem.type == type)
			direction[3]++;
	}
	POSITION pos;
	int pick = direction[0] + direction[1] + direction[2] + direction[3];
	if (pick == 0) {
		pos.x = i;
		pos.y = j;
		return pos;
	}
	pick = (gen + i + j) % pick;
	int dir = 0;
	while (dir < 5) {
		if (direction[dir] == 1) {
			if (pick == 0) {
				break;
			}
			else {
				pick--;
				dir++;
			}
		}
		else {
			dir++;
		}
	}
	switch (dir) {
		case 0:
			pos.x = i - 1;
			pos.y = j;
			return pos;
		case 1:
			pos.x = i;
			pos.y = j + 1;
			return pos;
		case 2:
			pos.x = i + 1;
			pos.y = j;
			return pos;
		case 3:
			pos.x = i;
			pos.y = j - 1;
			return pos;
		case 4:
			pos.x = i;
			pos.y = j;
			return pos;
		default:
			pos.x = 0;
			pos.y = 0;
			return pos;
	}
}

/*
Function that clears the animal elements of an ecosystem

@param ECO_ELEMENT*		new_eco		Ecosystem to clear
@param int				size		Number of possible positions on the grid
*/
void clear_fauna(ECO_ELEMENT *new_eco, int size) {
	/// Run in parallel with no restictions
	#pragma omp for schedule(static)
	for (int i = 0; i < size; i++) {
		if (new_eco[i].type != ROCK) {
			new_eco[i].type = EMPTY;
			new_eco[i].temp_type = EMPTY;
			new_eco[i].gen_proc = 0;
			new_eco[i].gen_food = 0;
		}
	}
}

/*
Function that copies the specified type of elements
from one ecosystem to another

@param ECO_ELEMENT*		current_eco		Source ecosystem
@param ECO_ELEMENT*		new_eco			Target ecosystem
@param int				size			Number of possible positions on the grid
@param int				type			Type of elements to be transmited (as defined above)
*/
void transmit_type(ECO_ELEMENT* current_eco, ECO_ELEMENT* new_eco, int size, int type) {
	/// Run in parallel with no restictions
	#pragma omp for schedule(static)
	for (int i = 0; i < size; i++) {
		if (current_eco[i].type == type) {
			new_eco[i] = current_eco[i];
		}
	}
}

/*
Function that evolves the postion of the rabbits
in the ecosystem

@param ECO_ELEMENT*		current_eco				Source ecosystem
@param ECO_ELEMENT*		new_eco					Target ecosystem
@param int				R						Number of possible positions on the grid
@param int				C						Type of elements to be transmited (as defined above)
@param int				GEN_PROC_RABBITS		Type of elements to be transmited (as defined above)
*/
void rabbit_pusher(int gen, ECO_ELEMENT* current_eco, ECO_ELEMENT* new_eco, int R, int C, int GEN_PROC_RABBITS) {
	int i, j;
	int current_idx, new_idx;

	/// Parallel iteration
	#pragma omp for private(j) schedule(guided)
	for (i = 0; i < R; i++) {
		for (j = 0; j < C; j++) {
			current_idx = i*C + j;
			if (current_eco[current_idx].type == RABBIT) {

				/// Calculate new possible position
				POSITION pos = new_position(gen, current_eco, i, j, R, C, EMPTY);
				new_idx = pos.x*C + pos.y;

				if (new_idx != current_idx) {

					/// Spin-locks avoid multiple threads trying to update 
					/// the same position

					if (current_eco[current_idx].gen_proc >= GEN_PROC_RABBITS) {
						/// Handles reproduction of the Rabbits
						omp_set_lock(&lock_matrix[current_idx]);

						new_eco[current_idx].type = RABBIT;
						new_eco[current_idx].gen_proc = -1;

						omp_unset_lock(&lock_matrix[current_idx]);
						current_eco[current_idx].gen_proc = -1;
					}

					omp_set_lock(&lock_matrix[new_idx]);
					
					if (new_eco[new_idx].type == EMPTY || (new_eco[new_idx].type == RABBIT && current_eco[current_idx].gen_proc > new_eco[new_idx].gen_proc)) {
						/// Solve conflicting Rabbits
						new_eco[new_idx] = current_eco[current_idx];
					}

					omp_unset_lock(&lock_matrix[new_idx]);
				}
				else {
					/// Rabbit stays in the same place
					omp_set_lock(&lock_matrix[new_idx]);

					new_eco[new_idx] = current_eco[current_idx];

					omp_unset_lock(&lock_matrix[new_idx]);
				}
			}
		}
	}

	/// Update the proc of all rabbits, in parallel
	#pragma omp for schedule(static)
	for (i = 0; i < R*C; i++) {
		if (new_eco[i].type == RABBIT) {
			new_eco[i].gen_proc++;
			new_eco[i].temp_type = RABBIT;
		}
	}
}

/*
Function that evolves the postion of the foxes
in the ecosystem

@param ECO_ELEMENT*		current_eco				Source ecosystem
@param ECO_ELEMENT*		new_eco					Target ecosystem
@param int				R						Number of possible positions on the grid
@param int				C						Type of elements to be transmited (as defined above)
@param int				GEN_PROC_RABBITS		Type of elements to be transmited (as defined above)
*/
void fox_pusher(int gen, ECO_ELEMENT* current_eco, ECO_ELEMENT* new_eco, int R, int C, int GEN_PROC_FOXES, int GEN_FOOD_FOXES) {
	int i, j;
	int current_idx, new_idx;

	/// Parallel iteration
	#pragma omp for private(j) schedule(guided)
	for (i = 0; i < R; i++) {
		for (j = 0; j < C; j++) {
			current_idx = i*C + j;
			if (current_eco[current_idx].type == FOX) {

				/// Selects te next position based on both rabbits and empy spaces
				POSITION pos = new_position(gen, current_eco, i, j, R, C, RABBIT);
				if (pos.x == i && pos.y == j) {
					pos = new_position(gen, current_eco, i, j, R, C, EMPTY);
				}
				new_idx = pos.x*C + pos.y;

				/// Spin-locks avoid multiple threads trying to update 
				/// the same position

				if (current_idx != new_idx) {
					if (current_eco[current_idx].gen_proc >= GEN_PROC_FOXES){ 
						/// Handles reproduction of the Foxes if they don't starve or move into a Rabbit 
						if (current_eco[current_idx].gen_food + 1 < GEN_FOOD_FOXES) {
							omp_set_lock(&lock_matrix[current_idx]); /// LOCK current_idx
							new_eco[current_idx].type = FOX;
							new_eco[current_idx].gen_proc = -1;
							new_eco[current_idx].gen_food = -1;
							current_eco[current_idx].gen_proc = -1;
							omp_unset_lock(&lock_matrix[current_idx]); /// UNLOCK current_idx
						}
						else {
							omp_set_lock(&lock_matrix[new_idx]); /// LOCK new_idx
							if (new_eco[new_idx].temp_type == RABBIT) {
								omp_set_lock(&lock_matrix[current_idx]); /// LOCK current_idx
								new_eco[current_idx].type = FOX;
								new_eco[current_idx].gen_proc = -1;
								new_eco[current_idx].gen_food = -1;
								current_eco[current_idx].gen_proc = -1;
								omp_unset_lock(&lock_matrix[current_idx]); /// UNLOCK current_idx
							}
							omp_unset_lock(&lock_matrix[new_idx]); /// UNLOCK new_idx
						}
					}

					omp_set_lock(&lock_matrix[new_idx]); /// LOCK new_idx
					if (new_eco[new_idx].type == RABBIT) {
						/// Eat the Rabbit
						new_eco[new_idx] = current_eco[current_idx];
						new_eco[new_idx].gen_food = -1;
						new_eco[new_idx].temp_type = RABBIT;
					}
					else if (new_eco[new_idx].type == EMPTY && current_eco[current_idx].gen_food + 1 < GEN_FOOD_FOXES) {
						/// Fox moves to an empty place if it doesn't starve
						new_eco[new_idx] = current_eco[current_idx];
					}
					else if (new_eco[new_idx].type == FOX && current_eco[current_idx].gen_food + 1 < GEN_FOOD_FOXES
						&& (current_eco[current_idx].gen_proc > new_eco[new_idx].gen_proc 
							|| (current_eco[current_idx].gen_proc == new_eco[new_idx].gen_proc 
								&& current_eco[current_idx].gen_food < new_eco[new_idx].gen_food))) {
						/// Solve conflicting Foxes
						new_eco[new_idx] = current_eco[current_idx];
					}
					omp_unset_lock(&lock_matrix[new_idx]); /// UNLOCK new_idx
				}
				else if (current_eco[current_idx].gen_food + 1 < GEN_FOOD_FOXES) {
					/// Fox stays in the same place if it doesn't starve
					omp_set_lock(&lock_matrix[current_idx]); /// LOCK current_idx
					new_eco[current_idx] = current_eco[current_idx];
					omp_unset_lock(&lock_matrix[current_idx]); /// UNLOCK current_idx
				}
			}
		}
	}

	/// Update the food and proc of all foxes, in parallel
	#pragma omp for schedule(static)
	for (i = 0; i < R*C; i++) {
		if (new_eco[i].type == FOX) {
			new_eco[i].gen_proc++;
			new_eco[i].gen_food++;
			new_eco[i].temp_type = FOX;
		}
	}
}