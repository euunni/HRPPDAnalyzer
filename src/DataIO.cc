#include "../include/DataIO.h"
#include "../include/Ntupler.h"
#include "../include/Config.h"

#include <iostream>
#include <sys/stat.h>
#include <libgen.h>
#include "TString.h"


namespace HRPPD {

DataIO::DataIO() : 
    ntuplePath_(CONFIG_NTUPLE_PATH), outputPath_(CONFIG_OUTPUT_PATH),
    autoNtuplize_(true) {
}

DataIO::~DataIO() {
    Close();
}

void DataIO::SetPath(const std::string& outputPath) {
    outputPath_ = outputPath;
}

bool DataIO::Load(int runNumber, const int channelNumber, bool autoNtuplize) {
    std::string ntuplePath = Ntupler::GetPath(runNumber, ntuplePath_);
    
    bool ntupleExists = Ntupler::Check(runNumber, ntuplePath_);
    if (!ntupleExists && autoNtuplize) {
        std::cout << "Ntuple file does not exist: " << ntuplePath << std::endl;
        std::cout << "Starting automatic ntuplizing..." << std::endl;
        
        // Create Ntupler instance if not existing
        if (!ntupler_) {
            ntupler_ = std::make_unique<Ntupler>();
        }
        
        bool success = ntupler_->Convert(runNumber, -1);
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
    inputFile_.reset(TFile::Open(ntuplePath.c_str(), "READ"));
    if (!inputFile_ || inputFile_->IsZombie()) {
        std::cerr << "Error: Failed to open file - " << ntuplePath << std::endl;
        return false;
    }
    
    tree_ = (TTree*)inputFile_->Get("MCPTree");
    if (!tree_) {
        std::cerr << "Error: Cannot find ntuple tree" << std::endl;
        Close(ntuplePath);
        return false;
    }
    
    triggerWaveform_ = nullptr;
    mcpWaveform_ = nullptr;
    channelNumber_ = channelNumber;
    
    tree_->SetBranchAddress("eventNumber", &eventNum_);
    tree_->SetBranchAddress("triggerWave", &triggerWaveform_);
    
    // Connect MCP channel branch
    TString mcpBranchName = Form("mcpWave%d", channelNumber_);
    if (tree_->GetBranch(mcpBranchName)) {
        tree_->SetBranchAddress(mcpBranchName, &mcpWaveform_);
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
    
    outputFile_.reset(new TFile(fileName.c_str(), "RECREATE"));
    if (!outputFile_ || outputFile_->IsZombie()) {
        std::cerr << "Error: Failed to create output file - " << fileName << std::endl;
        return false;
    }
    
    return true;
}

void DataIO::Close(const std::string& fileName) {
    // If file name is empty, close all files
    if (fileName.empty()) {
        if (inputFile_) {
            tree_ = nullptr; // Tree belongs to file
            inputFile_.reset();
        }
        
        if (outputFile_) {
            outputFile_->Write();
            outputFile_.reset();
        }
        return;
    }
    
    // If file name is given, close only that file
    if (inputFile_ && fileName == inputFile_->GetName()) {
        tree_ = nullptr; // Tree belongs to file
        inputFile_.reset();
    }
    else if (outputFile_ && fileName == outputFile_->GetName()) {
        outputFile_->Write();
        outputFile_.reset();
    }
}

bool DataIO::GetEvent(int eventIndex) {
    if (!tree_) {
        std::cerr << "Error: Tree not loaded" << std::endl;
        return false;
    }
    
    if (eventIndex < 0 || eventIndex >= tree_->GetEntries()) {
        std::cerr << "Error: Event index out of range (" << eventIndex << "/" << tree_->GetEntries() << ")" << std::endl;
        return false;
    }
    
    tree_->GetEntry(eventIndex);
    return true;
}

int DataIO::GetEventN() const {
    return eventNum_;
}

int DataIO::GetEntries() const {
    return tree_ ? tree_->GetEntries() : 0;
}

std::vector<float> DataIO::GetWaveform(const std::string& type) const {
    std::vector<float> waveform;
    if (type == "trigger") {
        waveform = *triggerWaveform_;
    } 
    else if (type == "mcp") {
        waveform = *mcpWaveform_;
    }
    
    return waveform;
}

void DataIO::Save(TH1* hist, const std::string& dirName) {
    if (!outputFile_ || !hist) {
        return;
    }
    
    TDirectory* currentDir = gDirectory;
    
    if (!dirName.empty()) {
        TDirectory* dir = outputFile_->GetDirectory(dirName.c_str());
        if (!dir) {
            dir = outputFile_->mkdir(dirName.c_str());
        }
        dir->cd();
    } else {
        outputFile_->cd();
    }
    
    hist->Write();
    currentDir->cd();
}

void DataIO::SetDir(const std::string& dirName) {
    if (!outputFile_) {
        return;
    }
    
    if (!outputFile_->GetDirectory(dirName.c_str())) {
        outputFile_->mkdir(dirName.c_str());
    }
}

} // namespace HRPPD 