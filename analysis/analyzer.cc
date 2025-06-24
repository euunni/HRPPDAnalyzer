#include "../include/DataIO.h"
#include "../include/WaveformProcessor.h"
#include "../include/EventAnalyzer.h"
#include "../include/Config.h"
#include "../include/Ntupler.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <memory>
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "TFile.h"
#include "TSystem.h"

using namespace HRPPD;


// Default configuration file path
const std::string DEFAULT_CONFIG_FILE = "../config/config.txt";

// Common IO setup function
bool Init(DataIO& dataIO, const int runNumber, const int channelNumber, 
             const std::string& outputSuffix, std::string& outputFileName) {

    outputFileName = Form("%s/run%d/%s_Run_%d.root", 
                         CONFIG_OUTPUT_PATH.c_str(), runNumber, outputSuffix.c_str(), runNumber);

    dataIO.SetPath(CONFIG_OUTPUT_PATH); 
    
    if (!dataIO.Load(runNumber, channelNumber, true)) {
        std::cerr << "Failed to open file: Run " << runNumber << ", Channel " << channelNumber << std::endl;
        return false;
    }
    
    if (!dataIO.SetFile(outputFileName)) {
        std::cerr << "Failed to create output file: " << outputFileName << std::endl;
        std::string ntuplePath = Ntupler::GetPath(runNumber, CONFIG_NTUPLE_PATH);
        dataIO.Close(ntuplePath);
        return false;
    }
    
    return true;
}


