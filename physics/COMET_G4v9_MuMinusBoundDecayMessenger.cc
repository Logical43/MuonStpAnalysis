//-*-C++-*-
// $Id: COMET_G4v9_MuMinusBoundDecayMessenger.cc 150 2008-07-02 10:35:35Z comet $
#include "COMET_G4v9_MuMinusBoundDecayMessenger.hh"
#include "COMET_G4v9_MuMinusBoundDecay.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4TransportationManager.hh"
#include "G4PropagatorInField.hh"
#include "globals.hh"

COMET_G4v9_MuMinusBoundDecayMessenger::
COMET_G4v9_MuMinusBoundDecayMessenger(COMET_G4v9_MuMinusBoundDecay* anAction)
:fMuMinusBoundDecay(anAction) {
    //
    // Assuming COMET_G4v9_MuonMinusCaptureAtRestMessenger was alread called,
    // and /process/muonMinus directory exists.
    //

    fEMinDIOCmd =
        new G4UIcmdWithADoubleAndUnit("/process/muonMinus/minEnergyDIO",this);
    fEMinDIOCmd->
        SetGuidance("Set the minimum of the total energy of electrons produced via DIO process.");
    fEMinDIOCmd->SetParameterName("minEnergyDIO",false);
    fEMinDIOCmd->SetRange("minEnergyDIO>=0.");
    fEMinDIOCmd->SetUnitCategory("Energy");
    fEMinDIOCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

}


COMET_G4v9_MuMinusBoundDecayMessenger::~COMET_G4v9_MuMinusBoundDecayMessenger() {
    delete fEMinDIOCmd;
}


void COMET_G4v9_MuMinusBoundDecayMessenger::SetNewValue(G4UIcommand* command,G4String newValue) {

    if( command == fEMinDIOCmd ) {
        fMuMinusBoundDecay->SetEMinDIO(fEMinDIOCmd->GetNewDoubleValue(newValue));
    }

}
