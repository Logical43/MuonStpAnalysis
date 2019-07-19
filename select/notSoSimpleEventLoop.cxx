#include "IG4Trajectory.hxx"
#include "IOADatabase.hxx"
#include "IDatum.hxx"
#include "IHandle.hxx"
#include "IVInputFile.hxx"
#include "IRooTrackerFile.hxx"

#include "ICOMETEvent.hxx"
#include "ICOMETEventLoopFunction.hxx"
#include "cometEventLoop.hxx"
#include <IG4Trajectory.hxx>
#include "IG4VHit.hxx"
#include "IG4HitSegment.hxx"

#include "TTree.h"
#include "TRandom3.h"
#include "TFile.h"
#include "TObject.h"
#include "TDirectory.h"
#include "TParticlePDG.h"
#include "TDatabasePDG.h"
#include "TGeoNode.h"
#include "TROOT.h"
#include "TGeoManager.h"
#include "HEPUnits.hxx"

#include <string> 
#include <vector>
#include <sstream> 
#include <iostream> 
#include <fstream>

class TMySimpleEventLoop: public COMET::ICOMETEventLoopFunction{
public:
    TMySimpleEventLoop():
        fInput(NULL), fOutFile(NULL),
        fOutFileName("strippedMuon.rootracker"),fNEventsRead(0)
        {}
        virtual ~TMySimpleEventLoop(){}

        void Initialize(void);
        void BeginFile(COMET::IVInputFile *const);
        void Finalize(COMET::ICOMETOutput* output);
        bool SetOption(std::string option,std::string value="");
        bool operator () (COMET::ICOMETEvent& event);

private:
    const COMET::IVInputFile* fInput;
    COMET::IHandle<COMET::IRooTrackerFile> fOutFile;
    std::string fOutFileName;
    int fNEventsRead;
};

// -------------- CLASS Functions ---------------
// ----------------------------------------------
void TMySimpleEventLoop::Initialize() {
    // Open the output file
    fOutFile=COMET::IRooTrackerFile::Open(fOutFileName,"UPDATE");

    if(!fOutFile->IsOpen()){
        COMETSevere("Unable to open output RooTracker file, '"<<fOutFileName<<"'");
        throw;
    }
    int dummy = fOutFile->AddMonitor("DetectorSolenoid");

}

void TMySimpleEventLoop::BeginFile(COMET::IVInputFile *const input) {
    fInput=input;
    //fCheckGeometry = true;
}

bool TMySimpleEventLoop::SetOption(std::string option,std::string value) {
    if(option=="rt_out") fOutFileName=value;
    else return false;
    return true;
}

void TMySimpleEventLoop::Finalize(COMET::ICOMETOutput* output) {
    //fOutFile->Write(); 
    std::cout << "ICOMETOutput: " << output << " | fOutFile: " << fOutFile <<std::endl;
    fOutFile->Close(); 
}

bool TMySimpleEventLoop::operator () (COMET::ICOMETEvent& event) {
    // Reset the output file
    fOutFile->Reset();

    // ----------------------------------------------------------------------------------
    // ------------------------- Trajectory Loop
    // ----------------------------------------------------------------------------------
    // Get the trajectory container
    COMET::IHandle<COMET::IG4TrajectoryContainer> trajectories = event.Get<COMET::IG4TrajectoryContainer>("~/truth/G4Trajectories");
    // Get the muon stopping target system hits
    COMET::IHandle<COMET::IG4HitContainer> mstHits = event.Get<COMET::IG4HitContainer>("truth/g4Hits/CDC");
    if(!trajectories) 
    {
    // Just a Warning
    // Note: Happens often when a lot particles are blacklisted.
    //COMETError("Error in SaveStoppedMuons: Couldn't find trajectory container...");
        return false;
    }
    
    if(mstHits){
        // Loop over trajectories
        for(COMET::IG4TrajectoryContainer::const_iterator i_traj = trajectories->begin(); i_traj != trajectories->end(); ++i_traj){
            const COMET::IG4Trajectory& traj=i_traj->second;

            // If not a particle (muon) we want to track, continue
            int pdgCode = traj.GetPDGEncoding();
            if (!(pdgCode == 13)) { continue; }


            // Save the initial position of the particle
            // -----------------------------------------------------
            TLorentzVector pos = traj.GetInitialPosition();
            TLorentzVector mom = traj.GetInitialMomentum(); // should be (0,0,0) for stopped muons...

            // Get the particle mass
            TParticlePDG* pdgPart = TDatabasePDG::Instance()->GetParticle(pdgCode);
            if (!pdgPart) {
              COMETSevere("Particle PDG '"<<pdgCode<<"' not recognized");
              throw;
            }
            double mass = pdgPart->Mass() * (unit::GeV); //fk always returns in GeV = 0.510999 MeV
            //std::cout << "mass: " << mass << std::endl;

            double energy = std::sqrt(mom.Mag2() + mass*mass); 

            int particleNum=fOutFile->NewParticle();
            fOutFile->SetStdHepX4(      particleNum, pos.X(), pos.Y(), pos.Z(), pos.T());
            fOutFile->SetStdHepP4(      particleNum, mom.X(), mom.Y(), mom.Z(), mom.T());
            fOutFile->SetStdHepPdg(     particleNum, pdgCode);
            // Track ID not required... we do not want to match?
            //fOutFile->SetStdHepTrackId( particleNum, traj.GetTrackId());
            // Assume unity for the weight
            fOutFile->SetWeight(        particleNum, 1);
            // TODO have thi[s support more than one monitor
            fOutFile->SetMonitorId(     particleNum, 1);
            // set to 1 since that is what SimG4 is expecting for particles that it can track
            fOutFile->SetStdHepStatus(  particleNum, 1);
            // Set the event number
            fOutFile->SetEvtNum(fNEventsRead);
            // Fill stuff
            fOutFile->Fill();
            fNEventsRead++;
            return true;
        }
    }

  // ----------------------
  //fOutFile->Fill();
  // Don't write oaEvents (always return false)
  return false;
}

// ----------------------------------------------
// ----------------------------------------------
// ----------------------------------------------
int main(int argc, char **argv) {
    TMySimpleEventLoop userCode;
    cometEventLoop(argc,argv,userCode);
}