void analyzer(const int runNumber, const int channelNumber = 10, const int maxEvents = -1, 
              const std::string& configFile = DEFAULT_CONFIG_FILE, bool processAll = true,
              bool doWaveform = false, bool doWaveform2D = false, bool doToT = false,
              bool doTiming = false, bool doAmplitude = false, bool doNpe = false) {

    DataIO dataIO;
    WaveformProcessor processor;
    EventAnalyzer analyzer;
    
    if (!Load(configFile)) {
        std::cerr << "Failed to load config file, proceeding with default values." << std::endl;
    }
    
    // Set parameters
    processor.fCalibrationConstant = CONFIG_CALIBRATION_CONSTANT;
    processor.fDeltaT = CONFIG_DELTA_T;
    processor.fSamplingRate = CONFIG_SAMPLING_RATE;
    analyzer.fTriggerCfdFraction = CONFIG_TRIGGER_CFD_FRACTION;
    analyzer.fTriggerCfdDelay = CONFIG_TRIGGER_CFD_DELAY;
    analyzer.fMcpCfdFraction = CONFIG_MCP_CFD_FRACTION;
    analyzer.fMcpCfdDelay = CONFIG_MCP_CFD_DELAY;
    analyzer.fTriggerWindowMin = CONFIG_TRIGGER_WINDOW_MIN;
    analyzer.fTriggerWindowMax = CONFIG_TRIGGER_WINDOW_MAX;
    analyzer.fMcpWindowMin = CONFIG_MCP_WINDOW_MIN;
    analyzer.fMcpWindowMax = CONFIG_MCP_WINDOW_MAX;
    analyzer.fFftCutoffFrequency = CONFIG_FFT_CUTOFF_FREQUENCY;
    analyzer.fApplyFFTFilter = CONFIG_APPLY_FFT_FILTER;

    if (processAll) {
        doWaveform = doWaveform2D = doToT = doTiming = doAmplitude = doNpe = true;
    } else {
        if (!doWaveform && !doWaveform2D && !doToT && !doTiming && !doAmplitude && !doNpe) {
            doWaveform = CONFIG_DO_WAVEFORM;
            doWaveform2D = CONFIG_DO_WAVEFORM2D;
            doToT = CONFIG_DO_TOT;
            doTiming = CONFIG_DO_TIMING;
            doAmplitude = CONFIG_DO_AMPLITUDE;
            doNpe = CONFIG_DO_NPE;
        }
    }
    
    gSystem->mkdir(CONFIG_OUTPUT_PATH.c_str(), true);
    gSystem->mkdir(Form("%s/run%d", CONFIG_OUTPUT_PATH.c_str(), runNumber), true);
    
    std::cout << "=== Starting analysis for Run " << runNumber << ", Ch " << channelNumber << " ===" << std::endl;
    
    std::string outputFileName;
    if (!Init(dataIO, runNumber, channelNumber, "Analysis", outputFileName)) {
        std::cerr << "IO setup failed. Aborting analysis." << std::endl;
        return;
    }   
    
    if (doWaveform) {
        dataIO.SetDir("Waveforms_Trig");
        dataIO.SetDir("Waveforms_MCP");
        // dataIO.SetDir("Waveforms_MCP_filtered");
        dataIO.SetDir("CFD_Trig");
        dataIO.SetDir("CFD_MCP");
    }
    
    // Declare histograms using raw pointers
    TH2F* hTrig2D = nullptr;
    TH2F* hMCP2D = nullptr;
    if (doWaveform2D) {
        hTrig2D = new TH2F("Waveform_2D_Trig", "Trigger Waveforms 2D;Time [ns];Amplitude [mV]", 1000, 0., 200., 4000, -1000., 1000.);
        hMCP2D = new TH2F("Waveform_2D_MCP", "MCP Waveforms 2D;Time [ns];Amplitude [mV]", 1000, 0., 200., 4000, -1000., 1000.);
    }

    TH2F* hToT = nullptr;
    if (doToT) {
        hToT = new TH2F("ToT", "ToT;Amplitude [mV];Time [ps]", 100, 0., 60., 40, 0., 8000.);
    }
    
    // Timing histograms
    TH1F* hTrigTiming = nullptr;
    TH1F* hMCPTiming = nullptr;
    TH1F* hDiffTiming = nullptr;
    if (doTiming) {
        hTrigTiming = new TH1F("Timing_Trig", "Trigger Timing;Time [ps];Counts", 1000, 0., 120000.);
        hMCPTiming = new TH1F("Timing_MCP", "MCP Timing;Time [ps];Counts", 1000, 20000., 240000.);
        hDiffTiming = new TH1F("Timing_Diff", "Timing Resolution;Time [ps];Counts", 1000, 50000., 80000.);
    }
    
    // Amplitude histogram
    TH1F* hAmp = nullptr;
    if (doAmplitude) {
        hAmp = new TH1F("Amplitude", "MCP Amplitude;Amplitude [mV];Counts", 1000, 0., 100.);
    }
    
    // Npe histogram
    TH1F* hNpe = nullptr;
    if (doNpe) {    
        hNpe = new TH1F("Npe", "Number of Photoelectrons;Npe;Counts", 1000, 0., 10000000.);
    }
    
    analyzer.Init();
    

    // Event loop
    int totalEvents = dataIO.GetEntries();
    int processEvents = (maxEvents < 0) ? totalEvents : std::min(maxEvents, totalEvents);
    
    std::cout << "Processing " << processEvents << " events..." << std::endl;
    
    for (int evt = 0; evt < processEvents; evt++) {
        if (!dataIO.GetEvent(evt)) continue;
        if (evt % 1000 == 0) std::cout << "Processing event " << evt << "/" << processEvents << "..." << std::endl;
        
        int eventNum = dataIO.GetEventN();
        std::vector<float> trigWave = dataIO.GetWaveform("trigger");
        std::vector<float> mcpWave = dataIO.GetWaveform("mcp");
        
        // Correct waveforms
        std::vector<float> corrTrig = processor.Correct(trigWave);
        std::vector<float> corrMCP = processor.Correct(mcpWave);
        
        // Signal validation
        float amp = analyzer.GetAmp(corrMCP, analyzer.fMcpWindowMin, analyzer.fMcpWindowMax);
        float threshold = 4.0 * processor.GetStdDev(corrMCP);
        bool isSignal = (amp > threshold && processor.ToTCut(corrMCP, analyzer.fMcpWindowMin, analyzer.fMcpWindowMax));

        // Waveform analysis
        if (doWaveform && isSignal) {
            // FFT filtering
            std::vector<float> filtered = processor.FFTFilter(corrMCP, analyzer.fFftCutoffFrequency, eventNum, channelNumber);
            
            TH1F hTrig(Form("Trig_Wave_Evt%d", eventNum), Form("Trigger Waveform - Run %d, Event %d", runNumber, eventNum), 1000, 0, 200.);
            TH1F hMCP(Form("MCP_Wave_Evt%d_Ch%d", eventNum, channelNumber), Form("MCP Waveform - Run %d, Event %d, Ch %d", runNumber, eventNum, channelNumber), 1000, 0, 200.); 
            TH1F hMCP_filt(Form("MCP_Wave_Evt%d_Ch%d_filtered", eventNum, channelNumber), Form("MCP Waveform (Filtered) - Run %d, Event %d, Ch %d", runNumber, eventNum, channelNumber), 1000, 0, 200.);
            
            // Fill histograms
            for (int i = 0; i < 1000; i++) {
                hTrig.SetBinContent(i+1, corrTrig[i]);
                hMCP.SetBinContent(i+1, corrMCP[i]);
                hMCP_filt.SetBinContent(i+1, filtered[i]);
            }
            
            hMCP.GetYaxis()->SetRangeUser(-100., 50.);
            hMCP_filt.GetYaxis()->SetRangeUser(-100., 50.);
            
            dataIO.Save(&hTrig, "Waveforms_Trig");
            dataIO.Save(&hMCP, "Waveforms_MCP");
            // dataIO.Save(&hMCP_filt, "Waveforms_MCP_filtered");
        }

        // 2D waveform analysis
        if (doWaveform2D && isSignal) {
            for (int i = 0; i < 1000; i++) {
                double time = i * 0.2;
                hTrig2D->Fill(time, corrTrig[i]);
                hMCP2D->Fill(time, corrMCP[i]);
            }
        }

        // ToT analysis
        if (doToT && isSignal) {
            hToT->Fill(amp, processor.GetToT(corrMCP, analyzer.fMcpWindowMin, analyzer.fMcpWindowMax));
        }
        
        // Timing analysis
        if (doTiming && isSignal) {
            float triggerTime = analyzer.GetCFDTime(corrTrig, 0, eventNum, analyzer.fTriggerWindowMin, analyzer.fTriggerWindowMax, analyzer.fTriggerCfdFraction, analyzer.fTriggerCfdDelay, true, true, "CFD_Trig");
            float mcpTime = analyzer.GetCFDTime(corrMCP, channelNumber, eventNum, analyzer.fMcpWindowMin, analyzer.fMcpWindowMax, analyzer.fMcpCfdFraction, analyzer.fMcpCfdDelay, false, true, "CFD_MCP");

            hTrigTiming->Fill(triggerTime);
            hMCPTiming->Fill(mcpTime);
            hDiffTiming->Fill(mcpTime - triggerTime);
        }

        // Amplitude analysis
        if (doAmplitude && isSignal) {
            hAmp->Fill(amp);
        }
        
        // Npe analysis
        if (doNpe && isSignal) {
            float npe = analyzer.GetNpe(corrMCP, analyzer.fMcpWindowMin, analyzer.fMcpWindowMax);
            if (npe > threshold) {
                hNpe->Fill(npe);
            }
        }
    }
    
    // Save summary histograms
    if (doWaveform2D) {
        dataIO.Save(hTrig2D);
        dataIO.Save(hMCP2D);
    }

    if (doToT) {
        dataIO.Save(hToT);
    }

    if (doTiming) {
        dataIO.Save(hTrigTiming);
        dataIO.Save(hMCPTiming);
        dataIO.Save(hDiffTiming);

        std::cout << "Timing analysis completed" << std::endl;
    }
    
    if (doAmplitude) {
        dataIO.Save(hAmp);

        std::cout << "Amplitude analysis completed" << std::endl;
    }
    
    if (doNpe) {
        dataIO.Save(hNpe);

        std::cout << "Npe analysis completed" << std::endl;
    }
    
    dataIO.Close();
    
    std::cout << "=== Analysis for Run " << runNumber << " completed ===" << std::endl;
    std::cout << "Results saved to: " << outputFileName << std::endl;
}


