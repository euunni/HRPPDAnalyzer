#include "../include/DataIO.h"
#include "../include/Ntupler.h"
#include "../include/Config.h"

#include <iostream>
#include <sys/stat.h>
#include <libgen.h>
#include "TString.h"


namespace HRPPD {

DataIO::DataIO() : 
    fNtuplePath(CONFIG_NTUPLE_PATH), fOutputPath(CONFIG_OUTPUT_PATH),
    fAutoNtuplize(true) {
}

DataIO::~DataIO() {
    Close();
}

void DataIO::SetPath(const std::string& outputPath) {
    fOutputPath = outputPath;
}

bool DataIO::Load(int runNumber, const int channelNumber, bool autoNtuplize) {
    std::string ntuplePath = Ntupler::GetPath(runNumber, fNtuplePath);
    
    bool ntupleExists = Ntupler::Check(runNumber, fNtuplePath);
    if (!ntupleExists && autoNtuplize) {
        std::cout << "Ntuple file does not exist: " << ntuplePath << std::endl;
        std::cout << "Starting automatic ntuplizing..." << std::endl;
        
        // Create Ntupler instance if not existing
        if (!fNtupler) {
            fNtupler = std::make_unique<Ntupler>();
        }
        
        bool success = fNtupler->Convert(runNumber, -1);
        if (!success) {
            std::cerr << "Ntuplizing failed" << std::endl;
            return false;
        }
        
        std::cout << "Ntuplizing completed" << std::endl;
    } else if (!ntupleExists) {
        std::cerr << "Error: Ntuple file does not exist and auto-ntuplizing is disabled" << std::endl;
        return false;
    }
    
    Close(ntuplePath);
    
    // Open the ntuple file
    fInputFile = TFile::Open(ntuplePath.c_str(), "READ");
    if (!fInputFile || fInputFile->IsZombie()) {
        std::cerr << "Error: Failed to open file - " << ntuplePath << std::endl;
        return false;
    }
    
    fTree = (TTree*)fInputFile->Get("MCPTree");
    if (!fTree) {
        std::cerr << "Error: Cannot find ntuple tree" << std::endl;
        Close(ntuplePath);
        return false;
    }
    
    fTriggerWaveform = nullptr;
    fMcpWaveform = nullptr;
    fChannelNumber = channelNumber;
    
    fTree->SetBranchAddress("eventNumber", &fEventNum);
    fTree->SetBranchAddress("triggerWave", &fTriggerWaveform);
    
    // Connect MCP channel branch
    TString mcpBranchName = Form("mcpWave%d", fChannelNumber);
    if (fTree->GetBranch(mcpBranchName)) {
        fTree->SetBranchAddress(mcpBranchName, &fMcpWaveform);
    } else {
        std::cerr << "Warning: Branch " << mcpBranchName << " does not exist" << std::endl;
        return false;
    }
    
    return true;
}

bool DataIO::SetFile(const std::string& fileName) {
    Close(fileName);
    
    size_t slashPos = fileName.find_last_of('/');
    if (slashPos != std::string::npos) {
        std::string dirPath = fileName.substr(0, slashPos);
        mkdir(dirPath.c_str(), 0755);
    }
    
    fOutputFile = new TFile(fileName.c_str(), "RECREATE");
    if (!fOutputFile || fOutputFile->IsZombie()) {
        std::cerr << "Error: Failed to create output file - " << fileName << std::endl;
        return false;
    }
    
    return true;
}

void DataIO::Close(const std::string& fileName) {
    // If file name is empty, close all files
    if (fileName.empty()) {
        if (fInputFile) {
            fTree = nullptr; // Tree belongs to file
            fInputFile->Close();
            delete fInputFile;
            fInputFile = nullptr;
        }
        
        if (fOutputFile) {
            fOutputFile->Write();
            fOutputFile->Close();
            delete fOutputFile;
            fOutputFile = nullptr;
        }
        return;
    }
    
    // If file name is given, close only that file
    if (fInputFile && fileName == fInputFile->GetName()) {
        fTree = nullptr; // Tree belongs to file
        fInputFile->Close();
        delete fInputFile;
        fInputFile = nullptr;
    }
    else if (fOutputFile && fileName == fOutputFile->GetName()) {
        fOutputFile->Write();
        fOutputFile->Close();
        delete fOutputFile;
        fOutputFile = nullptr;
    }
}

bool DataIO::GetEvent(int eventIndex) {
    if (!fTree) {
        std::cerr << "Error: Tree not loaded" << std::endl;
        return false;
    }
    
    if (eventIndex < 0 || eventIndex >= fTree->GetEntries()) {
        std::cerr << "Error: Event index out of range (" << eventIndex << "/" << fTree->GetEntries() << ")" << std::endl;
        return false;
    }
    
    fTree->GetEntry(eventIndex);
    return true;
}

int DataIO::GetEventN() const {
    return fEventNum;
}

int DataIO::GetEntries() const {
    return fTree ? fTree->GetEntries() : 0;
}

std::vector<float> DataIO::GetWaveform(const std::string& type) const {
    std::vector<float> waveform;
    if (type == "trigger") {
        waveform = *fTriggerWaveform;
    } 
    else if (type == "mcp") {
        waveform = *fMcpWaveform;
    }
    
    return waveform;
}

void DataIO::Save(TH1* hist, const std::string& dirName) {
    if (!fOutputFile || !hist) {
        return;
    }
    
    TDirectory* currentDir = gDirectory;
    
    if (!dirName.empty()) {
        TDirectory* dir = fOutputFile->GetDirectory(dirName.c_str());
        if (!dir) {
            dir = fOutputFile->mkdir(dirName.c_str());
        }
        dir->cd();
    } else {
        fOutputFile->cd();
    }
    
    hist->Write();
    currentDir->cd();
}

void DataIO::SetDir(const std::string& dirName) {
    if (!fOutputFile) {
        return;
    }
    
    if (!fOutputFile->GetDirectory(dirName.c_str())) {
        fOutputFile->mkdir(dirName.c_str());
    }
}

} // namespace HRPPD 