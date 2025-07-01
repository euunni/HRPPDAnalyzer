#include <stdio.h>
#include <TFile.h>
#include <TH1F.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TLine.h>

struct Data {
  double angle;
  double amplitude;
  double entries;
};

Data getData(int run_number, double angle) {
  TFile *file = new TFile(Form("../../output/250609_Angle/run%d/Analysis_Run_%d.root", run_number, run_number), "READ");
  TH1F* hist = (TH1F*)file->Get("Amplitude");
  
  Data data;
  data.angle = angle;
  data.amplitude = hist->GetMean();
  data.entries = hist->GetEntries();
  
  file->Close();
  delete file;

  return data;
}

void angleScan() {

  std::vector<Data> points;
  
  points.push_back(getData(156, -60.));
  points.push_back(getData(155, -45.));
  // points.push_back(getData(154, -30.)); // Too low statistics
  points.push_back(getData(153, -20.));
  points.push_back(getData(152, -15.));
  points.push_back(getData(151, -10.));
  points.push_back(getData(150, -7.5));
  points.push_back(getData(149, -5.));
  points.push_back(getData(148, -2.5));
  points.push_back(getData(147, 0.));
  points.push_back(getData(157, 2.5));
  points.push_back(getData(158, 5.));
  points.push_back(getData(159, 7.5));
  points.push_back(getData(160, 10.));
  points.push_back(getData(161, 15.));
  points.push_back(getData(162, 20.));
  points.push_back(getData(163, 30.));
  points.push_back(getData(164, 45.));
  points.push_back(getData(165, 60.));

  TCanvas *c1 = new TCanvas("", "", 800, 600);
  TGraphErrors *gr = new TGraphErrors();

  int i = 0;
  for (const auto& point : points) {
    gr->SetPoint(i, point.angle, point.amplitude);
    // gr->SetPointError(i, 0, point.amplitude / sqrt(point.entries));
    i++;
  }

  gStyle->SetTitleFont(22,"xyz"); 
  gStyle->SetLabelFont(22,"xyz");
  gStyle->SetTitleOffset(1.4,"xy");

  gr->SetLineColor(kBlack);
  gr->SetMarkerColor(kBlack);
  gr->SetMarkerStyle(21);
  gr->SetMarkerSize(1);
  gr->SetLineWidth(2);
  // gr->SetTitle("Angle scan at 1.4 T");
  gr->SetTitle("");
  gr->GetXaxis()->SetTitle("Angle [degree]");
  gr->GetYaxis()->SetTitle("Amplitude [mV]");
  gr->GetXaxis()->CenterTitle();
  gr->GetYaxis()->CenterTitle();
  gr->Draw("APL");

  // Draw a vertical line at x = 0
  TLine *line = new TLine(0, gr->GetYaxis()->GetXmin(), 0, gr->GetYaxis()->GetXmax());
  line->SetLineStyle(2); 
  line->SetLineColor(kRed);
  // line->Draw();

  c1->SaveAs("../../output/250609_Angle/angle_scan_test.pdf");
}
