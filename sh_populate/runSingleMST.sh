## script to run one of the MSTS from a generation

#!/bin/sh
# reading the positional variables
export GEN=$1
export ID=$2
start=$(date +"%T")

echo "starting MST generation: " $GEN " id: " $ID

# exporting the existing import file for muon events only
export INPUT_FILE=/vols/comet/users/sy5118/newBuild/OfflineProject/packages/MuonStpAnalysis/populate/strippedMuon.rootracker
export SH_NUM_EVENTS=100

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
end=$(date +"%T")
echo "-------------------------"
echo "Start time: $start"
echo "End time: $end"
echo "-------------------------"
