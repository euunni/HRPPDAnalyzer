#include "../include/Ntupler.h"
#include "../include/Config.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <libgen.h>
#include "TString.h"

namespace HRPPD {

Ntupler::Ntupler() : 
    rawDataPath_(CONFIG_RAWDATA_PATH),
    ntuplePath_(CONFIG_NTUPLE_PATH) {
    // Create output directory if it doesn't exist
    if (!ntuplePath_.empty()) {
        mkdir(ntuplePath_.c_str(), 0755);
    }
}

Ntupler::~Ntupler() {
}

bool Ntupler::Check(int runNumber, const std::string& ntuplePath) {
    std::string ntuplefile = GetPath(runNumber, ntuplePath);
    
    // Check if file exists
    std::ifstream file(ntuplefile);
    return file.good();
}   

std::string Ntupler::GetPath(int runNumber, const std::string& ntuplePath) {
    std::string path = ntuplePath.empty() ? CONFIG_NTUPLE_PATH : ntuplePath;
    return path + "/MCP_Run_" + std::to_string(runNumber) + "_ntuple.root";
}

bool Ntupler::Convert(int runNumber, int numEvents, 
                              const std::string& dataBasePath, 
                              const std::string& ntuplePath) {
    // Update paths if provided as arguments
    if (!dataBasePath.empty()) rawDataPath_ = dataBasePath;
    if (!ntuplePath.empty()) ntuplePath_ = ntuplePath;
    
    if (!ntuplePath_.empty()) {
        mkdir(ntuplePath_.c_str(), 0755);
    }
    
    std::string runDir;
    if (runNumber < 136) {
        runDir = rawDataPath_ + "/run" + std::to_string(runNumber);
    } else if (runNumber >= 147 && runNumber <= 165) {
        runDir = rawDataPath_ + "/1.4T Angle scan/run" + std::to_string(runNumber);
    } else {
        runDir = rawDataPath_ + "/Run after 135/run" + std::to_string(runNumber);
    }
    std::string outputFileName = GetPath(runNumber, ntuplePath_);
    
    std::cout << "== Ntuplizing Run " << runNumber << " ==" << std::endl;
    std::cout << "Data path: " << runDir << std::endl;
    std::cout << "Output file: " << outputFileName << std::endl;
    
    // Verify and create output directory
    size_t slashPos = outputFileName.find_last_of('/');
    if (slashPos != std::string::npos) {
        std::string dirPath = outputFileName.substr(0, slashPos);
        mkdir(dirPath.c_str(), 0755);
    }
    
    TFile* outFile = new TFile(outputFileName.c_str(), "RECREATE");
    if (!outFile || outFile->IsZombie()) {
        std::cerr << "Error: Failed to create output file - " << outputFileName << std::endl;
        return false;
    }
    
    TTree* tree = new TTree("MCPTree", "MCP Raw Waveform Data");
    
    int eventNum;
    std::vector<float> triggerWave(1024, 0.0); 
    std::vector<std::vector<float>> mcpWaves; 
    
    // Initialize mcpWaves with proper size
    mcpWaves.resize(16);
    for (int ch = 0; ch < 16; ch++) {
      mcpWaves[ch].resize(1024, 0.0); 
    }
    
    tree->Branch("eventNumber", &eventNum, "eventNum/I");
    tree->Branch("triggerWave", &triggerWave);
    
    // Create MCP channel branches (channels 0-15)
    for (int ch = 0; ch < 16; ch++) {
        TString branchName = Form("mcpWave%d", ch);
        tree->Branch(branchName, &mcpWaves[ch]);
    }
    
    // Open trigger file
    std::string triggerFile = runDir + "/TR_0_0.dat";
    std::ifstream trigFile(triggerFile, std::ios::binary | std::ios::ate);
    
    if (!trigFile) {
        std::cerr << "Error: Cannot open trigger file - " << triggerFile << std::endl;
        outFile->Close();
        delete outFile;
        return false;
    }
    
    // Calculate total events
    std::streamsize fileSize = trigFile.tellg();
    const int binSize = sizeof(float);
    const int binsPerEvent = 1024;
    const int eventSize = binSize * binsPerEvent;
    
    int totalEvents = fileSize / eventSize;
    std::cout << "Found " << totalEvents << " events in run " << runNumber << std::endl;
    
    // Set number of events to process
    if (numEvents <= 0 || numEvents > totalEvents) {
        numEvents = totalEvents;
    }
    std::cout << "Will process " << numEvents << " events" << std::endl;
    
    // Reset file pointer
    trigFile.seekg(0, std::ios::beg);
    
    // Open all channel files
    std::vector<std::ifstream> chFiles;
    chFiles.reserve(16);
    
    for (int ch = 0; ch < 16; ch++) {
        std::string chFileName = runDir + "/wave_" + std::to_string(ch) + ".dat";
        chFiles.emplace_back(chFileName, std::ios::binary);
        
        if (!chFiles[ch]) {
            std::cout << "Warning: Cannot open channel " << ch << " file - " << chFileName << std::endl;
            std::cout << "Channel will be filled with zeros." << std::endl;
        }
    }
    
    // Process events
    for (eventNum = 0; eventNum < numEvents; eventNum++) {
        if (eventNum % 1000 == 0) {
            std::cout << "Processing event: " << eventNum << "/" << numEvents << std::endl;
        }
        
        // Read trigger waveform data
        triggerWave.clear();
        triggerWave.resize(1024);
        for (int bin = 0; bin < 1024; bin++) {
            float value;
            trigFile.read((char*)&value, sizeof(float));
            triggerWave[bin] = value;
        }
        
        // Read MCP channel waveform data
        for (int ch = 0; ch < 16; ch++) {
            mcpWaves[ch].clear();
            mcpWaves[ch].resize(1024);
            
            if (chFiles[ch]) {
                for (int bin = 0; bin < 1024; bin++) {
                    float value;
                    chFiles[ch].read((char*)&value, sizeof(float));
                    mcpWaves[ch][bin] = value;
                }
            } else {
                // Fill with zeros if file cannot be opened
                for (int bin = 0; bin < 1024; bin++) {
                    mcpWaves[ch][bin] = 0.0;
                }
            }
        }
        tree->Fill();
    }
    
    trigFile.close();
    for (auto& file : chFiles) {
        if (file.is_open()) {
            file.close();
        }
    }
    
    outFile->cd();
    tree->Write();
    outFile->Close();
    
    std::cout << numEvents << " events processed" << std::endl;
    std::cout << "Output file: " << outputFileName << std::endl;
    
    delete outFile;
    
    return true;
}

} // namespace HRPPD 