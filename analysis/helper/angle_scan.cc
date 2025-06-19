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

double getAmp(int run_number) {
  TFile *file = new TFile(Form("../../output/250609_Angle/run%d/Analysis_Run_%d.root", run_number, run_number), "READ");
  TH1F* hist = (TH1F*)file->Get("Amplitude");
  double amplitude = hist->GetMean();
  file->Close();
  delete file;

  return amplitude;
}

double getEntries(int run_number) {
  TFile *file = new TFile(Form("../../output/250609_Angle/run%d/Analysis_Run_%d.root", run_number, run_number), "READ");
  TH1F* hist = (TH1F*)file->Get("Amplitude");
  double entries = hist->GetEntries();
  file->Close();
  delete file;

  return entries;
}

void angle_scan() {

  std::vector<Data> points;
  
  points.push_back({-60., getAmp(156), getEntries(156)});
  points.push_back({-45., getAmp(155), getEntries(155)});
  // points.push_back({-30., getAmp(154), getEntries(154)});
  points.push_back({-20., getAmp(153), getEntries(153)});
  points.push_back({-15., getAmp(152), getEntries(152)});
  points.push_back({-10., getAmp(151), getEntries(151)});
  points.push_back({-7.5, getAmp(150), getEntries(150)});
  points.push_back({-5., getAmp(149), getEntries(149)});
  points.push_back({-2.5, getAmp(148), getEntries(148)});
  points.push_back({0., getAmp(147), getEntries(147)});
  points.push_back({2.5, getAmp(157), getEntries(157)});
  points.push_back({5., getAmp(158), getEntries(158)});
  points.push_back({7.5, getAmp(159), getEntries(159)});
  points.push_back({10., getAmp(160), getEntries(160)});
  points.push_back({15., getAmp(161), getEntries(161)});
  points.push_back({20., getAmp(162), getEntries(162)});
  points.push_back({30., getAmp(163), getEntries(163)});
  points.push_back({45., getAmp(164), getEntries(164)});
  points.push_back({60., getAmp(165), getEntries(165)});

  TCanvas *c1 = new TCanvas("", "", 800, 600);
  TGraphErrors *gr = new TGraphErrors();

  int i = 0;
  for (const auto& point : points) {
    gr->SetPoint(i, point.angle, point.amplitude);
    gr->SetPointError(i, 0, point.amplitude / sqrt(point.entries));
    i++;
  }

  gStyle->SetTitleFont(62,"xyz"); 
  gStyle->SetLabelFont(42,"xyz");
  gStyle->SetTitleOffset(1.4,"xy");

  gr->SetLineColor(kBlack);
  gr->SetMarkerColor(kBlack);
  gr->SetMarkerStyle(kOpenSquare);
  gr->SetMarkerSize(1.0);
  gr->SetLineWidth(1.0);
  gr->SetTitle("Angle scan at 1.4 T");
  gr->GetXaxis()->SetTitle("Angle [deg]");
  gr->GetYaxis()->SetTitle("Amplitude [mV]");
  gr->Draw("APL");

  // Draw a vertical line at x = 0
  TLine *line = new TLine(0, gr->GetYaxis()->GetXmin(), 0, gr->GetYaxis()->GetXmax());
  line->SetLineStyle(2); 
  line->SetLineColor(kRed);
  line->Draw();

  c1->SaveAs("../../output/250609_Angle/angle_scan.pdf");
}
