#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

namespace HRPPD {

// Global configuration variables
extern std::string CONFIG_OUTPUT_PATH;
extern std::string CONFIG_RAWDATA_PATH;     
extern std::string CONFIG_NTUPLE_PATH;

extern float CONFIG_TRIGGER_CFD_FRACTION;
extern int CONFIG_TRIGGER_CFD_DELAY;
extern int CONFIG_TRIGGER_WINDOW_MIN;
extern int CONFIG_TRIGGER_WINDOW_MAX;

extern float CONFIG_MCP_CFD_FRACTION;
extern int CONFIG_MCP_CFD_DELAY;
extern int CONFIG_MCP_WINDOW_MIN;
extern int CONFIG_MCP_WINDOW_MAX;

extern float CONFIG_FFT_CUTOFF_FREQUENCY;
extern bool CONFIG_APPLY_FFT_FILTER;

extern float CONFIG_CALIBRATION_CONSTANT;  // ADC to mV conversion constant
extern float CONFIG_DELTA_T;               // Sampling interval (ps)
extern float CONFIG_SAMPLING_RATE;         // Sampling rate (Hz)

extern bool CONFIG_DO_WAVEFORM;
extern bool CONFIG_DO_WAVEFORM2D;
extern bool CONFIG_DO_TOT;
extern bool CONFIG_DO_TIMING;
extern bool CONFIG_DO_AMPLITUDE;
extern bool CONFIG_DO_NPE;

// Configuration file loading function
bool Load(const std::string& configFile);

} // namespace HRPPD

#endif // CONFIG_H 