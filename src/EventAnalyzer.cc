#include "../include/EventAnalyzer.h"
#include "../include/Config.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <TH1F.h>
#include <TDirectory.h>
#include <TSpline.h>
#include <TH1D.h>
#include <TGraph.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLine.h>
#include <TMarker.h>
#include <TLegend.h>
#include <TROOT.h>

namespace HRPPD {

EventAnalyzer::EventAnalyzer() 
    : m_triggerCfdFraction(0.5),
      m_triggerCfdDelay(5),
      m_mcpCfdFraction(0.5),
      m_mcpCfdDelay(5),
      m_triggerWindowMin(500),
      m_triggerWindowMax(600),
      m_mcpWindowMin(500),
      m_mcpWindowMax(600),
      m_fftCutoffFrequency(0.7e9),
      m_applyFFTFilter(false) {
}

EventAnalyzer::~EventAnalyzer() {
}

bool EventAnalyzer::Initialize() {
    return true;
}

// Functions moved from WaveformProcessor

float EventAnalyzer::GetAmplitude(const std::vector<float>& waveform, int windowMin, int windowMax) {
    float amp = *std::min_element(waveform.begin() + windowMin, waveform.begin() + windowMax);
    return (abs(amp));
}

float EventAnalyzer::GetNpe(const std::vector<float>& waveform, int windowMin, int windowMax) {
    auto peakIter = std::min_element(waveform.begin() + windowMin, waveform.begin() + windowMax);
    int peakIdx = std::distance(waveform.begin(), peakIter);

    float integral = 0.;
    for (int i = peakIdx - 5; i < peakIdx + 5; i++) {
        if (i >= 0 && i < waveform.size() && waveform[i] < 0) {
            integral += waveform[i];
        }
    }

    float deltaT = CONFIG_DELTA_T;
    float qfast = -1. * (integral * deltaT / 50); // Q = I * t = V/R * t [fC]
    float gain = (qfast * 1e-15) / 1.6e-19; // Number of electrons
    
    return gain;
}

// Timing analysis functions moved from WaveformProcessor

TSpline3* EventAnalyzer::CreateCFDSpline(TH1D* hcfd, int binLow, int binHigh, const char* name) {
    if (binLow > binHigh) {
        int temp = binLow;
        binLow = binHigh;
        binHigh = temp;
    }
    
    int nPoints = binHigh - binLow + 1;
    
    double* xArr = new double[nPoints];
    double* yArr = new double[nPoints];
    
    for (int i = 0; i < nPoints; i++) {
        int bin = binLow + i;
        xArr[i] = hcfd->GetBinCenter(bin);
        yArr[i] = hcfd->GetBinContent(bin);
    }
    
    TSpline3* spline = new TSpline3(name, xArr, yArr, nPoints);
    
    delete[] xArr;
    delete[] yArr;
    
    return spline;
}

void EventAnalyzer::VisualizeSpline(TH1D* hcfd, int binLow, int binHigh, TSpline3* spline, 
                                  TGraph* pointGraph, TGraph* splineGraph) {
    if (binLow > binHigh) {
        int temp = binLow;
        binLow = binHigh;
        binHigh = temp;
    }
    
    // Data points
    for (int bin = binLow; bin <= binHigh; bin++) {
        double x = hcfd->GetBinCenter(bin);
        double y = hcfd->GetBinContent(bin);
        pointGraph->SetPoint(pointGraph->GetN(), x, y);
    }
    
    // Spline curve
    double x_min = hcfd->GetBinCenter(binLow);
    double x_max = hcfd->GetBinCenter(binHigh);
    
    // For smooth curve, use more points
    int nSteps = 100;
    for (int i = 0; i <= nSteps; i++) {
        double x = x_min + (x_max - x_min) * i / nSteps;
        double y = spline->Eval(x);
        splineGraph->SetPoint(i, x, y);
    }
}

float EventAnalyzer::CFDDiscriminator(const std::vector<float>& waveform, int channel, int eventNum, 
                                    float fitWindowMin, float fitWindowMax, 
                                    float fractionCFD, int delayCFD, 
                                    bool isPositive, bool isVisualize, 
                                    std::string dirName) {
    const int dimSize = waveform.size(); // 1024 bins
    float deltaT = m_processor.m_deltaT; // Access directly from WaveformProcessor
    
    float time = 0;
    char CFDSplineName[100];
    char CFDSplineTitle[100];

    sprintf(CFDSplineName, "CFDSpline_ch%d_evt%d", channel, eventNum);
    sprintf(CFDSplineTitle, "CFDSpline | CFDSpline_ch%d_evt%d", channel, eventNum);
    
    // Create histograms from input waveform
    TH1D *hwav = new TH1D(Form("hwav_ch%d_evt%d", channel, eventNum), "Waveform", dimSize, 0, dimSize * deltaT);
    TH1D *hcfd = new TH1D(Form("hcfd_ch%d_evt%d", channel, eventNum), "CFD Signal", dimSize, 0, dimSize * deltaT);
    TH1D *hinv = new TH1D(Form("hinv_ch%d_evt%d", channel, eventNum), "Inverted", dimSize, 0, dimSize * deltaT);
    
    // Fill the waveform histogram
    for (int i = 0; i < dimSize; i++) {
        hwav->SetBinContent(i+1, waveform[i]);
    }
    
    // Create CFD signal
    // First, copy original waveform
    for (int i = 0; i < dimSize; i++) {
        hcfd->SetBinContent(i+1, waveform[i]);
    }

    // Reverse and attenuate
    for (int i = 0; i < dimSize; i++) {
        hinv->SetBinContent(i+1, -1. * fractionCFD * waveform[i]); // reverse and attenuate
        if (i < delayCFD) {
            hcfd->SetBinContent(i+1, 0);
        } else {
            hcfd->SetBinContent(i+1, waveform[i-delayCFD]); // delay
        }
    }
    hcfd->Add(hinv);

    hcfd->GetXaxis()->SetRange(fitWindowMin, fitWindowMax);

    int binLow = 0;
    int binHigh = 0;

    if (isPositive) {
        binLow = hcfd->GetMinimumBin();
        binHigh = hcfd->GetMaximumBin();
    } else {
        binLow = hcfd->GetMaximumBin();
        binHigh = hcfd->GetMinimumBin();
    }
    
    int searchEnd = -1;
    
    // Find zero crossing by moving left from minimum value
    if (isPositive) { 
        for (int i = binLow; i < dimSize; i++) { 
            if (hcfd->GetBinContent(i) <= 0) {
                searchEnd = i;
                break;
            }
            if (i <= binHigh) {
                searchEnd = binHigh;
                break;
            }
        }
    } else { 
        for (int i = binHigh; i > 0; i--) {
            if (hcfd->GetBinContent(i) >= 0) {
                searchEnd = i;
                break;
            }
            // If maximum value (binLow) is reached, exit
            if (i <= binLow) {
                searchEnd = binLow;
                break;
            }
        }
    }
    
    if (searchEnd < 0) {
        searchEnd = TMath::Max(1, binHigh - 10);
    }
    
    // Set fitting range - left rising edge section of minimum value
    int fitBinLow = searchEnd; // left boundary (0 crossing near or maximum value)
    int fitBinHigh = 0;   // right boundary (minimum value)
    
    if (isPositive) {
        fitBinHigh = fitBinLow + 10;
    } else {
        fitBinHigh = binHigh;  
    }

    // Adjust the range to be at least 5 bins
    if (fitBinHigh - fitBinLow < 5) {
        int needed = 5 - (fitBinHigh - fitBinLow);
        fitBinLow = TMath::Max(1, fitBinLow - needed);
    }
    
    TSpline3 *fsplinecfd = CreateCFDSpline(hcfd, fitBinLow, fitBinHigh, CFDSplineName);

    hcfd->GetXaxis()->SetRange();
    
    // Search for zero crossing within fitted range
    int bin = fitBinLow; // starting point
    bool foundCrossing = false;
    
    // Search for zero crossing
    if (isPositive) {
        for (int i = fitBinLow; i < fitBinHigh; i++) {
            if (hcfd->GetBinContent(i) * hcfd->GetBinContent(i+1) <= 0) {
                if (hcfd->GetBinContent(i) <= 0 && hcfd->GetBinContent(i+1) > 0) {
                    bin = i;
                    foundCrossing = true;
                    break;
                }
            }
        }
    } else {
        for (int i = fitBinLow; i < fitBinHigh; i++) {
            if (hcfd->GetBinContent(i) * hcfd->GetBinContent(i+1) <= 0) {
                if (hcfd->GetBinContent(i) >= 0 && hcfd->GetBinContent(i+1) < 0) {
                    bin = i;
                    foundCrossing = true;
                    break;
                }
            }
        }
    }
    
    // If zero crossing is not found, exit function
    if (!foundCrossing) {
        time = 0;
        delete hwav;
        delete hcfd;
        delete hinv;
        delete fsplinecfd;
        return time;
    }
    
    // Search for exact zero crossing using binary search
    double eps = 1e-3;
    double xlow = hcfd->GetBinCenter(bin);
    double xhigh = hcfd->GetBinCenter(bin + 1) + eps;
    double xmid;
    int iterCount = 0;
    
    // Search for exact zero crossing using binary search
    while ((xhigh - xlow) >= eps && iterCount < 50) {
        xmid = (xlow + xhigh) / 2;
        
        if (fsplinecfd->Eval(xmid) == 0) {
            time = xmid;
            break;
        }
            
        if (fsplinecfd->Eval(xlow) * fsplinecfd->Eval(xmid) < 0) {  
            xhigh = xmid;
        } else {
            xlow = xmid;
        }
        iterCount++;
    }
    time = xlow;
    
    // Visualization code
    if (isVisualize && eventNum < 200) {
        TDirectory* current = gDirectory;

        TDirectory* dir = nullptr;
        if (gDirectory->Get(dirName.c_str())) {
            dir = static_cast<TDirectory*>(gDirectory->Get(dirName.c_str()));
        } else {
            dir = gDirectory->mkdir(dirName.c_str());
        }
        
        dir->cd();
        
        TCanvas* c = new TCanvas(Form("CFD_evt%d", eventNum), Form("CFD Analysis - Evt%d", eventNum), 900, 600);
        
        gStyle->SetOptStat(0);
        gStyle->SetOptTitle(1);
        
        hcfd->SetLineColor(kBlue);
        hcfd->SetTitle(Form("CFD Signal - Evt%d", eventNum));
        hcfd->GetXaxis()->SetTitle("Time [ps]");
        hcfd->GetYaxis()->SetTitle("Amplitude [mV]");
        
        double y_min = hcfd->GetMinimum();
        double y_max = hcfd->GetMaximum();
        double margin = 0.2 * (y_max - y_min);
        hcfd->GetYaxis()->SetRangeUser(y_min - margin, y_max + margin);
        
        hcfd->Draw();
        
        TGraph* fitted_points = new TGraph();
        for (int i = fitBinLow; i <= fitBinHigh; i++) {
            fitted_points->SetPoint(fitted_points->GetN(), hcfd->GetBinCenter(i), hcfd->GetBinContent(i));
        }
        
        TGraph* spline_curve = new TGraph();
        double x_min = hcfd->GetBinCenter(fitBinLow);
        double x_max = hcfd->GetBinCenter(fitBinHigh);
        int nSteps = 200;
        
        for (int i = 0; i <= nSteps; i++) {
            double x = x_min + (x_max - x_min) * i / nSteps;
            double y = fsplinecfd->Eval(x);
            spline_curve->SetPoint(i, x, y);
        }
        
        spline_curve->SetLineColor(kRed);
        spline_curve->SetLineWidth(2);
        spline_curve->Draw("L SAME");
        
        fitted_points->SetMarkerStyle(20);
        fitted_points->SetMarkerSize(0.4);
        fitted_points->SetMarkerColor(kRed);
        
        TLine* zero_line = new TLine(fitWindowMin * deltaT, 0, fitWindowMax * deltaT, 0);
        zero_line->SetLineStyle(2);
        zero_line->SetLineColor(kBlack);
        zero_line->Draw();
        
        TMarker* crossing = new TMarker(time, 0, 20);
        crossing->SetMarkerColor(kRed);
        crossing->SetMarkerSize(0.8);
        crossing->SetMarkerStyle(21);
        crossing->Draw();
        
        TLegend* leg = new TLegend(0.65, 0.75, 0.88, 0.88);
        leg->SetLineWidth(0);
        leg->AddEntry((TObject*)hcfd, "#font[42]{CFD Signal}", "l");
        leg->AddEntry((TObject*)spline_curve, "#font[42]{Spline Fit}", "l");
        leg->AddEntry((TObject*)crossing, Form("#font[42]{Zero Crossing: %.1f ps}", time), "p");
        leg->Draw();
        
        c->Write();
        
        delete fitted_points;
        delete spline_curve;
        delete zero_line;
        delete crossing;
        delete leg;
        delete c;
        
        current->cd();
    }
    
    delete hwav;
    delete hcfd;
    delete hinv;
    delete fsplinecfd;

    return time;
}

} // namespace HRPPD