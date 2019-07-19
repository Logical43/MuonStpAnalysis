// version 2018 of IMuonTargetTrajectoryAnalyzer, no tagger included
#include "IMuonTargetTrajectoryAnalyzer.hxx"
#include "HEPUnits.hxx"
#include "IRootHistogram.hxx"
#include "IPIDMultiHist.hxx"
#include <IHandle.hxx>
#include <ICOMETEvent.hxx>
#include <IEventFolder.hxx>
#include <IG4VHit.hxx>
#include <IG4HitSegment.hxx>
#include <IG4Trajectory.hxx>
#include <IEventFolder.hxx>
#include "I2DParticleTally.hxx"
#include "IVolumePlot.hxx"
#include "IAnalyzerFactory.hxx"
#include "IAnalysisUtilities.hxx"
#include <IOADatabase.hxx>
#include <IOARuntimeParameters.hxx>
#include <TGeoNavigator.h>
#include <TGeoManager.h>
#include <TGeoNode.h>
#include <TString.h>

// testing if it can write muon stopping rate to a file
#include <iostream>
#include <fstream>
using namespace std;

using COMET::IAnalysisUtilities::GetNode;
using COMET::IAnalysisUtilities::FullNodePath;
using COMET::IAnalysisUtilities::ParticleStoppedInStopTgt;
using COMET::IAnalysisUtilities::IsInStoppingTarget;
using COMET::IAnalysisUtilities::CheckVolumePath;
using COMET::IAnalysisUtilities::GetParticleFluxVolumes;
using COMET::IAnalysisUtilities::VolumeNames;
using COMET::IAnalysisUtilities::NameSet;

