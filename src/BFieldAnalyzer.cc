#include "../include/BFieldAnalyzer.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TPaveText.h"
#include <iostream>

namespace HRPPD {

BFieldAnalyzer::BFieldAnalyzer() : 
    inputPath_("./output/250509/"),
    outputPath_("./output/250509/bfield_study/") {
}

BFieldAnalyzer::~BFieldAnalyzer() {
    // Memory cleanup
    for (auto& fit : timeResFits_) if (fit) delete fit;
    for (auto& fit : ampFits_) if (fit) delete fit;
    for (auto& fit : npeFits_) if (fit) delete fit;
}

void BFieldAnalyzer::SetPaths(const std::string& inputPath, const std::string& outputPath) {
    inputPath_ = inputPath;
    outputPath_ = outputPath;
}

void BFieldAnalyzer::LoadData(const std::vector<int>& runNumbers, 
                           const std::vector<double>& bFieldValues,
                           const std::string& dataType) {
    if (runNumbers.size() != bFieldValues.size() || runNumbers.empty()) {
        std::cerr << "Error: Run numbers and B-field values must have the same size and not be empty." << std::endl;
        return;
    }
    
    // Initialize data
    bFieldValues_ = bFieldValues;
    timeResFits_.clear();
    ampFits_.clear();
    npeFits_.clear();
    
    // Reference run is the first run in the list
    int refRun = runNumbers[0];
    double refBField = bFieldValues[0];
    
    // Load timing data
    if (dataType == "all" || dataType == "timing") {
        for (size_t i = 0; i < runNumbers.size(); i++) {
            int run = runNumbers[i];
            
            // Open timing file
            std::string fileName = inputPath_ + "/run" + std::to_string(run) + "/Timing_Run_" + std::to_string(run) + ".root";
            TFile* file = TFile::Open(fileName.c_str(), "READ");
            
            if (!file || file->IsZombie()) {
                std::cerr << "Error: Could not open timing file for run " << run << std::endl;
                timeResFits_.push_back(nullptr);
                continue;
            }
            
            // Get timing resolution histogram
            TH1* hist = (TH1*)file->Get("Diff_Timing");
            if (!hist) {
                std::cerr << "Error: Could not find timing histogram for run " << run << std::endl;
                file->Close();
                timeResFits_.push_back(nullptr);
                continue;
            }
            
            // Fit with Gaussian
            TF1* fit = new TF1(Form("timeFit_%d", run), "gaus", 50000, 80000);
            hist->Fit(fit, "Q");
            
            // Save fit results
            timeResFits_.push_back(fit);
            file->Close();
        }
    }
    
    // Load amplitude data
    if (dataType == "all" || dataType == "amplitude") {
        for (size_t i = 0; i < runNumbers.size(); i++) {
            int run = runNumbers[i];
            
            // Open amplitude file
            std::string fileName = inputPath_ + "/run" + std::to_string(run) + "/Amplitude_Run_" + std::to_string(run) + ".root";
            TFile* file = TFile::Open(fileName.c_str(), "READ");
            
            if (!file || file->IsZombie()) {
                std::cerr << "Error: Could not open amplitude file for run " << run << std::endl;
                ampFits_.push_back(nullptr);
                continue;
            }
            
            // Get amplitude histogram
            TH1* hist = (TH1*)file->Get("MCP_Amplitude");
            if (!hist) {
                std::cerr << "Error: Could not find amplitude histogram for run " << run << std::endl;
                file->Close();
                ampFits_.push_back(nullptr);
                continue;
            }
            
            // Fit with Gaussian
            TF1* fit = new TF1(Form("ampFit_%d", run), "gaus", 0, 100);
            hist->Fit(fit, "Q");
            
            // Save fit results
            ampFits_.push_back(fit);
            file->Close();
        }
    }
    
    // Load Npe data
    if (dataType == "all" || dataType == "npe") {
        for (size_t i = 0; i < runNumbers.size(); i++) {
            int run = runNumbers[i];
            
            // Open Npe file
            std::string fileName = inputPath_ + "/run" + std::to_string(run) + "/Npe_Run_" + std::to_string(run) + ".root";
            TFile* file = TFile::Open(fileName.c_str(), "READ");
            
            if (!file || file->IsZombie()) {
                std::cerr << "Error: Could not open Npe file for run " << run << std::endl;
                npeFits_.push_back(nullptr);
                continue;
            }
            
            // Get Npe histogram
            TH1* hist = (TH1*)file->Get("Npe");
            if (!hist) {
                std::cerr << "Error: Could not find Npe histogram for run " << run << std::endl;
                file->Close();
                npeFits_.push_back(nullptr);
                continue;
            }
            
            // Fit with Gaussian
            double xmin = hist->GetMean() - 3 * hist->GetRMS();
            double xmax = hist->GetMean() + 3 * hist->GetRMS();
            
            TF1* fit = new TF1(Form("npeFit_%d", run), "gaus", xmin, xmax);
            hist->Fit(fit, "Q");
            
            // Save fit results
            npeFits_.push_back(fit);
            file->Close();
        }
    }
}

void BFieldAnalyzer::CreatePlots(const std::string& outputFileName, bool normalized) {
    // Create output file
    TFile* outputFile = TFile::Open(outputFileName.c_str(), "RECREATE");
    if (!outputFile || outputFile->IsZombie()) {
        std::cerr << "Error: Could not create output file " << outputFileName << std::endl;
        return;
    }
    
    // Timing plot
    if (!timeResFits_.empty()) {
        TCanvas* c1 = new TCanvas("c_timing", "Timing Resolution vs B-Field", 800, 600);
        
        // Create graph
        TGraphErrors* timeGraph = nullptr;
        if (normalized) {
            timeGraph = CreateNormalizedGraph(timeResFits_, bFieldValues_);
        } else {
            timeGraph = new TGraphErrors(bFieldValues_.size());
            
            for (size_t i = 0; i < bFieldValues_.size(); i++) {
                if (timeResFits_[i]) {
                    timeGraph->SetPoint(i, bFieldValues_[i], timeResFits_[i]->GetParameter(2));
                    timeGraph->SetPointError(i, 0, timeResFits_[i]->GetParError(2));
                }
            }
        }
        
        // Set graph style
        SetupPlotStyle(timeGraph, "Timing Resolution vs B-Field", 
                     "B-Field [T]", normalized ? "Normalized Resolution" : "Time Resolution [ps]");
        
        // Draw graph
        timeGraph->Draw("APL");
        c1->Write();
        delete timeGraph;
        delete c1;
    }
    
    // Amplitude plot
    if (!ampFits_.empty()) {
        TCanvas* c2 = new TCanvas("c_amplitude", "MCP Amplitude vs B-Field", 800, 600);
        
        // Create graph
        TGraphErrors* ampGraph = nullptr;
        if (normalized) {
            ampGraph = CreateNormalizedGraph(ampFits_, bFieldValues_);
        } else {
            ampGraph = new TGraphErrors(bFieldValues_.size());
            
            for (size_t i = 0; i < bFieldValues_.size(); i++) {
                if (ampFits_[i]) {
                    ampGraph->SetPoint(i, bFieldValues_[i], ampFits_[i]->GetParameter(1));
                    ampGraph->SetPointError(i, 0, ampFits_[i]->GetParError(1));
                }
            }
        }
        
        // Set graph style
        SetupPlotStyle(ampGraph, "MCP Amplitude vs B-Field", 
                     "B-Field [T]", normalized ? "Normalized Amplitude" : "Amplitude [mV]");
        
        // Draw graph
        ampGraph->Draw("APL");
        c2->Write();
        delete ampGraph;
        delete c2;
    }
    
    // Npe plot
    if (!npeFits_.empty()) {
        TCanvas* c3 = new TCanvas("c_npe", "Number of Photoelectrons vs B-Field", 800, 600);
        
        // Create graph
        TGraphErrors* npeGraph = nullptr;
        if (normalized) {
            npeGraph = CreateNormalizedGraph(npeFits_, bFieldValues_);
        } else {
            npeGraph = new TGraphErrors(bFieldValues_.size());
            
            for (size_t i = 0; i < bFieldValues_.size(); i++) {
                if (npeFits_[i]) {
                    npeGraph->SetPoint(i, bFieldValues_[i], npeFits_[i]->GetParameter(1));
                    npeGraph->SetPointError(i, 0, npeFits_[i]->GetParError(1));
                }
            }
        }
        
        // Set graph style
        SetupPlotStyle(npeGraph, "Number of Photoelectrons vs B-Field", 
                     "B-Field [T]", normalized ? "Normalized Npe" : "Npe");
        
        // Draw graph
        npeGraph->Draw("APL");
        c3->Write();
        delete npeGraph;
        delete c3;
    }
    
    outputFile->Close();
    delete outputFile;
    std::cout << "B-Field analysis plots saved to: " << outputFileName << std::endl;
}

TGraphErrors* BFieldAnalyzer::CreateNormalizedGraph(const std::vector<TF1*>& fits, 
                                                  const std::vector<double>& xValues) {
    TGraphErrors* graph = new TGraphErrors(xValues.size());
    
    // Get reference value (B=0)
    double refValue = 0;
    bool refFound = false;
    
    for (size_t i = 0; i < xValues.size(); i++) {
        if (fits[i] && xValues[i] == 0.0) {
            refValue = fits[i]->GetParameter(1);
            refFound = true;
            break;
        }
    }
    
    // If no B=0 found, use first value as reference
    if (!refFound && !fits.empty() && fits[0]) {
        refValue = fits[0]->GetParameter(1);
    }
    
    // Create normalized graph
    for (size_t i = 0; i < xValues.size(); i++) {
        if (fits[i]) {
            double value = fits[i]->GetParameter(1);
            double error = fits[i]->GetParError(1);
            
            graph->SetPoint(i, xValues[i], value / refValue);
            graph->SetPointError(i, 0, error / refValue);
        }
    }
    
    return graph;
}

void BFieldAnalyzer::SetupPlotStyle(TGraphErrors* graph, const std::string& title, 
                                  const std::string& xTitle, const std::string& yTitle) {
    if (!graph) return;
    
    graph->SetTitle(title.c_str());
    graph->GetXaxis()->SetTitle(xTitle.c_str());
    graph->GetYaxis()->SetTitle(yTitle.c_str());
    
    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(1.2);
    graph->SetMarkerColor(kBlue);
    graph->SetLineColor(kBlue);
    graph->SetLineWidth(2);
    
    // Fit with polynomial
    TF1* fitFunc = new TF1("fitFunc", "pol2", 0, 1.0);
    graph->Fit(fitFunc, "Q");
    fitFunc->SetLineColor(kRed);
    fitFunc->SetLineWidth(2);
}

} // namespace HRPPD 