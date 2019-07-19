## script to evaluate one of the MSTS from a generation

#!/bin/sh

# reading in positional variables
export GEN=$1
export ID=$2
start=$(date +"%T")
echo "evaluating MST generation: " $GEN " id: " $ID

# creating output directory wrt generation and individual id
export OUTPUT_DIR=/vols/comet/users/sy5118/newBuild/OfflineProject/packages/MuonStpAnalysis/populate/evalOut/generation$GEN/MST$ID
if [ ! -d $OUTPUT_DIR ]; then
   echo "no output directory so making one"
   mkdir -p $OUTPUT_DIR
fi

# reading in input files
export INPUT_FILE=$ICEDUSTROOT/OfflineProject/packages/MuonStpAnalysis/populate/runOutput/generation$GEN/MST$ID/*root
RunIcedustControl -c $ICEDUSTROOT/OfflineProject/packages/MuonStpAnalysis/cfg/validate.cfg

# timestamping
end=$(date +"%T")
echo "-------------------------"
echo "Start time: $start"
echo "End time: $end"
echo "-------------------------"
