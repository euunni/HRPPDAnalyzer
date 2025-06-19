#ifndef HRPPD_NTUPLER_H
#define HRPPD_NTUPLER_H

#include <string>
#include <vector>
#include "TFile.h"
#include "TTree.h"


namespace HRPPD {
    class Ntupler {
    public:
        Ntupler();
        ~Ntupler();

        // Convert .dat file to .root
        bool Convert(int runNumber, int numEvents = -1, 
                    const std::string& dataBasePath = "", 
                    const std::string& ntuplePath = "");
        
        // Utility functions
        static bool Check(int runNumber, const std::string& ntuplePath = "");
        static std::string GetPath(int runNumber, const std::string& ntuplePath = "");
        
    private:
        std::string fRawDataPath;  // Path where .dat files are located
        std::string fNtuplePath;    // Path to save ntuple files
    };
}

#endif // HRPPD_NTUPLER_H 