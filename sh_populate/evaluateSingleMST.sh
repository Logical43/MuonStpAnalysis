## script to evaluate one of the MSTS from a generation

#!/bin/sh

# reading in positional variables
export GEN=$1
export ID=$2
start_time=$(date +"%T")
start_date=$(date +"%F")
echo "-------------------------"
echo "Start date: $start_date"
echo "Start time: $start_time"
echo "evaluating MST generation: " $GEN " id: " $ID
echo "-------------------------"

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
end_time=$(date +"%T")
end_date=$(date +"%F")
echo "-------------------------"
echo "Start date: $start_date"
echo "End date: $end_date"
echo "Start time: $start_time"
echo "End time: $end_time"
echo "finished evaluating MST generation: " $GEN " id: " $ID
echo "-------------------------"