bool COMET::IMuonTargetTrajectoryAnalyzer::SetupHistograms(){
    COMET::IOARuntimeParamBlock params(GetDatumName());

    std::string title = "Particle Multiplicity";
    std::string hist_name = "hMultiplicity";
    hMultiplicity =
        new COMET::IPIDMultiHist("IH1F",GetDatumName(),hist_name,title);
    hMultiplicity->Setup("No. Particles per event",100,0,100);
    AddHistogram(hMultiplicity);

    title = "Number of trajectory points";
    hist_name = "hNoTrajectoryPoints";
    hNoTrajectoryPoints =
        new COMET::IPIDMultiHist("IH1F",GetDatumName(),hist_name,title);
    hNoTrajectoryPoints->Setup("No. Trajectory Points",100,0,100);
    AddHistogram(hNoTrajectoryPoints);

    title = "Parent-Daughter Tally";
    hist_name = "hParentDaughterTally";
    hParentDaughterTally =
        new COMET::I2DParticleTally(GetDatumName(),hist_name,title);
    hParentDaughterTally->Setup("Parent Type",100,0,100,"Daughter Type",1,0,1);
    hParentDaughterTally->SetDrawStyle("colz text30");
    AddHistogram(hParentDaughterTally);

    title = "Momentum of {particle}";
    hist_name = "hMomentum";
    hMomentum = new COMET::IPIDMultiHist("IH1F",GetDatumName(),hist_name,title);
    hMomentum->Setup("Momentum (MeV/c)",1e3,0,150);
    hMomentum->CombineAllPlots("Momentum of all particles");
    hMomentum->SetDrawStyle("nostack");
    AddHistogram(hMomentum);

    title = "Momentum of {particle} Coming from Muons";
    hist_name = "hMomentumMuonProducts";
    hMomentumMuonProducts = new
        COMET::IPIDMultiHist("IH1F",GetDatumName(),hist_name,title);
    hMomentumMuonProducts->Setup("Momentum (MeV/c)",1e3,0,150);
    hMomentumMuonProducts->CombineAllPlots("Momentum of Muon Products");
    hMomentumMuonProducts->SetDrawStyle("nostack");
    AddHistogram(hMomentumMuonProducts);

    title = "Kinetic Energy of All Particles";
    hist_name = "hKinetic";
    hKinetic = new COMET::IPIDMultiHist("IH1F",GetDatumName(),hist_name,title);
    hKinetic->Setup("Kinetic Energy (MeV/c^{2})",1e3,0,150);
    hKinetic->SetDrawStyle("nostack");
    AddHistogram(hKinetic);

    title = "Kinetic Energy of Muon Products";
    hist_name = "hKineticMuonProducts";
    hKineticMuonProducts =
        new COMET::IPIDMultiHist("IH1F",GetDatumName(),hist_name,title);
    hKineticMuonProducts->Setup("Kinetic Energy (MeV/c^{2})",1e3,0,150);
    hKineticMuonProducts->SetDrawStyle("nostack");
    AddHistogram(hKineticMuonProducts);

    title = "Branching Ratio For Muon Products";
    hist_name = "hRateMuonProducts";
    hRateMuonProducts =
        new COMET::IPIDMultiHist("ISingleBin",GetDatumName(),hist_name,title);
    hRateMuonProducts->SetDrawStyle("text30 h");
    hRateMuonProducts->GroupDynamicParticlesTogether(false);
    AddHistogram(hRateMuonProducts);

    title = "Branching Ratio For Muon Products";
    hist_name = "hRateMuonProductsDynamicCombined";
    hRateMuonProductsDynamicCombined =
        new COMET::IPIDMultiHist("ISingleBin",GetDatumName(),hist_name,title);
    hRateMuonProductsDynamicCombined->SetDrawStyle("text30 h");
    AddHistogram(hRateMuonProductsDynamicCombined);

    title = "Branching Ratio For Muon Products, p > 0.5 MeV/c";
    hist_name = "hRateMuonProductsMomCut";
    hRateMuonProductsMomCut =
        new COMET::IPIDMultiHist("ISingleBin",GetDatumName(),hist_name,title);
    hRateMuonProductsMomCut->SetDrawStyle("text30 h");
    AddHistogram(hRateMuonProductsMomCut);

    title = "Timing of Muon Products (Global Time)";
    hist_name = "hGlobalTimeMuonProducts";
    hGlobalTimeMuonProducts =
        new COMET::IPIDMultiHist("IH1F",GetDatumName(),hist_name,title);
    hGlobalTimeMuonProducts->Setup("Time (ns)",1e3,0,1e3);
    hGlobalTimeMuonProducts->SetDrawStyle("nostack");
    AddHistogram(hGlobalTimeMuonProducts);

    title = "\\mathrm{Timing\\,of\\,Muon\\,Products\\,}(t_{0}"
            "= t_\\mathrm{MuonEnd})";
    hist_name = "hLifeTimeMuonProducts";
    hLifeTimeMuonProducts =
        new COMET::IPIDMultiHist("IH1F",GetDatumName(),hist_name,title);
    hLifeTimeMuonProducts->Setup("Time (ns)",1e3,0,1e3);
    hLifeTimeMuonProducts->SetDrawStyle("nostack");
    AddHistogram(hLifeTimeMuonProducts);

    double x_min = params.GetDouble("x_low",-1e3);
    double x_max = params.GetDouble("x_high",1e4);
    double y_min = params.GetDouble("y_low",-1e3);
    double y_max = params.GetDouble("y_high",1e4);
    double z_min = params.GetDouble("z_low",-1e3);
    double z_max = params.GetDouble("z_high",1e4);

    title = "Position of First Trajectory Point";
    hist_name = "hPosition";
    hPosition = new COMET::IVolumePlot(GetDatumName(),hist_name,title);
    hPosition->Setup("X (mm)",1e3,x_min,x_max,
                     "Y (mm)",1e3,y_min,y_max,
                     "Z (mm)",1e3,z_min,z_max);
    hPosition->SetDrawStyle("colz");
    hPosition->SetDirectory(GetOutputDirectory());
    AddHistogram(hPosition);

    title = "Position of First Trajectory Point For Muon Products";
    hist_name = "hPositionMuonProducts";
    hPositionMuonProducts =
        new COMET::IVolumePlot(GetDatumName(),hist_name,title);
    hPositionMuonProducts->Setup("X (mm)",1e3,x_min,x_max,
                                 "Y (mm)",1e3,y_min,y_max,
                                 "Z (mm)",1e3,z_min,z_max);
    hPositionMuonProducts->SetDrawStyle("colz");
    hPositionMuonProducts->SetDirectory(GetOutputDirectory());
    AddHistogram(hPositionMuonProducts);

    title = "Counts of Muons Passing Through Stopping Target Disks";
    hist_name = "hPassMuonDisk";
    hPassMuonDisk = new COMET::IH1F(GetDatumName(), hist_name, title);
    hPassMuonDisk->Setup("Disk ID", 30, 0, 30);
    hPassMuonDisk->SetDirectory(GetOutputDirectory());
    AddHistogram(hPassMuonDisk);

    title = "Counts of All Passing Through Stopping Target Disks";
    hist_name = "hPassAllDisk";
    hPassAllDisk = new COMET::IH1F(GetDatumName(), hist_name, title);
    hPassAllDisk->Setup("Disk ID", 30, 0, 30);
    hPassAllDisk->SetDirectory(GetOutputDirectory());
    AddHistogram(hPassAllDisk);

    title = "Counts of Stopped Muons in Target Disks";
    hist_name = "hStopMuonDisk";
    hStopMuonDisk = new COMET::IH1F(GetDatumName(), hist_name, title);
    hStopMuonDisk->Setup("Disk ID", 30, 0, 30);
    hStopMuonDisk->SetDirectory(GetOutputDirectory());
    AddHistogram(hStopMuonDisk);

    return true;
}

