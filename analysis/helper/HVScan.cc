#include <stdio.h>
#include <TFile.h>
#include <TH1F.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TColor.h>
#include <vector>
#include <map>

struct Data {
    int run_number;
    int HV;
    double B_field;
    double amplitude;
    double gain;
    double entries;
};

Data getData(int run_number, int HV, double B_field) {
    TFile *file = new TFile(Form("../../output/250630_HVScan/run%d/Analysis_Run_%d.root", run_number, run_number), "READ");
    
    if (!file || !file->IsOpen()) {
        std::cerr << "Error opening file for run " << run_number << std::endl;
    }
    
    TH1F* hist = (TH1F*)file->Get("Amplitude");
    TH1F* hist2 = (TH1F*)file->Get("Npe");
    
    Data data;
    data.run_number = run_number;
    data.HV = HV;
    data.B_field = B_field;
    data.amplitude = hist->GetMean();
    data.gain = hist2->GetMean();
    data.entries = hist->GetEntries();
    
    file->Close();
    delete file;
    
    return data;
}

void HVScan() {

    std::vector<int> colors;
    // colors.push_back(TColor::GetColor("#e76300"));
    colors.push_back(TColor::GetColor("#009900"));
    colors.push_back(TColor::GetColor("#1845fb")); 
    colors.push_back(TColor::GetColor("#C91f16")); 
    colors.push_back(TColor::GetColor("#000000"));
    colors.push_back(TColor::GetColor("#717581"));
    colors.push_back(TColor::GetColor("#c849a9"));

    std::vector<int> markers = {21, 20, 22, 23, 33, 47};
    std::vector<double> markerSize = {1.2, 1.3, 1.4, 1.4, 1.8, 1.4};
    std::vector<Data> points;

    // points.push_back(getData(122, 975, 2.0));
    points.push_back(getData(123, 1000, 2.0));
    points.push_back(getData(124, 1025, 2.0));
    points.push_back(getData(125, 1050, 2.0));
    points.push_back(getData(126, 1075, 2.0));
    points.push_back(getData(127, 1100, 2.0));
    points.push_back(getData(128, 1125, 2.0));

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

    std::map<int, std::vector<std::pair<double, double>>> hvMap_amp;
    std::map<int, std::vector<std::pair<double, double>>> hvMap_gain;
    for (const auto& point : points) {
        hvMap_amp[point.HV].push_back({point.B_field, point.amplitude});
        hvMap_gain[point.HV].push_back({point.B_field, point.gain});
    }

    TCanvas *c1 = new TCanvas("", "", 800, 600);
    TMultiGraph *mg_amp = new TMultiGraph();
    TMultiGraph *mg_gain = new TMultiGraph();
    TLegend *legend_amp = new TLegend(0.743, 0.557, 0.9, 0.9);
    TLegend *legend_gain = new TLegend(0.743, 0.557, 0.9, 0.9);

    gStyle->SetOptStat(0);
    gStyle->SetTitleFont(22,"xyz"); 
    gStyle->SetLabelFont(22,"xyz");
    gStyle->SetTitleOffset(1.4,"xy");

    int color = 0;
    std::vector<std::pair<TGraph*, int>> graphsAndHV_amp;
    std::vector<std::pair<TGraph*, int>> graphsAndHV_gain;
    
    // Fill amp
    for (const auto& [hv, points] : hvMap_amp) {
        TGraph *g = new TGraph(points.size());
        for (size_t i = 0; i < points.size(); i++) {
            g->SetPoint(i, points[i].first, points[i].second);
        }
        
        g->SetMarkerStyle(markers[color]);
        g->SetMarkerSize(markerSize[color]);
        g->SetMarkerColor(colors[color]);
        g->SetLineColor(colors[color]);
        g->SetLineWidth(2);
        
        mg_amp->Add(g);
        graphsAndHV_amp.push_back({g, hv});
        color++;
    }

    // Fill gain
    color = 0;
    for (const auto& [hv, points] : hvMap_gain) {
        TGraph *g = new TGraph(points.size());
        for (size_t i = 0; i < points.size(); i++) {
            g->SetPoint(i, points[i].first, points[i].second);
        }

        g->SetMarkerStyle(markers[color]);
        g->SetMarkerSize(markerSize[color]);
        g->SetMarkerColor(colors[color]);
        g->SetLineColor(colors[color]);
        g->SetLineWidth(2);

        mg_gain->Add(g);
        graphsAndHV_gain.push_back({g, hv});
        color++;
    }

    // Add legend entries in reverse order
    // legend->SetLineWidth(0);
    legend_amp->SetTextSize(0.03); 
    for (int i = graphsAndHV_amp.size() - 1; i >= 0; i--) {
        legend_amp->AddEntry(graphsAndHV_amp[i].first, Form("#font[22]{  %.0d V}", graphsAndHV_amp[i].second), "pl");
    }

    legend_gain->SetTextSize(0.03); 
    for (int i = graphsAndHV_gain.size() - 1; i >= 0; i--) {
        legend_gain->AddEntry(graphsAndHV_gain[i].first, Form("#font[22]{  %.0d V}", graphsAndHV_gain[i].second), "pl");
    }

    c1->SetGrid();
    // c1->SetLogy();
    
    mg_amp->SetTitle("");
    mg_amp->GetXaxis()->SetTitle("Magnetic Field [T]");
    mg_amp->GetYaxis()->SetTitle("Amplitude [mV]");
    mg_amp->GetXaxis()->CenterTitle();
    mg_amp->GetYaxis()->CenterTitle();
    // mg_amp->GetYaxis()->SetRangeUser(4., 18.);
    mg_amp->GetYaxis()->SetRangeUser(5., 30.);
    mg_amp->Draw("ALP");
    legend_amp->Draw();
    // c1->SaveAs("../../output/250630_HVScan/HVScan_amp_log.pdf");

    c1->Clear();
    c1->SetGrid();

    mg_gain->SetTitle("");
    mg_gain->GetXaxis()->SetTitle("Magnetic Field [T]");
    mg_gain->GetYaxis()->SetTitle("Gain");
    mg_gain->GetXaxis()->CenterTitle();
    mg_gain->GetYaxis()->CenterTitle();
    mg_gain->GetYaxis()->SetRangeUser(400000., 2500000.);
    mg_gain->Draw("ALP");
    legend_gain->Draw();
    c1->SaveAs("../../output/250630_HVScan/HVScan_gain.pdf");

}