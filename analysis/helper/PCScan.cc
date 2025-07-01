#include <stdio.h>
#include <TFile.h>
#include <TH1F.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <vector>

struct Data {
    double pcHV;
    double gain;
    double amplitude;
    double entries;
};

Data getData(int run_number, double pcHV) {
    TFile *file = new TFile(Form("../../output/250701_PCScan/run%d/Analysis_Run_%d.root", run_number, run_number), "READ");
    TH1F* hist = (TH1F*)file->Get("Npe");
    TH1F* hist2 = (TH1F*)file->Get("Amplitude");

    Data data;
    data.pcHV = pcHV;
    data.gain = hist->GetMean();
    data.amplitude = hist2->GetMean();
    data.entries = hist->GetEntries();

    file->Close();
    delete file;

    return data;
}

void PCScan() {
   
    std::vector<Data> points;

    points.push_back(getData(138, 100));
    points.push_back(getData(141, 150));
    points.push_back(getData(142, 200));
    points.push_back(getData(143, 250));
    points.push_back(getData(144, 300));
    points.push_back(getData(145, 350));
    points.push_back(getData(146, 400));

    TCanvas *c1 = new TCanvas("", "", 800, 600);
    TGraphErrors *gr_gain = new TGraphErrors();
    TGraphErrors *gr_amplitude = new TGraphErrors();

    int i = 0;
    for (const auto& point : points) {
        gr_gain->SetPoint(i, point.pcHV, point.gain);
        gr_amplitude->SetPoint(i, point.pcHV, point.amplitude);
        i++;
    }

    // c1->SetLogy();  
    c1->SetGrid();

    gStyle->SetTitleFont(22,"xyz"); 
    gStyle->SetLabelFont(22,"xyz");
    gStyle->SetTitleOffset(1.4,"xy");
    // gStyle->SetTitleOffset(1.,"y");

    gr_gain->SetLineColor(TColor::GetColor("#C91f16"));
    gr_gain->SetMarkerColor(TColor::GetColor("#C91f16"));
    gr_gain->SetMarkerStyle(21);
    gr_gain->SetMarkerSize(1);
    gr_gain->SetLineWidth(2);
    gr_gain->SetTitle("");
    gr_gain->GetYaxis()->SetRangeUser(900000., 3000000.);
    gr_gain->GetXaxis()->SetTitle("#Delta V (Photocathode - MCP) [V]");
    gr_gain->GetYaxis()->SetTitle("Gain");
    gr_gain->GetXaxis()->CenterTitle();
    gr_gain->GetYaxis()->CenterTitle();
    gr_gain->Draw("APL");
    // c1->SaveAs("../../output/250701_PCScan/PCScan.pdf");

    c1->Clear();
    c1->SetGrid();

    gr_amplitude->SetLineColor(TColor::GetColor("#C91f16"));
    gr_amplitude->SetMarkerColor(TColor::GetColor("#C91f16"));
    gr_amplitude->SetMarkerStyle(21);
    gr_amplitude->SetMarkerSize(1);
    gr_amplitude->SetLineWidth(2);
    gr_amplitude->SetTitle("");
    gr_amplitude->GetYaxis()->SetRangeUser(10., 14.);
    gr_amplitude->GetXaxis()->SetTitle("#Delta V (Photocathode - MCP) [V]");
    gr_amplitude->GetYaxis()->SetTitle("Amplitude [mV]");
    gr_amplitude->GetXaxis()->CenterTitle();
    gr_amplitude->GetYaxis()->CenterTitle();
    gr_amplitude->Draw("APL");
    c1->SaveAs("../../output/250701_PCScan/PCScan_amp.pdf");
}
