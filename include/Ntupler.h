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
        bool ConvertDatToRoot(int runNumber, int numEvents = -1, 
                             const std::string& dataBasePath = "", 
                             const std::string& ntuplePath = "");
        
        // Utility functions
        static bool CheckNtupleExists(int runNumber, const std::string& ntuplePath = "");
        static std::string GetNtuplePath(int runNumber, const std::string& ntuplePath = "");
        
    private:
        std::string dataBasePath_;  // Path where .dat files are located
        std::string ntuplePath_;    // Path to save ntuple files
    };
}

#endif // HRPPD_NTUPLER_H 