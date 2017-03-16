# Concept
Simulation of a Foxes &amp; Rabbits ecosystem 

The simulation is implemented in C and paralelized with OpenMP

The idea behind this project is to explore the capabilities of the OpenMP
API and to implement a solution with a significant performance increase
when compared to its the single-thread alternative.

The code focuses on simulating the generational evolution of an ecosystem
composed of Foxes, Rabbits and Rocks. The Rabbits navigate the ecosystem's
positions and procreate according to a given rule. The Foxes also navigate the
grid while trying to eat the rabbits. The foxes can also procreate according to
a rule and they can starve if they don't eat a rabbit for a given number of
iterations. Rocks are places simply to limit the possibilities of movement 
of the sepecies.

# Compiling and running

The repository contains a makefile for compilation

To run the compiled code, the user must pass as an argument to the program
the path to a file where the first line contains the following ints, seperated by spaces

GEN_PROC_RABBITS - number of generations until a rabbit can procreate

GEN_PROC_FOXES - number of generations until a fox can procreate

GEN_FOOD_FOXES - number of generations for a fox to die of starvation

N_GEN - number of generations for the simulation

R - number of rows of the matrix representing the ecosystem

C - number of columns of the matrix representing the ecosystem

N - number of objects in the initial ecosystem

and the next lines are of the type

SPECIES POS_X POS_Y

# Extra code

This repository also contains some simple python scripts for input creation and data analysis on
the python_scripts folder
