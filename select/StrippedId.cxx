#include "IG4Trajectory.hxx"
#include "IBeamlineCoordinates.hxx"
#include "IBeamlineParameterExtractor.hxx"
#include "IOADatabase.hxx"
#include "IDatum.hxx"
#include "IHandle.hxx"
#include "IVInputFile.hxx"
#include "IRooTrackerFile.hxx"

#include "ICOMETEvent.hxx"
#include "ICOMETEventLoopFunction.hxx"
#include "cometEventLoop.hxx"

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

// -------------- CLASS DEFINITION --------------
// ----------------------------------------------
class TMySimpleEventLoop: public COMET::ICOMETEventLoopFunction 
{
  public:
    TMySimpleEventLoop(): 
      fInput(NULL), fOutFile(NULL), 
      fOutFileName("strippedMuon.rootracker"), fNEventsRead(0),
      fRandomGen(NULL), fChangeToSignalElectrons(false), fCheckGeometry(false)
      {}
    virtual ~TMySimpleEventLoop() {} 

    void Initialize(void); 
    void BeginFile(COMET::IVInputFile *const);
    void Finalize(COMET::ICOMETOutput* output); 
    virtual bool SetOption(std::string option,std::string value="");
    bool operator () (COMET::ICOMETEvent& event);

  private:
    const COMET::IVInputFile* fInput;
    COMET::IHandle<COMET::IRooTrackerFile> fOutFile;
    std::string fOutFileName;
    int fNEventsRead;

    TRandom3* fRandomGen;
    bool fChangeToSignalElectrons;
    
    bool fCheckGeometry;
    COMET::IBeamlineCoordinates fBeamlineCoords;
};

// -------------- CLASS Functions ---------------
// ----------------------------------------------
void TMySimpleEventLoop::Initialize() {
  // Open the output file
  fOutFile=COMET::IRooTrackerFile::Open(fOutFileName,"RECREATE");

  if(!fOutFile->IsOpen()){
    COMETSevere("Unable to open output RooTracker file, '"<<fOutFileName<<"'");
    throw;
  }
  int dummy = fOutFile->AddMonitor("StoppingTarget");
  fRandomGen = new TRandom3(0); //equals fRandomGen->SetSeed(0), see documentation


  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Signal or DIO? 
  fChangeToSignalElectrons = true;
  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
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
  // ------------------------- Load Geometry and Initialise BeamLineCoordinates
  // ----------------------------------------------------------------------------------
  if(fCheckGeometry) { // Updated every time we open a new file
    TGeoManager* geometry=COMET::IOADatabase::Get().UpdateGeometry();
    if(!geometry){
      COMETError("No valid geometry found. Beamline coordinates won't work");
    }
    else {
      // Setup the beamline coordinate system from the geometry
      COMET::IBeamlineParameterExtractor::Load(fBeamlineCoords,geometry);
      fBeamlineCoords.Initialise();
    }
    fCheckGeometry = false;
  }
  

  // ----------------------------------------------------------------------------------
  // ------------------------- Trajectory Loop
  // ----------------------------------------------------------------------------------
  // Get the trajectory container
  COMET::IHandle<COMET::IG4TrajectoryContainer> trajectories = event.Get<COMET::IG4TrajectoryContainer>("~/truth/G4Trajectories");
  if(!trajectories) 
  {
    // Just a Warning
    // Note: Happens often when a lot particles are blacklisted.
    //COMETError("Error in SaveStoppedMuons: Couldn't find trajectory container...");
    return false;
  }
  
  // Loop over trajectories
  for(COMET::IG4TrajectoryContainer::const_iterator i_traj = trajectories->begin(); i_traj != trajectories->end(); ++i_traj){
    const COMET::IG4Trajectory& traj=i_traj->second;

    // If not a particle (muon) we want to track, continue
    int pdgCode = traj.GetPDGEncoding();
    if (!(pdgCode == 13)) { continue; }

    // Check if stopped in discs
    TVector3 endPos_cm = traj.GetFinalPosition().Vect()*(1/unit::cm); // req. in cm for gGeoManager!! 
    gGeoManager->PushPath();
    gGeoManager->GetTopNode()->cd();
    TGeoNode* volume=gGeoManager->FindNode(endPos_cm.X(),endPos_cm.Y(),endPos_cm.Z());
    gGeoManager->PopPath();
    std::string volumeName = volume->GetName();
    if(!(volume &&  TString(volumeName).Contains("TargetDisk") && !TString(volumeName).Contains("Prod"))) { continue; }



    // Still here, than stopped muon in disk -> Fill Data
    // -----------------------------------------------------
    TLorentzVector pos = traj.GetFinalPosition();
    TVector3 mom = traj.GetTrajectoryPoints().back().GetMomentum(); // should be (0,0,0) for stopped muons...
    if(!(mom.x() == 0 && mom.y() == 0 && mom.z() == 0)) { 
      COMETWarn("Initial muon-mom was not 0, really a stopped particle??");
    }

    // Change particle to electon?
    if(fChangeToSignalElectrons) { pdgCode = 11; }

    // Get the particle mass
    TParticlePDG* pdgPart = TDatabasePDG::Instance()->GetParticle(pdgCode);
    if (!pdgPart) {
      COMETSevere("Particle PDG '"<<pdgCode<<"' not recognized");
      throw;
    }
    double mass = pdgPart->Mass() * (unit::GeV); //fk always returns in GeV = 0.510999 MeV
    //std::cout << "mass: " << mass << std::endl;

    // Change momentum to signal-e?
    if(fChangeToSignalElectrons) { 
      // Get isotropic momentum 
      double cosTheta = -1.0 + 2.0*fRandomGen->Rndm();
      double phi = twopi * fRandomGen->Rndm();
      double sinTheta = std::sqrt(1 - cosTheta*cosTheta);
      TVector3 nomralisedMom(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
      // Set energy
      double signalElecEnergy = 105.0*(unit::MeV); // (muon-mass: 105.658MeV)
      double a = std::sqrt(signalElecEnergy*signalElecEnergy - mass*mass);
      // Overwrite momentum of primary particle
      mom = nomralisedMom * a;
    }
    double energy = std::sqrt(mom.Mag2() + mass*mass); 
    //std::cout << "Momentum: (" << mom.x() << "," << mom.y() << "," << mom.z() << ")" << std::endl;
    //std::cout << "New energy: " << energy/(unit::MeV) << std::endl;

    int particleNum=fOutFile->NewParticle();
    fOutFile->SetStdHepX4(      particleNum, pos.X(), pos.Y(), pos.Z(), pos.T());
    fOutFile->SetStdHepP4(      particleNum, mom.x(), mom.y(), mom.z(), energy);
    fOutFile->SetStdHepPdg(     particleNum, pdgCode);
    // Track ID not required... we do not want to match?
    //fOutFile->SetStdHepTrackId( particleNum, traj.GetTrackId());
    // Assume unity for the weight
    fOutFile->SetWeight(        particleNum, 1);
    // TODO have this support more than one monitor
    fOutFile->SetMonitorId(     particleNum, 1);
    // set to 1 since that is what SimG4 is expecting for particles that it can track
    fOutFile->SetStdHepStatus(  particleNum, 1);
    // Set the event number
    fOutFile->SetEvtNum(fNEventsRead);
    // Fill stuff
    fOutFile->Fill();
    fNEventsRead++;
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
