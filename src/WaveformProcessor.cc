#include "../include/WaveformProcessor.h"
#include "../include/Config.h"

#include <TH1F.h>
#include <TDirectory.h>
#include <TSpline.h>
#include <TH1D.h>
#include <TGraph.h>
#include <TMath.h>
#include <TVirtualFFT.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLine.h>
#include <TMarker.h>
#include <TLegend.h>
#include <TROOT.h>
#include <TObject.h>
#include <TString.h>

#include <numeric>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace HRPPD {

WaveformProcessor::WaveformProcessor() {
    m_calibrationConstant = CONFIG_CALIBRATION_CONSTANT;
    m_deltaT = CONFIG_DELTA_T;
    m_samplingRate = CONFIG_SAMPLING_RATE;
}

WaveformProcessor::~WaveformProcessor() {
}

std::vector<float> WaveformProcessor::CorrectWaveform(const std::vector<float>& waveform) {
    float ped = std::accumulate(waveform.begin(), waveform.begin() + 128, 0.) / 128;
    std::vector<float> corrWave;
    
    for (int i = 0; i < waveform.size(); i++) {
        corrWave.push_back((waveform[i] - ped) * m_calibrationConstant);
    }

    return corrWave;
}

float WaveformProcessor::GetStdDev(const std::vector<float>& waveform) {
  float mean = std::accumulate(waveform.begin(), waveform.begin() + 128, 0.) / 128;
  
  float sumSquaredDiff = 0.;
  for (int i = 0; i < 128; ++i) {
    float diff = waveform.at(i) - mean;
    sumSquaredDiff += diff * diff;
  }
  float ped = std::sqrt(sumSquaredDiff / 128);
    
  return ped;
}

float WaveformProcessor::GetOvershoot(const std::vector<float>& waveform, int fitWindowMin, int fitWindowMax) {
    // Direct implementation since GetAmplitude was removed
    float amp = *std::min_element(waveform.begin() + fitWindowMin, waveform.begin() + fitWindowMax);
    auto ampIter = std::min_element(waveform.begin() + fitWindowMin, waveform.begin() + fitWindowMax);
    int ampIdx = std::distance(waveform.begin(), ampIter);

    float overshoot = *std::max_element(waveform.begin() + ampIdx, waveform.begin() + (ampIdx + 10));
    
    return overshoot;
}

int WaveformProcessor::GetToTBin(const std::vector<float>& waveform, int fitWindowMin, int fitWindowMax) {
    int totBin = 0;
    float threshold = -4. * GetStdDev(waveform);

    for (int i = fitWindowMin; i < fitWindowMax; i++) {
        if (waveform.at(i) < threshold) {
            totBin++;
        }
    }
    
    return totBin;
}

float WaveformProcessor::GetToT(const std::vector<float>& waveform, int fitWindowMin, int fitWindowMax) {
    int totBin = GetToTBin(waveform, fitWindowMin, fitWindowMax);
    float tot = totBin * m_deltaT;
    
    return tot;
}

bool WaveformProcessor::ToTCut(const std::vector<float>& waveform, int fitWindowMin, int fitWindowMax) {
    return GetToT(waveform, fitWindowMin, fitWindowMax) > 800.;
}

float WaveformProcessor::LowPassFilter(float cutoffFrequency, int order, float inputFreq) {
    double f = 1.0/(1+TMath::Power(inputFreq/cutoffFrequency, 2*order));
    return f;
}

std::vector<float> WaveformProcessor::FFTFilter(const std::vector<float>& waveform, float cutoffFrequency,
                                           int eventNum, int channel) {
    int dimSize = 1000;
    double f;
    
    TH1F* waveOriginal = new TH1F("wave_original", "wave_original", dimSize, 0, 200.);
    for (int i = 0; i < dimSize && i < waveform.size(); i++) {
        waveOriginal->SetBinContent(i+1, waveform.at(i));
    }

    TString uniqueName = Form("evt%d_ch%d", eventNum, channel);

    // FFT real part
    TH1 *hr = 0;
    hr = waveOriginal->FFT(hr, "RE");
    hr->SetName(Form("hr_%s", uniqueName.Data()));
    
    // FFT imaginary part
    TH1 *him = 0;
    him = waveOriginal->FFT(him, "IM");
    him->SetName(Form("him_%s", uniqueName.Data()));

    // FFT magnitude
    TH1D *hmag = 0;
    hmag = (TH1D*)waveOriginal->FFT(hmag, "MAG");
    hmag->SetName(Form("hmag_%s", uniqueName.Data()));
    hmag->SetTitle("Magnitude of the 1st transform");
    hmag->GetXaxis()->Set(dimSize, 0, m_samplingRate);
    hmag->Scale(1./sqrt(dimSize)); //scale to 1/SQRT(n)

    TVirtualFFT *fftSignal = TVirtualFFT::GetCurrentTransform();
        
    //Frequency filtering
    std::vector<float> reFft;
    std::vector<float> imFft;
    std::vector<float> magFft;

    double *reFull = new double[dimSize];
    double *imFull = new double[dimSize];
    fftSignal->GetPointsComplex(reFull, imFull);

    for (int i = 0; i < dimSize; i++) {
        f = LowPassFilter(cutoffFrequency, 8, i * m_samplingRate / dimSize);

        if (i < (dimSize / 2)) {
            reFft.push_back(reFull[i]);
            imFft.push_back(imFull[i]);
            magFft.push_back(20 * log10(hmag->GetBinContent(i+1))); // Note: the power has already been scaled by sqrt(n). Y axis is translated to 20*log(mag_fft) dB
        }          

        // Spectrum filtering
        std::vector<float> magProcessed;

        reFull[i] = (reFull[i])*f;
        imFull[i] = (imFull[i])*f;
        if (i < (dimSize / 2)) {
            magProcessed.push_back(20*log10(sqrt(reFull[i]*reFull[i]+imFull[i]*imFull[i]) / sqrt(dimSize)));
        }
    }
        
    // Now let's make a backward transform:
    TVirtualFFT *fftBack = TVirtualFFT::FFT(1, &dimSize, "C2R M K");
    fftBack->SetPointsComplex(reFull,imFull);
    fftBack->Transform();
    TH1 *hb = 0;

    hb = TH1::TransformHisto(fftBack,hb,"Re");
    hb->SetTitle("The backward transform result");
    hb->Scale(1.0/dimSize);
    hb->GetXaxis()->Set(dimSize, 0, dimSize * m_deltaT); //ps

    std::vector<float> waveVecFiltered;
    for(int i = 0; i < dimSize; i++) {
        waveVecFiltered.push_back(hb->GetBinContent(i+1));
    }

    delete[] reFull;
    delete[] imFull;
    delete hb;
    delete hr;
    delete him;
    delete hmag;
    delete waveOriginal;
    fftBack = 0;

    return waveVecFiltered;
}

} // namespace HRPPD 