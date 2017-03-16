#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/functions.h"


int main(int argc, char *argv[]){

	/// Check if an imput file was given
	if (argc < 2) {
		printf("No input file supplied\n");
		return 1;
	}

	/// Check if the number of threads was given as input
	/// If no input is given, code uses maximum number 
	/// of available threads
	int NUM_THREADS;
	if (argc == 3) {
		NUM_THREADS = atoi(argv[2]);
	}
	else {
		NUM_THREADS = omp_get_max_threads();
	}

	/// Verify that the number of threads is valid for the system
	if (NUM_THREADS > omp_get_max_threads()) {
		printf("Can't launch %d threads, using system limit: %d\n", NUM_THREADS, omp_get_max_threads());
		NUM_THREADS = omp_get_max_threads();
	}
	omp_set_num_threads(NUM_THREADS);

	/// Read the input file
	FILE *file;
	file = fopen(argv[1], "r");
	/// Obtain the rules and information about the given ecosystem
	ECO_SETTINGS settings = read_settings(file); 
	/// First generation is allocated on array_1
	ECO_ELEMENT *array_1 = read_gen0(file, settings.R, settings.C, settings.N);
	fclose(file);

	/// array_2 contains a copy of the first array
	ECO_ELEMENT *array_2 = malloc(settings.size*sizeof(ECO_ELEMENT));
	memcpy(array_2, array_1, settings.size*sizeof(ECO_ELEMENT));
	
	/// Create a grid of spin locks for the ecosystem positions
	/// Grid of locks helps because the threads are only locked if they 
	/// try to access the same position as another thread, instead of locking
	/// at every movement.
	instanciate_locks(settings.size);

	/// Start simulating 
	int gen;					/// Current gen counter
	double start, stop;			/// Variables for time storage
	start = omp_get_wtime();	/// Obtain computation init time

	/// Parallel region begins here
	/// Threads are launched just once to save time and keep cache 
	/// as structured as possible
	#pragma omp parallel private(gen) shared(settings, array_1, array_2)
	{
		/// Iterate over the given number of generations to simulate
		for (gen = 0; gen < settings.N_GEN; gen++) {

			/// Force explicit barrier for thread sync
			#pragma omp barrier

			/*
			The implemented algorithm runs as follows
			First the auxiliary array, array_2 is cleansed and left only with rabbits and foxes
			Then the rabbits in array_1 are moved according to the rules and placed on array_2
			The foxes are then moved to array_2 but still on their position form the previous gen
			The animals are then cleansed from array_1 for the update of the foxes
			The rabbits are placed on their new positions on array_1
			The foxes on array_2 move to their new position using array_2 and placed on their final position on array_1
			*/

			clear_fauna(array_2, settings.size);
			rabbit_pusher(gen, array_1, array_2, settings.R, settings.C, settings.GEN_PROC_RABBITS);
			transmit_type(array_1, array_2, settings.size, FOX);

			clear_fauna(array_1, settings.size);
			transmit_type(array_2, array_1, settings.size, RABBIT);
			fox_pusher(gen, array_2, array_1, settings.R, settings.C, settings.GEN_PROC_FOXES, settings.GEN_FOOD_FOXES);
		}
	}
	/// Obtain time after compuation
	stop = omp_get_wtime();

	/// Print computation time
	printf("%d\t%f\n", NUM_THREADS, stop - start);

	/// Save output to file
	save_result(settings, array_1);

	// Freeeeeeeeeeedom (Mel Gibson)
	destroy_locks(settings.size);
	free(array_1);
	free(array_2);
	return 0;
}
