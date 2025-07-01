#include <stdio.h>
#include <TFile.h>
#include <TH1F.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <vector>

struct Data {
    double bField;
    double amplitude;
    double entries;
};

Data getData(int run_number, double bField) {
    TFile *file = new TFile(Form("../../output/250630_BFieldScan/run%d/Analysis_Run_%d.root", run_number, run_number), "READ");
    TH1F* hist = (TH1F*)file->Get("Amplitude");

    Data data;
    data.bField = bField;
    data.amplitude = hist->GetMean();
    data.entries = hist->GetEntries();

    file->Close();
    delete file;

    return data;
}

void BFieldScan() {
   
    std::vector<Data> points;
    std::vector<double> xBins = {0.04, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2945, 1.406, 1.505, 1.608, 1.704};

    for (int i = 0; i < 17; i++) {
        points.push_back(getData(i+101, xBins.at(i)));
    }

    TCanvas *c1 = new TCanvas("", "", 800, 600);
    TGraphErrors *gr = new TGraphErrors();

    int i = 0;
    for (const auto& point : points) {
        gr->SetPoint(i, point.bField, point.amplitude);
        // gr->SetPointError(i, 0, point.amplitude / sqrt(point.entries));
        i++;
    }

    gStyle->SetTitleFont(22,"xyz"); 
    gStyle->SetLabelFont(22,"xyz");
    gStyle->SetTitleOffset(1.4,"x");
    gStyle->SetTitleOffset(1.,"y");

    gr->SetLineColor(kBlue);
    gr->SetMarkerColor(kBlue);
    gr->SetMarkerStyle(21);
    gr->SetMarkerSize(1);
    gr->SetLineWidth(2);
    // gr->SetTitle("B Field Dependence of Amplitude");
    gr->SetTitle("");
    gr->GetXaxis()->SetRangeUser(0., 1.79);
    gr->GetXaxis()->SetTitle("Magnetic Field [T]");
    gr->GetYaxis()->SetTitle("Amplitude [mV]");
    gr->GetXaxis()->CenterTitle();
    gr->GetYaxis()->CenterTitle();
    gr->Draw("APL");
    // c1->SaveAs("../../output/250630_BFieldScan/amp_bField.pdf");

    // For the normalized plot
    gr->Clear();
    c1->Clear();

    double maxVal = 0;
    for (const auto& point : points) {
        if (point.amplitude > maxVal) maxVal = point.amplitude;
    }

    i = 0;
    for (const auto& point : points) {
        double normalizedVal = point.amplitude / maxVal;
        gr->SetPoint(i, point.bField, normalizedVal);
        i++;
    }
    
    gr->SetLineColor(kBlue);
    gr->SetMarkerColor(kBlue);
    gr->SetMarkerStyle(21);
    gr->SetMarkerSize(1);
    gr->SetLineWidth(2);
    // gr->SetTitle("B Field Dependence of Amplitude");
    gr->SetTitle("");
    gr->GetXaxis()->SetRangeUser(0., 1.79);
    gr->GetXaxis()->SetTitle("Magnetic Field [T]");
    gr->GetYaxis()->SetTitle("Normalized Amplitude");
    gr->GetXaxis()->CenterTitle();
    gr->GetYaxis()->CenterTitle();
    gr->GetYaxis()->SetRangeUser(0.1, 1.1);
    c1->SetLogy();
    c1->SetGrid();
    gStyle->SetLineStyleString(3, "[4 8]");
    gr->Draw("APL");
    c1->SaveAs("../../output/250630_BFieldScan/amp_bField_norm.pdf");
}
