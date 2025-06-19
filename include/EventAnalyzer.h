#ifndef EVENT_ANALYZER_H
#define EVENT_ANALYZER_H

#include "WaveformProcessor.h"

#include <vector>
#include <string>
#include <TH1F.h>
#include <TDirectory.h>
#include <TSpline.h>
#include <TH1D.h>
#include <TGraph.h>


namespace HRPPD {

class EventAnalyzer {
public:
    EventAnalyzer();
    ~EventAnalyzer();
    
    bool Init();
    
    // Functions moved from WaveformProcessor
    float GetAmp(const std::vector<float>& waveform, int windowMin, int windowMax);
    float GetNpe(const std::vector<float>& waveform, int windowMin, int windowMax);
    
    // Timing analysis functions moved from WaveformProcessor
    TSpline3* CreateCFDSpline(TH1D* hcfd, int binLow, int binHigh, const char* name);
    void VisualizeSpline(TH1D* hcfd, int binLow, int binHigh, TSpline3* spline, 
                       TGraph* pointGraph, TGraph* splineGraph);
    float GetCFDTime(const std::vector<float>& waveform, int channel, int eventNum, 
                         float fitWindowMin, float fitWindowMax, 
                         float fractionCFD, int delayCFD, 
                         bool isPositive, bool isVisualize, 
                         std::string dirName);
    
    // CFD parameters
    float fTriggerCfdFraction;   // Trigger CFD fraction
    int fTriggerCfdDelay;        // Trigger CFD delay
    float fMcpCfdFraction;       // MCP CFD fraction
    int fMcpCfdDelay;            // MCP CFD delay
    
    // Window ranges
    int fTriggerWindowMin;       // Trigger window minimum
    int fTriggerWindowMax;       // Trigger window maximum
    int fMcpWindowMin;           // MCP window minimum
    int fMcpWindowMax;           // MCP window maximum
    
    // FFT parameters
    float fFftCutoffFrequency;   // FFT cutoff frequency
    bool fApplyFFTFilter;        // Apply FFT filter flag
    
    // Waveform processor
    WaveformProcessor fProcessor;
};

} // namespace HRPPD

#endif // EVENT_ANALYZER_H