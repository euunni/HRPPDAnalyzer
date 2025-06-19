#include "../include/Config.h"

#include <iostream>
#include <limits>


namespace HRPPD {

// Initialize configuration variables (default values)
std::string CONFIG_OUTPUT_PATH = "../output";
std::string CONFIG_RAWDATA_PATH = "/u/user/haeun/SE_UserHome/ANL/MCP_Data/Feb2023/HRPPD6";
std::string CONFIG_NTUPLE_PATH = "../data";
float CONFIG_TRIGGER_CFD_FRACTION = 0.5f;
int CONFIG_TRIGGER_CFD_DELAY = 3;
int CONFIG_TRIGGER_WINDOW_MIN = 200;
int CONFIG_TRIGGER_WINDOW_MAX = 300;
float CONFIG_MCP_CFD_FRACTION = 0.5f;
int CONFIG_MCP_CFD_DELAY = 3;
int CONFIG_MCP_WINDOW_MIN = 500;
int CONFIG_MCP_WINDOW_MAX = 600;
float CONFIG_FFT_CUTOFF_FREQUENCY = 0.7e9f;
bool CONFIG_APPLY_FFT_FILTER = false;
float CONFIG_CALIBRATION_CONSTANT = 0.48828125f;
float CONFIG_DELTA_T = 200.0f;
float CONFIG_SAMPLING_RATE = 5.0e9f;
bool CONFIG_DO_WAVEFORM = true;
bool CONFIG_DO_WAVEFORM2D = true;
bool CONFIG_DO_TOT = true;
bool CONFIG_DO_TIMING = true;
bool CONFIG_DO_AMPLITUDE = true;
bool CONFIG_DO_NPE = true;

// Configuration file loading function
bool Load(const std::string& configFile) {
    // Open file
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open configuration file: " << configFile << std::endl;
        return false;
    }
    
    // Read line by line
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Remove comments (even those in the middle of a line)
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Split the variable name and value using whitespace
        std::istringstream iss(line);
        std::string key, value;
        
        if (iss >> key >> value) {
            // Path settings
            if (key == "output_path") {
                CONFIG_OUTPUT_PATH = value;
            }
            else if (key == "rawdata_path") {
                CONFIG_RAWDATA_PATH = value;
            }
            else if (key == "ntuple_path") {
                CONFIG_NTUPLE_PATH = value;
            }
            // Trigger CFD settings
            else if (key == "trigger_cfd_fraction") {
                try { 
                    CONFIG_TRIGGER_CFD_FRACTION = std::stof(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert trigger_cfd_fraction" << std::endl; }
            }
            else if (key == "trigger_cfd_delay") {
                try { 
                    CONFIG_TRIGGER_CFD_DELAY = std::stoi(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert trigger_cfd_delay" << std::endl; }
            }
            else if (key == "trigger_window_min") {
                try { 
                    CONFIG_TRIGGER_WINDOW_MIN = std::stoi(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert trigger_window_min" << std::endl; }
            }
            else if (key == "trigger_window_max") {
                try { 
                    CONFIG_TRIGGER_WINDOW_MAX = std::stoi(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert trigger_window_max" << std::endl; }
            }
            // MCP CFD settings
            else if (key == "mcp_cfd_fraction") {
                try { 
                    CONFIG_MCP_CFD_FRACTION = std::stof(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert mcp_cfd_fraction" << std::endl; }
            }
            else if (key == "mcp_cfd_delay") {
                try { 
                    CONFIG_MCP_CFD_DELAY = std::stoi(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert mcp_cfd_delay" << std::endl; }
            }
            else if (key == "mcp_window_min") {
                try { 
                    CONFIG_MCP_WINDOW_MIN = std::stoi(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert mcp_window_min" << std::endl; }
            }
            else if (key == "mcp_window_max") {
                try { 
                    CONFIG_MCP_WINDOW_MAX = std::stoi(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert mcp_window_max" << std::endl; }
            }
            // FFT settings
            else if (key == "fft_cutoff_frequency") {
                try { 
                    CONFIG_FFT_CUTOFF_FREQUENCY = std::stof(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert fft_cutoff_frequency" << std::endl; }
            }
            else if (key == "apply_fft_filter") {
                CONFIG_APPLY_FFT_FILTER = (value == "true");
            }
            // Waveform processor settings
            else if (key == "calibration_constant") {
                try { 
                    CONFIG_CALIBRATION_CONSTANT = std::stof(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert calibration_constant" << std::endl; }
            }
            else if (key == "delta_t") {
                try { 
                    CONFIG_DELTA_T = std::stof(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert delta_t" << std::endl; }
            }
            else if (key == "sampling_rate") {
                try { 
                    CONFIG_SAMPLING_RATE = std::stof(value); 
                }
                catch (...) { std::cerr << "Warning: Failed to convert sampling_rate" << std::endl; }
            }
            // Analysis type settings
            else if (key == "do_waveform") {
                CONFIG_DO_WAVEFORM = (value == "true");
            }
            else if (key == "do_waveform2D") {
                CONFIG_DO_WAVEFORM2D = (value == "true");
            }
            else if (key == "do_tot") {
                CONFIG_DO_TOT = (value == "true");
            }
            else if (key == "do_timing") {
                CONFIG_DO_TIMING = (value == "true");
            }
            else if (key == "do_amplitude") {
                CONFIG_DO_AMPLITUDE = (value == "true");
            }
            else if (key == "do_npe") {
                CONFIG_DO_NPE = (value == "true");
            }
            else {
                std::cerr << "Warning: Unknown configuration key: " << key << std::endl;
            }
        }
    }
    
    file.close();
    return true;
}

} // namespace HRPPD 