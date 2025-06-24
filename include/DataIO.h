#ifndef HRPPD_DATAIO_H
#define HRPPD_DATAIO_H

#include <string>
#include <vector>
#include <memory>
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"


namespace HRPPD {
    class Ntupler;
    
    class DataIO {
    public:
        DataIO();
        ~DataIO();
        
        // File management
        bool Load(int runNumber, const int channelNumber, bool autoNtuplize = true);
        bool SetFile(const std::string& fileName);
        void Close(const std::string& fileName = "");
        
        // Event data access
        bool GetEvent(int eventIndex);
        int GetEntries() const;
        std::vector<float> GetWaveform(const std::string& type) const;
        
        // Output management
        void Save(TH1* hist, const std::string& dirName = "");
        void SetDir(const std::string& dirName);
        void SetPath(const std::string& outputPath);
        
    private:
        TFile* fInputFile = nullptr;
        TFile* fOutputFile = nullptr;
        TTree* fTree = nullptr; 
        
        int fEventNum = 0;
        std::vector<float>* fTriggerWaveform = nullptr;
        std::vector<float>* fMcpWaveform = nullptr;
        int fChannelNumber = 0;
        
        std::string fNtuplePath = "./data";
        std::string fOutputPath = "./output";
        
        bool fAutoNtuplize = true;
        
        std::unique_ptr<Ntupler> fNtupler;
    };
}

#endif // HRPPD_DATAIO_H 