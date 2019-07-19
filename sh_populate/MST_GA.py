# a test program to generate MST macros with GA
##################
from deap import base
from deap import creator
from deap import tools
from MST import *

import subprocess
import random
import numpy as np
import os
import shutil

# grid definition
nDisksOnAxis = 2
nDisks = int(pow(nDisksOnAxis,3))

# initialising for the fitness criteria and the MST individual
creator.create("FitnessMax", base.Fitness, weights=(1.0,1.0))
creator.create("thickness", np.ndarray, fitness=creator.FitnessMax)

# Attribute generator 
toolbox = base.Toolbox()
toolbox.register("random_disk",np.random.rand)	# randomising thickness of a single disk
toolbox.register("single_MST", tools.initRepeat,creator.thickness,toolbox.random_disk,nDisks)	# MST system
toolbox.register("MST_pop",tools.initRepeat,list,toolbox.single_MST)

# random fitness generator for sanity check 
def mock_eval(mst):
	return [np.random.normal(sum(mst)/mst.size,0),np.random.normal(sum(mst)/mst.size,0)]

# get fitness from file 
def eval(gen,id):
    os.system("./runSingleMST.sh "+str(int(gen))+" "+str(int(id)))
    os.system("./evaluateSingleMST.sh "+str(int(gen))+" "+str(int(id)))

    # initialise stopping rate and number of high energy e-
    sr = 0
    nHE = 0

    # read from file
    if os.path.exists('./stoppingRate.txt'):
        srLine = subprocess.check_output(['tail', '-1', "stoppingRate.txt"])
        sr = float(srLine.rstrip('\n'))
    if os.path.exists('./highEElectrons.txt'):
        nHELine = subprocess.check_output(['tail', '-1', "highEElectrons.txt"])
        nHE = float(nHELine.rstrip('\n'))

    return [sr,nHE]

# GA technique definitions
toolbox.register("evaluate",eval)	# fitness evaluation
toolbox.register("mate", tools.cxOnePoint) # cross function
toolbox.register("mutate", tools.mutShuffleIndexes, indpb=0.05)
toolbox.register("select", tools.selTournament, tournsize=3)

# main program for genetic algorithm
def main():
    # defining initial population
    pop = toolbox.MST_pop(n=nDisks)

    # clean up old files
    if os.path.exists('./stoppingRate.txt'):
        os.remove('stoppingRate.txt')
    if os.path.exists('./highEElectrons.txt'):
        os.remove('highEElectrons.txt')

    # clean up old result directories
    shutil.rmtree('runOutput',ignore_errors=True)
    print('Cleared run output')
    shutil.rmtree('evalOut',ignore_errors=True)
    print("Cleared evaluation output")

    # making new directories
    if not os.path.exists('./runOutput'):
        os.mkdir("./runOutput/")
    if not os.path.exists('./evalOut'):
        os.mkdir("./evalOut")

    print("-- Generation 1 --")
    # creating SIMG4 directory for the first iteration
    simG4Dir = '/vols/comet/users/sy5118/newBuild/OfflineProject/packages/SimG4/geometry_macros/GA_MST/generated_MST1' 
    if not os.path.exists(simG4Dir):
        os.makedirs(simG4Dir)

    j = 0
    for mst in pop:
       baby_MST = MST(nDisksOnAxis,mst,gen=1,id=j)
       baby_MST.write_MST(simG4Dir,j)
       j = j + 1

    fitnesses = list(map(toolbox.evaluate, np.ones(nDisks), range(1,nDisks+1)))
    storage = np.array([pop[1]])

    for ind, fit in zip(pop, fitnesses):
        ind.fitness.values = fit

    # CXPB  is the probability with which two individuals
    #       are crossed
    #
    # MUTPB is the probability for mutating an individual
    CXPB, MUTPB = 0.5, 0.2

	# Extracting all the fitnesses of 
    fits = [ind.fitness.values[0] for ind in pop]
    # counter
    g = 1
    # Begin the evolution
    while max(fits) < 1 and g < 5:
        g = g + 1
    	# A new generation
        simG4Dir = '/vols/comet/users/sy5118/newBuild/OfflineProject/packages/SimG4/geometry_macros/GA_MST/generated_MST'+str(g)
        if not os.path.exists(simG4Dir):
            os.makedirs(simG4Dir)

        print("-- Generation %i --" % g)

        # Select the next generation individuals
        offspring = toolbox.select(pop, len(pop))
        # Apply crossover and mutation on the offspring
        for child1, child2 in zip(offspring[::2], offspring[1::2]):
            if random.random() < CXPB:
                toolbox.mate(child1, child2)
                del child1.fitness.values
                del child2.fitness.values

        for mutant in offspring:
            if random.random() < MUTPB:
                toolbox.mutate(mutant)
                del mutant.fitness.values

        j = 1
        for pop in offspring:
        	baby_MST = MST(nDisksOnAxis,pop,gen=g,id=j)
        	baby_MST.write_MST(simG4Dir,j)
        	pop.fitness.values = toolbox.evaluate(g,j)
	        j = j + 1
        # Clone the selected individuals
        offspring = list(map(toolbox.clone, offspring))

                # Evaluate the individuals with an invalid fitness
        invalid_ind = [ind for ind in offspring if not ind.fitness.valid]
        fitnesses = map(toolbox.evaluate, invalid_ind)
        for ind, fit in zip(invalid_ind, fitnesses):
            ind.fitness.values = fit

        pop = offspring
        # Gather all the fitnesses in one list and print the stats
        fits = [ind.fitness.values[0] for ind in pop]


if __name__ == '__main__':
    main()
