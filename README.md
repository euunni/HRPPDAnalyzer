# HRPPDAnalyzer

A modular analysis package for processing High Resolution Pixel Photon Detector (HRPPD) data.

## Build Instructions

```bash
# Set ROOT environment.
# This shell script is executable in KNU server, otherwise you should install ROOT on your machine.
source envset.sh

# Create build and install directories
mkdir -p build install

# Build the package
cd build
cmake ..
make -j4 install

# Run the analyzer
cd ../install
./bin/analyzer [runNumber] [channel] [maxEvents] [configFile] [analysisType]
```

### Arguments:
- `runNumber`: Run number to analyze (default: 101)
- `channel`: Channel number to analyze (default: 10)
- `maxEvents`: Maximum number of events to process (-1 for all events)
- `configFile`: Path to configuration file (default: ../config/config.txt)
- `analysisType`: Type of analysis to perform
  - `all`: All analysis types (default)
  - `w`: Waveform analysis
  - `2`: 2D waveform analysis
  - `t`: Timing/ToT analysis
  - `a`: Amplitude analysis
  - `n`: Npe analysis
  - You can combine these flags: e.g. `wta` for waveform, timing, and amplitude analysis

### Example:
```bash
./bin/analyzer 2000 10 1000 ../config/config.txt all
``` 