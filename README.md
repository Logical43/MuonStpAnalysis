# MuonStpAnalysis
Separate package to implement genetic algorithm into ICEDDUST

This is a new package in ICEDUST which allows for the optimisation of MuonStoppingTarget (MST) geometry.

Individual description: 

---- physics/ ---- 
This subfolder modifies physics (such as mu - e conversion rate), and the analysis method/output of the AnalysisBase class. 

---- select/ ----
This is mostly an oaEventLoop which outputs rootracker files. The rootracker files only contain muon that will enter CyDet. 

---- sh_populate ---- 
This is a folder of joy and reproduction. Includes shell scripts to run the individual geometries, evaluate them, and use 
genetic algorithm to optimise them. 
