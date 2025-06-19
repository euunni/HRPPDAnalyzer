#ifndef HRPPD_DATAIO_H
#define HRPPD_DATAIO_H

#include <string>
#include <vector>
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
        bool OpenInputFile(const std::string& fileName, const int channelNumber);
        bool OpenInputFileByRun(int runNumber, const int channelNumber, bool autoNtuplize = true);
        bool CreateOutputFile(const std::string& fileName);
        void CloseInputFile();
        void CloseOutputFile();
        
        // Event data access
        bool GetEvent(int eventIndex);
        int GetEventNumber() const;
        int GetTotalEvents() const;
        std::vector<float> GetWaveform(const std::string& type) const;
        
        // Output management
        void SaveHistogram(TH1* hist, const std::string& dirName = "");
        void CreateDirectory(const std::string& dirName);
        
        // Ntuplizing settings
        void SetDataPath(const std::string& dataPath);
        void SetOutputPath(const std::string& outputPath);
        void SetAutoNtuplize(bool enable);
        
    private:
        TFile* inputFile_ = nullptr;
        TFile* outputFile_ = nullptr;
        TTree* tree_ = nullptr;
        
        int eventNum_ = 0;
        std::vector<float>* triggerWaveform_ = nullptr;
        std::vector<float>* mcpWaveform_ = nullptr;
        int channelNumber_ = 0;
        
        std::string dataPath_ = "./data";
        std::string outputPath_ = "./output";
        
        bool autoNtuplize_ = true;
        
        Ntupler* ntupler_ = nullptr;
    };
}

#endif // HRPPD_DATAIO_H 