bool COMET::IMuonTargetTrajectoryAnalyzer::AnalyzeTrajectory(
        const IG4Trajectory& trajectory,
        const IHandle<IG4TrajectoryContainer>& trajectories){

    // Get the PID code
    const int pid = trajectory.GetPDGEncoding();
    // Get the track id
    const int trackID = trajectory.GetTrackId();
    // Check if its a muon
    bool isMuon = pid == 13;
    bool isPion = pid == -211;
    if(isMuon) ++fAllTrajData.nMuons;
    // Record this multiplicity
    ++fAllTrajData.fMultiplicities[pid];
    // Record the number of trajectory point
    hNoTrajectoryPoints->FillWith(pid, trajectory.GetTrajectoryPoints().size());

    // Get some parent information
    const COMET::IHandle<COMET::IG4Trajectory> parent =
        trajectories->GetTrajectory(trajectory.GetParentId());
    // Default values
    int parent_pid = -1;
    bool parent_was_muon = false;
    if(parent) {
        // Get the real values
        int parent_pid = parent->GetPDGEncoding();
        bool parent_was_muon = (parent_pid == 13);
        // Fill the parent daughter tally
        hParentDaughterTally->FillWith(parent_pid, pid);
        // Record the number of muon secondaries
        // TODO name nMuonSecondaries as n_muon_secondaries
        if(parent_was_muon) ++fAllTrajData.nMuonSecondaries;
    }

    // Get trajs initial/final position and momentum
    TLorentzVector momentum = trajectory.GetInitialMomentum();
    TLorentzVector pos4_i = trajectory.GetInitialPosition();
    TLorentzVector pos4_f = trajectory.GetFinalPosition();
    // Fill the histograms
    hMomentum->FillWith(pid, momentum.P());
    hPosition->FillWith(pos4_i.X(), pos4_i.Y(), pos4_i.Z());
    hKinetic->FillWith(pid, momentum.E()-momentum.Mag());

    // Check the flux through each disk
    VolumeNames matchNames;
    matchNames.push_back("TargetDisk");
    matchNames.push_back("MuonStoppingTarget");
    // Get the volume name set to fill
    NameSet passedDisks;
    GetParticleFluxVolumes(trajectory, passedDisks, true, &matchNames);
    // Check if the particle stopped or not
    bool stoppedPart = ParticleStoppedInStopTgt(trajectory);
    // Fill the total count
    if (passedDisks.size() > 0) {
        // Check if it is stopped eventually or not
        if (!stoppedPart) {
            if (isMuon) ++fAllTrajData.nNotStoppedMuons;
            ++fAllTrajData.nNotStoppedAll;
        }
        // Iterate through the contents
        for (NameSet::const_iterator aDisk = passedDisks.begin();
                  aDisk != passedDisks.end();
                ++aDisk){
            // Get the disk ID
            int diskID = GetMSTDiskID(*aDisk);
            if (isMuon) hPassMuonDisk->Fill(diskID);
            hPassAllDisk->Fill(diskID);
        }
    }

    // Check that this particle is a stopped muon
    if(isMuon && stoppedPart){
        // Increment total number of stopped muons
        TVector3 pos3_f = pos4_f.Vect();
        hStopMuonDisk->Fill(GetMSTDiskID(pos3_f));
    }

    // Check if this particle is a stopped pion
    if (isPion && stoppedPart) ++fAllTrajData.nStoppedPions;

    // Check secondaries of muons that are created inside the target
    if(parent_was_muon){
        // Check initial pos of trajectory is inside the muon stopping target
        TVector3 pos3_i = pos4_i.Vect();
        if(IsInTargetOrSupport(pos3_i)){
            // Count this particle
            ++fAllTrajData.nMuonSecondariesInTgt;
            // Fill all histograms relating to stopped muon products
            hMomentumMuonProducts->FillWith(pid, momentum.P());
            hKineticMuonProducts->FillWith(pid, momentum.E()-momentum.Mag());
            hRateMuonProducts->FillWith(pid,1);
            hRateMuonProductsDynamicCombined->FillWith(pid,1);
            if(momentum.P()>0.5) hRateMuonProductsMomCut->FillWith(pid,1);
            hPositionMuonProducts->FillWith(pos4_i.X(),pos4_i.Y(),pos4_i.Z());
            hGlobalTimeMuonProducts->FillWith(pid,pos4_i.T());
            hLifeTimeMuonProducts->FillWith(pid,pos4_i.T() -
                                            parent->GetInitialPosition().T());
        }
    }
    return true;
}

