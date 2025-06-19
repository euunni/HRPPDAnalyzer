#include "../include/DataIO.h"
#include "../include/Ntupler.h"
#include <iostream>
#include <sys/stat.h>
#include <libgen.h>
#include "TString.h"

namespace HRPPD {

DataIO::DataIO() : 
    inputFile_(nullptr), outputFile_(nullptr), tree_(nullptr),
    triggerWaveform_(nullptr), mcpWaveform_(nullptr),
    dataPath_("../data"), outputPath_("../output"),
    autoNtuplize_(true), ntupler_(nullptr) {
}

DataIO::~DataIO() {
    CloseInputFile();
    CloseOutputFile();
    
    if (ntupler_) {
        delete ntupler_;
        ntupler_ = nullptr;
    }
}

void DataIO::SetDataPath(const std::string& dataPath) {
    dataPath_ = dataPath;
}

void DataIO::SetOutputPath(const std::string& outputPath) {
    outputPath_ = outputPath;
}

void DataIO::SetAutoNtuplize(bool enable) {
    autoNtuplize_ = enable;
}

bool DataIO::OpenInputFileByRun(int runNumber, const int channelNumber, bool autoNtuplize) {
    std::string ntuplePath = Ntupler::GetNtuplePath(runNumber, dataPath_);
    
    bool ntupleExists = Ntupler::CheckNtupleExists(runNumber, dataPath_);
    if (!ntupleExists && autoNtuplize) {
        std::cout << "Ntuple file does not exist: " << ntuplePath << std::endl;
        std::cout << "Starting automatic ntuplizing..." << std::endl;
        
        // Create Ntupler instance if not existing
        if (!ntupler_) {
            ntupler_ = new Ntupler();
        }
        
        // Ntupler now handles paths internally, we just pass empty strings
        // to use the default paths in the Ntupler class
        bool success = ntupler_->ConvertDatToRoot(runNumber, -1);
        if (!success) {
            std::cerr << "Ntuplizing failed" << std::endl;
            return false;
        }
        
        std::cout << "Ntuplizing completed" << std::endl;
    } else if (!ntupleExists) {
        std::cerr << "Error: Ntuple file does not exist and auto-ntuplizing is disabled" << std::endl;
        return false;
    }
    
    return OpenInputFile(ntuplePath, channelNumber);
}

bool DataIO::OpenInputFile(const std::string& fileName, const int channelNumber) {
    CloseInputFile();
    
    inputFile_ = TFile::Open(fileName.c_str(), "READ");
    if (!inputFile_ || inputFile_->IsZombie()) {
        std::cerr << "Error: Failed to open file - " << fileName << std::endl;
        return false;
    }
    
    tree_ = (TTree*)inputFile_->Get("MCPTree");
    if (!tree_) {
        std::cerr << "Error: Cannot find ntuple tree" << std::endl;
        CloseInputFile();
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

bool DataIO::CreateOutputFile(const std::string& fileName) {
    CloseOutputFile();
    
    size_t slashPos = fileName.find_last_of('/');
    if (slashPos != std::string::npos) {
        std::string dirPath = fileName.substr(0, slashPos);
        mkdir(dirPath.c_str(), 0755);
    }
    
    outputFile_ = new TFile(fileName.c_str(), "RECREATE");
    if (!outputFile_ || outputFile_->IsZombie()) {
        std::cerr << "Error: Failed to create output file - " << fileName << std::endl;
        return false;
    }
    
    return true;
}

void DataIO::CloseInputFile() {
    if (inputFile_) {
        inputFile_->Close();
        delete inputFile_;
        inputFile_ = nullptr;
        tree_ = nullptr; // Tree belongs to file
    }
}

void DataIO::CloseOutputFile() {
    if (outputFile_) {
        outputFile_->Write();
        outputFile_->Close();
        delete outputFile_;
        outputFile_ = nullptr;
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

int DataIO::GetEventNumber() const {
    return eventNum_;
}

int DataIO::GetTotalEvents() const {
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

void DataIO::SaveHistogram(TH1* hist, const std::string& dirName) {
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

void DataIO::CreateDirectory(const std::string& dirName) {
    if (!outputFile_) {
        return;
    }
    
    if (!outputFile_->GetDirectory(dirName.c_str())) {
        outputFile_->mkdir(dirName.c_str());
    }
}

} // namespace HRPPD 