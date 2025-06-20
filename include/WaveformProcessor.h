#ifndef WAVEFORM_PROCESSOR_H
#define WAVEFORM_PROCESSOR_H

#include <vector>
#include <string>
#include <TH1F.h>
#include <TDirectory.h>
#include <TSpline.h>
#include <TH1D.h>
#include <TGraph.h>


namespace HRPPD {

class WaveformProcessor {
public:
    WaveformProcessor();
    ~WaveformProcessor();
    
    // Waveform processing functions
    std::vector<float> Correct(const std::vector<float>& waveform);
    // float GetStdDev(const std::vector<float>& waveform, int start = 0, int end = -1);
    float GetStdDev(const std::vector<float>& waveform);
    std::vector<float> FFTFilter(const std::vector<float>& waveform, 
                               float cutoffFrequency, 
                               int eventNumber, int channelNumber);
    bool ToTCut(const std::vector<float>& waveform, int windowMin, int windowMax);
    
    // Internal utility functions
    float GetOverShoot(const std::vector<float>& waveform, int windowMin, int windowMax);
    float GetToT(const std::vector<float>& waveform, int windowMin, int windowMax);
    int GetToTBin(const std::vector<float>& waveform, int windowMin, int windowMax);
    float LowPassFilter(float cutoffFrequency, int order, float inputFreq);
    
    // Public member variables - directly accessible
    float fCalibrationConstant;  // Calibration constant
    float fDeltaT;               // Sampling interval (seconds)
    float fSamplingRate;         // Sampling rate (Hz)
};

} // namespace HRPPD

#endif // WAVEFORM_PROCESSOR_H