bool COMET::IMuonTargetTrajectoryAnalyzer::EventStart(){
    fAllTrajData.Reset();
    return true;
}

bool COMET::IMuonTargetTrajectoryAnalyzer::Analyze(const IHandle<IDatum> data){

    COMET::IHandle<COMET::IG4TrajectoryContainer> trajectories =
        data->Get<COMET::IG4TrajectoryContainer>(".");
    if (trajectories) {
       for(IG4TrajectoryContainer::const_iterator i_traj = trajectories->begin();
                                                  i_traj != trajectories->end();
                                                ++i_traj){
           bool ok = AnalyzeTrajectory(i_traj->second,trajectories);
           if(!ok) return false;
       }

       // Summary  data for this event
       for(auto i_tally = fAllTrajData.fMultiplicities.begin();
                i_tally != fAllTrajData.fMultiplicities.end();
              ++i_tally){
           hMultiplicity->FillWith(i_tally->first, i_tally->second);
       }

    } else return false;
    return true;
}

bool COMET::IMuonTargetTrajectoryAnalyzer::FinalizeHistograms(){
    // Get the number of stopped muons
    int nStoppedMuons = hStopMuonDisk->Hist()->GetEntries();

    // finding total stopping rate
    int nStoppedMuonsAll = hStopMuonDisk->GetEntries();
    int nPassedMuonAll = hPassMuonDisk->GetEntries();
    std::vector<double> totalStoppingRate(nStoppedMuonsAll/(double) (nStoppedMuonsAll+nPassedMuonAll));
    ofstream txtfile;
    txtfile.open ("stoppingRate.txt", std::ios_base::app);
    txtfile << totalStoppingRate << "\n";
    txtfile.close();

    // Normalize the distributions
    hRateMuonProducts->Normalize(100./nStoppedMuons);
    hRateMuonProductsDynamicCombined->Normalize(100./nStoppedMuons);
    hRateMuonProductsMomCut->Normalize(100./nStoppedMuons);

    // Log the summary data
    COMET::IH1F* hSummaryData =
        new COMET::IH1F(GetDatumName(),"hSummaryData","Summary Numbers for Run");
    hSummaryData->Setup("",8,0,8);
    hSummaryData->Fill("Total Events",fAllTrajData.nEvents);
    hSummaryData->Fill("Total Muons",fAllTrajData.nMuons);
    hSummaryData->Fill("Not Stopped Muons",fAllTrajData.nNotStoppedMuons);
    hSummaryData->Fill("Stopped Muons",nStoppedMuons);
    hSummaryData->Fill("Stopped Pions",fAllTrajData.nStoppedPions);
    hSummaryData->Fill("Muon Secondaries",fAllTrajData.nMuonSecondaries);
    hSummaryData->Fill("Muon Secondaries in Tgt",
                       fAllTrajData.nMuonSecondariesInTgt);
    hSummaryData->Fill("Not Stopped All",fAllTrajData.nNotStoppedAll);
    hSummaryData->SetDrawStyle("h text30");
    AddHistogram(hSummaryData);

    return true;
}

