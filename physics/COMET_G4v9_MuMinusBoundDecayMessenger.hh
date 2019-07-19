//-*-C++-*-
// $Id: COMET_G4v9_MuMinusBoundDecayMessenger.hh 150 2008-07-02 10:35:35Z comet $

#ifndef COMET_G4v9_MuMinusBoundDecayMessenger_HH
#define COMET_G4v9_MuMinusBoundDecayMessenger_HH 1
#include "globals.hh"
#include "G4UImessenger.hh"

class COMET_G4v9_MuMinusBoundDecay;
class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;

class COMET_G4v9_MuMinusBoundDecayMessenger: public G4UImessenger {
    public:
        COMET_G4v9_MuMinusBoundDecayMessenger(COMET_G4v9_MuMinusBoundDecay*);
        ~COMET_G4v9_MuMinusBoundDecayMessenger();

        void SetNewValue(G4UIcommand*, G4String);

    private:
        COMET_G4v9_MuMinusBoundDecay*   fMuMinusBoundDecay;

        G4UIcmdWithADoubleAndUnit*  fEMinDIOCmd;

};
#endif
