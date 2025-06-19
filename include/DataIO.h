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
        int GetEventN() const;
        int GetEntries() const;
        std::vector<float> GetWaveform(const std::string& type) const;
        
        // Output management
        void Save(TH1* hist, const std::string& dirName = "");
        void SetDir(const std::string& dirName);
        void SetPath(const std::string& outputPath);
        
    private:
        std::unique_ptr<TFile> inputFile_;
        std::unique_ptr<TFile> outputFile_;
        TTree* tree_ = nullptr; 
        
        int eventNum_ = 0;
        std::vector<float>* triggerWaveform_ = nullptr;
        std::vector<float>* mcpWaveform_ = nullptr;
        int channelNumber_ = 0;
        
        std::string ntuplePath_ = "./data";
        std::string outputPath_ = "./output";
        
        bool autoNtuplize_ = true;
        
        std::unique_ptr<Ntupler> ntupler_;
    };
}

#endif // HRPPD_DATAIO_H 