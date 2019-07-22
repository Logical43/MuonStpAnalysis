## script to run one of the MSTS from a generation

#!/bin/sh
# reading the positional variables
export GEN=$1
export ID=$2
start_time=$(date +"%T")
start_date=$(date +"%F")
echo "-------------------------"
echo "Start date: $start_date"
echo "Start time: $start_time"
echo "running MST generation: " $GEN " id: " $ID
echo "-------------------------"

# exporting the existing import file for muon events only
export INPUT_FILE=/vols/comet/users/sy5118/newBuild/OfflineProject/packages/MuonStpAnalysis/populate/strippedMuon.rootracker
export SH_NUM_EVENTS=200

# creating output directory wrt generation and individual id
export OUTPUT_DIR=$ICEDUSTROOT/OfflineProject/packages/MuonStpAnalysis/populate/runOutput/generation${GEN}/MST$ID
if [ ! -d $OUTPUT_DIR ]; then
   echo "no output directory so making one"
   mkdir -p $OUTPUT_DIR
fi

# creating and running individual geometries
for i in $SIMG4ROOT/geometry_macros/GA_MST/generated_MST$GEN/MSTS_obj_$ID.macro; do
	echo $i
	cd $SIMG4ROOT/geometry_macros

	# creating new softlink for the MuonStoppingTargetSystem
	rm -f $SIMG4ROOT/geometry_macros/MuonStoppingTargetSystem.macro
	ln -s $i MuonStoppingTargetSystem.macro

	# running icedust control
	RunIcedustControl -c $ICEDUSTROOT/OfflineProject/packages/MuonStpAnalysis/cfg/MST_muon_gun.cfg

done

# timestamping
end_time=$(date +"%T")
end_date=$(date +"%F")
echo "-------------------------"
echo "Start date: $start_date"
echo "End date: $end_date"
echo "Start time: $start_time"
echo "End time: $end_time"
echo "finished running MST generation: " $GEN " id: " $ID
echo "-------------------------"
