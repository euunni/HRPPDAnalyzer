#include <stdio.h>
#include <TFile.h>
#include <TH1F.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TColor.h>
#include <TDirectory.h>
#include <TKey.h>
#include <TLine.h>
#include <vector>
#include <map>
#include <cmath>

struct Data {
    int run_number;
    int HV;
    double B_field;
    double afterpulse_ratio;
    double entries;
};

float GetStdDev(TH1F* hist) {
    // Calculate standard deviation from first 128 bins
    double sum = 0.0;
    for (int i = 1; i <= 128; i++) {
        sum += hist->GetBinContent(i);
    }
    double mean = sum / 128.0;
    
    double sumSquaredDiff = 0.0;
    for (int i = 1; i <= 128; i++) {
        double diff = hist->GetBinContent(i) - mean;
        sumSquaredDiff += diff * diff;
    }
    return std::sqrt(sumSquaredDiff / 128.0);
}

bool ToTCut(TH1F* waveHist, int fitWindowMin, int fitWindowMax) {
    float threshold = -4.0 * GetStdDev(waveHist);
    int count = 0;
    int consecutive = 0;

    for (int i = fitWindowMin; i < fitWindowMax; i++) {
        if (waveHist->GetBinContent(i) < threshold) {
            count++;
            consecutive = std::max(consecutive, count);
        } else {
            count = 0;
        }
    }
    float tot = consecutive * 200.;  
    
    return tot > 800.;
}

Data getData(int run_number, int HV, double B_field) {
    TFile *file = new TFile(Form("../../output/250630_HVScan/run%d/Analysis_Run_%d.root", run_number, run_number), "READ");
    
    if (!file || !file->IsOpen()) {
        std::cerr << "Error opening file for run " << run_number << std::endl;
        Data empty_data = {run_number, HV, B_field, 0.0, 0.0};
        return empty_data;
    }
    
    TDirectory* mcpDir = (TDirectory*)file->Get("Waveforms_MCP");
    if (!mcpDir) {
        std::cerr << "Waveforms_MCP directory not found for run " << run_number << std::endl;
        file->Close();
        delete file;
        Data empty_data = {run_number, HV, B_field, 0.0, 0.0};
        return empty_data;
    }
    
    TList* keyList = mcpDir->GetListOfKeys();
    int totalHistograms = 0;
    int afterpulseCount = 0;
    
    TIter iter(keyList);
    TKey* key;
    
    while ((key = (TKey*)iter())) {
        TString histName = key->GetName();
        
        if (histName.Contains("MCP_Wave_Evt") && histName.Contains("_Ch10")) {
            TH1F* waveHist = (TH1F*)mcpDir->Get(histName);
            if (!waveHist) continue;
            
            totalHistograms++;
            
            double threshold = 4.0 * GetStdDev(waveHist);
            bool isToT = ToTCut(waveHist, 680, 1000);
            bool hasAfterpulse = false;

            for (int i = 680; i < 1000; i++) {
                double amplitude = std::abs(waveHist->GetBinContent(i));
                if (amplitude > threshold && isToT) {
                    hasAfterpulse = true;
                    break;
                }
            }
            
            if (hasAfterpulse) {
                afterpulseCount++;
            }
        }
    }
    
    double ratio = (double)afterpulseCount / (double)totalHistograms * 100;
    
    file->Close();
    delete file;
    
    Data data;
    data.run_number = run_number;
    data.HV = HV;
    data.B_field = B_field;
    data.afterpulse_ratio = ratio;
    data.entries = totalHistograms;
    
    return data;
}