int COMET::IMuonTargetTrajectoryAnalyzer::GetMSTDiskID(
        const std::string& nodePath){
    // Check where the muon stopping target is mentioned
    TString targetName("TargetDisk_");
    int stringIndex = TString(nodePath.c_str()).Index(targetName);
    stringIndex += targetName.Length();
    // Check where the next underscore is
    TString subString = TString(nodePath.substr(stringIndex));
    int uSrcIdx = subString.Index("_");
    // Return the integer mentioned in this substring
    TString returnVal = subString(0, uSrcIdx);
    return returnVal.Atoi();
}

int COMET::IMuonTargetTrajectoryAnalyzer::GetMSTDiskID(const TVector3& pos3){
    // Get the volume from this position in ROOT geometry units (cm)
    TVector3 pos3_cm = pos3*(1/unit::cm);
    std::string nodePath = FullNodePath(pos3_cm);
    return GetMSTDiskID(nodePath);
}

bool COMET::IMuonTargetTrajectoryAnalyzer::IsInStoppingTargetSupport(
        const char* nodePath){
    // Check the node names
    VolumeNames toMatch;
    toMatch.push_back("Support");
    toMatch.push_back("MuonStoppingTarget");
    return CheckVolumePath(nodePath, &toMatch);
}

bool COMET::IMuonTargetTrajectoryAnalyzer::IsInStoppingTargetSupport(
        const TVector3& pos3){
    // Get the volume from this position in ROOT geometry units (cm)
    TVector3 pos3_cm = pos3*(1/unit::cm);
    return IsInStoppingTargetSupport(FullNodePath(pos3_cm).c_str());
}

bool COMET::IMuonTargetTrajectoryAnalyzer::IsInTargetOrSupport(
        const TVector3& pos3){
    // Get the volume from this position in ROOT geometry units (cm)
    TVector3 pos3_cm = pos3*(1/unit::cm);
    const char* nodePath = FullNodePath(pos3_cm).c_str();
    // Return if it is in eithe the support or in the target
    return IsInStoppingTarget(nodePath) || IsInStoppingTargetSupport(nodePath);
}

REGISTER_ANALYZER(IMuonTargetTrajectoryAnalyzer)
