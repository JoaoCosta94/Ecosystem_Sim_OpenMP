import os
import math
import make_environment

if __name__ == '__main__':

    os.system("make")

    GEN_PROC_RABBITS = 3
    GEN_PROC_FOXES = 20
    GEN_FOOD_FOXES = 10
    N_GEN = 500
    ROCK = 1
    RABBIT = 1
    FOX = 1

    N_THREADS = [1, 2, 4, 8]
    BASE_N = 200

    DIMS_LIST= [int(BASE_N*math.sqrt(N)) for N in N_THREADS]

    INPUT_FILES = []
    for dim in DIMS_LIST:
        ROCK = 200*dim**2 // (BASE_N**2)
        RABBIT = 600*dim**2 // (BASE_N**2)
        FOX = 600*dim**2 // (BASE_N**2)
        INPUT_FILES.append(make_environment.write_environment(GEN_PROC_RABBITS,
                                                              GEN_PROC_FOXES,
                                                              GEN_FOOD_FOXES,
                                                              N_GEN,
                                                              dim,
                                                              dim,
                                                              ROCK,
                                                              RABBIT,
                                                              FOX))

    for file_name in INPUT_FILES:
        output_file = file_name + "_output"
        f = open(output_file, "w+")
        f.close()
        for N in N_THREADS:
            os.system("ecosystem.exe %s %d >> %s" % (file_name, N, output_file))