void afterPulse() {

    std::vector<int> colors;
    // colors.push_back(TColor::GetColor("#e76300"));
    colors.push_back(TColor::GetColor("#009900"));
    colors.push_back(TColor::GetColor("#1845fb")); 
    colors.push_back(TColor::GetColor("#C91f16")); 
    colors.push_back(TColor::GetColor("#000000"));
    colors.push_back(TColor::GetColor("#717581"));
    // colors.push_back(TColor::GetColor("#c849a9"));

    std::vector<int> markers = {21, 20, 22, 23, 33, 47};
    std::vector<double> markerSize = {1.2, 1.3, 1.4, 1.4, 1.8, 1.4};
    std::vector<Data> points;

    // points.push_back(getData(122, 975, 2.0));
    points.push_back(getData(123, 1000, 2.0));
    points.push_back(getData(124, 1025, 2.0));
    points.push_back(getData(125, 1050, 2.0));
    points.push_back(getData(126, 1075, 2.0));
    points.push_back(getData(127, 1100, 2.0));
    // points.push_back(getData(128, 1125, 2.0));

    points.push_back(getData(129, 1000, 1.8));
    points.push_back(getData(130, 1025, 1.8));
    points.push_back(getData(131, 1050, 1.8));
    points.push_back(getData(132, 1075, 1.8));
    points.push_back(getData(133, 1100, 1.8));

    points.push_back(getData(134, 1000, 1.6));
    points.push_back(getData(135, 1025, 1.6));
    points.push_back(getData(136, 1050, 1.6));
    points.push_back(getData(137, 1075, 1.6));

    points.push_back(getData(138, 1000, 1.4));
    points.push_back(getData(139, 1025, 1.4));
    points.push_back(getData(140, 1050, 1.4));
    // points.push_back(getData(147, 1075, 1.4));

    std::map<int, std::vector<std::pair<double, double>>> hvMap;
    for (const auto& point : points) {
        hvMap[point.HV].push_back({point.B_field, point.afterpulse_ratio});
    }

    TCanvas *c1 = new TCanvas("", "", 800, 600);
    TMultiGraph *mg = new TMultiGraph();
    TLegend *legend = new TLegend(0.738, 0.63, 0.9, 0.9);

    gStyle->SetOptStat(0);
    gStyle->SetTitleFont(22,"xyz"); 
    gStyle->SetLabelFont(22,"xyz");
    gStyle->SetTitleOffset(1.4,"xy");

    int color = 0;
    std::vector<std::pair<TGraph*, int>> graphsAndHV;  // Store graphs and HV values
    
    for (const auto& [hv, points] : hvMap) {
        TGraph *g = new TGraph(points.size());
        for (size_t i = 0; i < points.size(); i++) {
            g->SetPoint(i, points[i].first, points[i].second);
        }
        
        g->SetMarkerStyle(markers[color]);
        g->SetMarkerSize(markerSize[color]);
        g->SetMarkerColor(colors[color]);
        g->SetLineColor(colors[color]);
        g->SetLineWidth(2);
        
        mg->Add(g);
        graphsAndHV.push_back({g, hv});
        color++;
    }
    
    // Add legend entries in reverse order
    // legend->SetLineWidth(0);
    legend->SetTextSize(0.03);
    legend->SetTextAlign(22);  // Center alignment (horizontal=2, vertical=2)
    for (int i = graphsAndHV.size() - 1; i >= 0; i--) {
        legend->AddEntry(graphsAndHV[i].first, Form("#font[22]{  %.0d V}", graphsAndHV[i].second), "pl");
    }

    c1->SetGrid();
    
    mg->SetTitle("");
    mg->GetXaxis()->SetTitle("Magnetic Field [T]");
    mg->GetYaxis()->SetTitle("Afterpulse Fraction [%]");
    mg->GetXaxis()->CenterTitle();
    mg->GetYaxis()->CenterTitle();
        // First draw an empty frame to establish the coordinate system
    mg->GetYaxis()->SetRangeUser(0., 40.);
    mg->GetXaxis()->SetRangeUser(1.35, 2.05);
    mg->Draw("ALP");

    legend->Draw();
    c1->SaveAs("../../output/250630_HVScan/AfterPulse.pdf");
}
