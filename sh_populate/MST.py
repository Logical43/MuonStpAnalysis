### MST Object class 
### defines uniformly placed target disks 
import numpy as np
import itertools
import os

# defined dimension of the stopping target region
dimension = 400
lamda = 1
# MST object class
class MST:

    # empty initialiser
    def __init__(self):
        self = []

    # initialiser with number of disks and thicknesses
    # number of disks (nDisksOnAxis) -> number of disks in x, y, z direction, could be uniform
    # thicknesses -> array of individual thicknesses 
    # radius -> array of disk radii. Default set to dim/nDisks
    def __init__(self, nDisksOnAxis, thicknesses, radius=None, gen=1,id=1):
        # generation and id of the genetic algorithm
        self.gen=gen
        self.id=id

        # if the radius isn't set, set it        
        if radius == None:
            radius = dimension/nDisksOnAxis

        self.radius = radius

        # if the disk distribution is uniform
        if np.array([nDisksOnAxis]).size == 1:
            # check for correct number of disks...
            if pow(nDisksOnAxis,3) != thicknesses.size:
                raise Exception('Number of target disks does not match granularity. Number of disk = '+str(thicknesses.size)+', divisions in space = '+str(pow(nDisksOnAxis,3)))
            else:
                self.nDisksOnAxis = nDisksOnAxis
                self.thicknesses = thicknesses

        # 3d nDisksOnAxis should be of the form of [x,y,z]
        else:
            # check for correct number of disks...
            if np.prod(nDisksOnAxis) != thicknesses.size:
                raise Exception('Number of target disks does not match granularity. Number of disk = '+str(thicknesses.size)+', divisions in space = '+str(np.prod(nDisksOnAxis)))
            else:
                self.nDisksOnAxis = nDisksOnAxis
                self.thicknesses = thicknesses

        # defining number of targets and their positions
        self.nTargets = pow(nDisksOnAxis,3) if np.array([nDisksOnAxis]).size == 1 else np.prod(nDisksOnAxis)
        self.def_position()

    # changing dimension
    def change_dimension(self, newDimension):
        self.dimension = newDimension

    # stopping rate only
    def change_fitness(self,fitness):
        self.fitness = fitness

    # stopping rate and electron energy loss
    def change_fitness(self,fitness1,fitness2):
        self.fitness = fitness1 - lamda*fitness2

    def change_weight(self,lamda):
        self.lamda = lamda

    # changing dimension
    def change_radius(self, newRadius):
        self.radius = newRadius

    # defining positions to individual target disks
    def def_position(self):
        nDisksOnAxis = self.nDisksOnAxis
        if 1 in np.array([nDisksOnAxis]):
        	raise Exception('This method requires at least 2 disks along all axis. Current number of disks on axis: '+str(nDisksOnAxis))

        separation = dimension/(nDisksOnAxis-1)

        # if uniform distribution
        if np.array([nDisksOnAxis]).size    == 1: 
        	# getting iterator based on the number of target disks
            iterable = np.arange(nDisksOnAxis)
            first = True

            # position defined as a grid centred at the stopping target region
            for i,j,k in itertools.product(iterable, iterable, iterable):
                position = np.array([[-dimension/2+i*separation,-dimension/2+j*separation,k*separation]])
                if first:
                    positions = position
                    first = False
                # concatenating positions
                else:
                    positions = np.concatenate((positions, position),axis=0)
            self.position = positions
            
        else:
        	# getting iterator based on the number of target disks
            iterable = dimension/nDisksOnAxis
            first = True

            # position defined as a grid centred at the stopping target region
            for i,j,k in itertools.product(np.arange(iterable[0]), np.arange(iterable[1]), np.arange(iterable[2])):
                position = np.array([[-dimension/2+i*separation[0],-dimension/2+j*separation[1],k*separation[2]]])
                if first:
                    positions = positions
                    first = False
                # concatenating positions
                else:
                    positions = np.concatenate((positions, position),axis=0)
            self.position = positions

    # write to macro file
    def write_MST(self,outdir,id):
    	# reference file. A similar file (with MuonStoppingTarget.macro) can be found on https://gitlab.in2p3.fr/comet/ICEDUST_packages/blob/master/SimG4/geometry_macros/MuonStoppingTargetSystem.macro
        with open('templateMSTS.macro') as MSTS:
        	# write to MSTS file
            with open(outdir+'/MSTS_obj_'+str(id)+'.macro','w') as newMSTS:
                counter = 0
                for line in itertools.islice(MSTS,0,18):
                    newMSTS.write(line)
                newMSTS.write("{parent}/Count     nTargets = "+str(self.nTargets)+"\n")
                for line in itertools.islice(MSTS,1,33):
                    newMSTS.write(line)
                if np.array([self.radius]).size==1:
                    newMSTS.write("{parent}/Dimension (i<[nTargets]) TargetDisk:OuterR   = "+str(self.radius)+"*mm\n")
                else: 
                    rCounter = 0
                    for i in self.radius:
                        newMSTS.write("{parent}/Dimension (i="+str(rCounter)+") TargetDisk:OuterR = "+str(i)+"*mm\n")
                for line in itertools.islice(MSTS,1,2):
                    newMSTS.write(line)
                
                newMSTS.write("# TargetDisk thickness \n")
                for i in range(0,self.thicknesses.size):
                    newMSTS.write("{parent}/Dimension (i="+str(i)+") TargetDisk:Length = "+str(self.thicknesses[i])+"*mm \n")
                for line in itertools.islice(MSTS,1,2):
                    newMSTS.write(line)
                newMSTS.write("# TargetDisk position \n")

                first = True
                tmpStore = np.array([])
                for i in self.position:
                    newMSTS.write("{parent}/Position (i="+str(counter)+") TargetDisk:Position = ("+str(i[0])+", "+str(i[1])+", "+str(i[2])+")\n")
                    counter = counter+1
                newMSTS.write("# TargetDisk rotation \n")
                newMSTS.write("{parent}/Rotation  (i<[nTargets]) TargetDisk:Rotation = (axis=(0,0,1),angle=0*degree) \n")
                for line in itertools.islice(MSTS,2,9):
                    newMSTS.write(line)