int main(int argc, char** argv) {
    // Default values
    int runNumber = 101;
    int channelNumber = 10;
    int maxEvents = -1;
    std::string configFile = DEFAULT_CONFIG_FILE;
    bool processAll = true;
    bool doWaveform = false;
    bool doWaveform2D = false;
    bool doToT = false;
    bool doTiming = false;
    bool doAmplitude = false;
    bool doNpe = false;
    if (argc > 1) runNumber = atoi(argv[1]);
    if (argc > 2) channelNumber = atoi(argv[2]);
    if (argc > 3) maxEvents = atoi(argv[3]);
    if (argc > 4) configFile = argv[4];
    if (argc > 5) {
        std::string mode = argv[5];
        if (mode == "all") {
            processAll = true;
        } else {
            processAll = false;
            doWaveform = (mode.find('w') != std::string::npos);
            doWaveform2D = (mode.find('2') != std::string::npos);
            doToT = (mode.find('t') != std::string::npos);
            doTiming = (mode.find('t') != std::string::npos);
            doAmplitude = (mode.find('a') != std::string::npos);
            doNpe = (mode.find('n') != std::string::npos);
        }
    }
    
    analyzer(runNumber, channelNumber, maxEvents, configFile, processAll, doWaveform, doWaveform2D, doToT, doTiming, doAmplitude, doNpe);
    
    return 0;
} 