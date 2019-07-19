#! /bin/sh
export INPUT_FILE=/vols/comet/users/sy5118/newBuild/OfflineProject/packages/IcedustValidate/validate.d/input/oa_g4_proton_01599939-0000_2d6gjxwtton3_SG4BH_000.rootracker

export SH_RUN_NUMBER=1
export SH_SUBRUN=0
export SH_VERSION_NUMBER=0
export SH_RANDOM_SEED=`expr $SH_RUN_NUMBER + $SH_VERSION_NUMBER`
export SH_NUM_EVENTS=50

RunIcedustControl -c $ICEDUSTVALIDATEROOT/config/rootracker_selection.cfg
