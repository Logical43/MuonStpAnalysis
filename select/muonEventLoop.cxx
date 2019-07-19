#include <cometEventLoop.hxx>
#include <memory>
#include <TTree.h>

class TMySimpleEventLoop: public COMET::ICOMETEventLoopFunction {
public:
    TMySimpleEventLoop() {}
    virtual ~TMySimpleEventLoop() {}

    virtual void Initialize(void) {
        fOutputTree = new TTree("userLoopTree","A simple summary tree");
        fOutputTree->Branch("NP0DHits", &fNP0DHits, "NP0DHits/I");
    }

    bool operator () (COMET::ICOMETEvent& event) {
       // Get the P0D hits
       COMET::IHandle<COMET::IHitSelection> p0d(event.GetHitSelection("p0d"));
       // Update Tree values
       if (p0d) fNP0DHits = p0d->size();
       else  fNP0DHits = 0;
       fOutputTree->Fill();
        return false; // Don't save events...
    }
 
    // Called if the output file is open.  Notice there are two forms...
    void Finalize(COMET::ICOMETOutput& output) {
        fOutputTree->Write();    
    }

private:
    TTree* fOutputTree;
    int    fNP0DHits;
};

int main(int argc, char **argv) {
    TMySimpleEventLoop userCode;
    cometEventLoop(argc,argv,userCode);
}
