#include <cometEventLoop.hxx>
#include <IG4Trajectory.hxx>
#include "IG4VHit.hxx"
#include "IG4HitSegment.hxx"

class TMySimpleEventLoop: public COMET::ICOMETEventLoopFunction {
    public:
        TMySimpleEventLoop() {}
        virtual ~TMySimpleEventLoop() {}

        void TMySimpleEventLoop::Initialize(){
            fOutFile=COMET::IRooTrackerFile::Open(fOutFileName,"RECREATE");
            if(!fOutFile->IsOpen()){
                COMETSevere("Unable to open output RooTracker file, '"<<fOutFileName<<"'");
                throw;
            }
        }

        bool operator () (COMET::ICOMETEvent& event) {
            fOutFile->Reset();
            COMET::IHandle<COMET::IG4HitContainer> cdcHits
                = event.Get<COMET::IG4HitContainer>("truth/g4Hits/CDC");
            COMET::IHandle<COMET::IG4TrajectoryContainer> trajectories
                = event.Get<COMET::IG4TrajectoryContainer>("truth/G4Trajectories");
            if(cdcHits) {
                for(COMET::IG4HitContainer::const_iterator seg = cdcHits->begin(); seg != cdcHits->end(); seg++) {
                    COMET::IG4HitSegment * seg1 = dynamic_cast<COMET::IG4HitSegment *>(*seg);
                    COMET::IHandle<COMET::IG4Trajectory> trajectory = trajectories->GetTrajectory(seg1->GetPrimaryId() );
                  
                    const int   cdc_parentId = trajectory->GetParentId();
                    const int   cdc_eventId   = event.GetEventId();
                    const int   cdc_truthPid    = trajectory->GetPDGEncoding();

                    if(cdc_truthPid==13) {
                        fOutFile->Fill();
                        return true;
                    }
                    return false;
                }
            }
        }
};

int main(int argc, char **argv) {
    TMySimpleEventLoop userCode;
    cometEventLoop(argc,argv,userCode);
}
