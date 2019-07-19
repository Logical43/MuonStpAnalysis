// version 2018 of IMuonTargetTrajectoryAnalyzer, no tagger included
#ifndef ANALYSIS_IMUONTARGETTRAJECTORYANALYZER_HXX
#define ANALYSIS_IMUONTARGETTRAJECTORYANALYZER_HXX

#include "IRootHistogram.hxx"
#include "IMultiHist.hxx"
#include <TObject.h>
#include <IHandle.hxx>
#include <IDatum.hxx>
#include <IVAnalyzer.hxx>
#include <IG4Trajectory.hxx>

class TGeoNode;
namespace COMET{
 class IMuonTargetTrajectoryAnalyzer;
 class IPIDMultiHist;
}

class COMET::IMuonTargetTrajectoryAnalyzer: public COMET::IVAnalyzer{
    public:
        IMuonTargetTrajectoryAnalyzer():COMET::IVAnalyzer("IMuonTargetTrajectoryAnalyzer"){}
        virtual ~IMuonTargetTrajectoryAnalyzer(){}

        virtual bool EventStart();
        virtual bool Analyze(const IHandle<IDatum> data);

        static std::string GetTargetDatumClass(){return "COMET::IG4TrajectoryContainer"; }

    private:
        virtual bool SetupHistograms();
        bool FinalizeHistograms();
        bool AnalyzeTrajectory(const IG4Trajectory& trajectory,
        const IHandle<IG4TrajectoryContainer>& trajectories);
        int GetMSTDiskID(const std::string& nodePath);
        int GetMSTDiskID(const TVector3& pos3);
        bool IsInStoppingTargetSupport(const char* nodePath);
        bool IsInStoppingTargetSupport(const TVector3& pos);
        bool IsInTargetOrSupport(const TVector3& pos);

        IMultiHist* hMultiplicity;
        IMultiHist* hNoTrajectoryPoints;
        IMultiHist* hMomentum;
        IMultiHist* hMomentumMuonProducts;
        IMultiHist* hKinetic;
        IMultiHist* hKineticMuonProducts;
        IPIDMultiHist* hRateMuonProducts;
        IPIDMultiHist* hRateMuonProductsDynamicCombined;
        IPIDMultiHist* hRateMuonProductsMomCut;
        IMultiHist* hGlobalTimeMuonProducts;
        IMultiHist* hLifeTimeMuonProducts;

        COMET::IVHistogram* hParentDaughterTally;
        COMET::IVHistogram* hPosition;
        COMET::IVHistogram* hPositionMuonProducts;

        // Information about the stopping in each disk and the flux through the 
        // disks
        COMET::IH1F* hPassMuonDisk;
        COMET::IH1F* hStopMuonDisk;
        COMET::IH1F* hPassAllDisk;

        struct OverallData{
            std::map<int,int> fMultiplicities;
            double nEvents;
            long int nMuons;
            long int nStoppedPions;
            long int nMuonSecondaries;
            long int nMuonSecondariesInTgt;
            long int nNotStoppedMuons;
            long int nNotStoppedAll;
            void Reset(){
                fMultiplicities.clear();
                ++nEvents;
            }
            OverallData():nEvents(0),
                          nMuons(0),
                          nStoppedPions(0),
                          nMuonSecondaries(0),
                          nMuonSecondariesInTgt(0),
                          nNotStoppedMuons(0),
                          nNotStoppedAll(0) {}
        } fAllTrajData;

};
#endif // ANALYSIS_IMUONTARGETTRAJECTORYANALYZER_HXX
