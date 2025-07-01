#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "TH1.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TStyle.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TMath.h"
#include "TLegend.h"

// Double Gaussian function
double doubleGaussian(double *x, double *par) {
    
    // par[0] = amplitude of first Gaussian
    // par[1] = mean of first Gaussian
    // par[2] = sigma of first Gaussian

    // par[3] = amplitude of second Gaussian
    // par[4] = mean of second Gaussian
    // par[5] = sigma of second Gaussian
    
    double firstGaus = par[0] * TMath::Gaus(x[0], par[1], par[2], kTRUE);
    double secondGaus = par[3] * TMath::Gaus(x[0], par[4], par[5], kTRUE);
    
    return firstGaus + secondGaus;
}

void timeRes(int runNumber) {
    
    float xmin = 61000;
    float xmax = 66000;
    
    TFile *file = new TFile((TString)("../../output/250630_BFieldScan/run" + std::to_string(runNumber) + "/Analysis_Run_" + std::to_string(runNumber) + ".root"), "READ");
    TH1F *h_timing = (TH1F*)file->Get("Timing_Diff");
    
    // Convert ps to ns by scaling X axis
    h_timing->GetXaxis()->Set(h_timing->GetNbinsX(), 
                              h_timing->GetXaxis()->GetXmin()/1000.0, 
                              h_timing->GetXaxis()->GetXmax()/1000.0);
    
    // Update range values for ns
    float xmin_ns = xmin / 1000.0;  // Convert to ns
    float xmax_ns = xmax / 1000.0;  // Convert to ns
    
    TF1 *fitFunc = new TF1("fitFunc", doubleGaussian, xmin_ns, xmax_ns, 6);

    gStyle->SetOptStat(0);    
    gStyle->SetTitleFont(22,"xyz"); 
    gStyle->SetLabelFont(22,"xyz");
    gStyle->SetTitleOffset(1.4,"xy");
    
    int maxBin = h_timing->GetMaximumBin();
    double peakX = h_timing->GetBinCenter(maxBin);
    double peakY = h_timing->GetBinContent(maxBin);

    fitFunc->SetParameters(peakY, peakX, 0.2, 20, peakX + 1, 0.4);  // Adjusted for ns scale
    
    fitFunc->SetParName(0, "Amp1");
    fitFunc->SetParName(1, "Mean1");
    fitFunc->SetParName(2, "Sigma1");
    fitFunc->SetParName(3, "Amp2");
    fitFunc->SetParName(4, "Mean2");
    fitFunc->SetParName(5, "Sigma2");
    
    TFitResultPtr fitResult = h_timing->Fit("fitFunc", "S0");
    double chi2 = fitResult->Chi2();
    int ndf = fitResult->Ndf();
    double prob = fitResult->Prob();
    
    TCanvas *canvas = new TCanvas("canvas", "Double Gaussian Fit", 800, 600);
    
    h_timing->GetXaxis()->SetRangeUser(xmin_ns, xmax_ns);
    h_timing->SetMarkerStyle(8);
    h_timing->SetMarkerSize(0.7);
    h_timing->SetMarkerColor(kBlack);
    h_timing->SetTitle("");
    h_timing->GetXaxis()->SetTitle("Time [ns]");
    h_timing->GetYaxis()->SetTitle("Counts");
    h_timing->GetXaxis()->SetTitleOffset(1.4);
    h_timing->GetYaxis()->SetTitleOffset(1.4);
    h_timing->GetXaxis()->SetTitleFont(22);
    h_timing->GetYaxis()->SetTitleFont(22);
    h_timing->GetXaxis()->SetLabelFont(22);
    h_timing->GetYaxis()->SetLabelFont(22);
    h_timing->GetXaxis()->CenterTitle();
    h_timing->GetYaxis()->CenterTitle();
    h_timing->Draw("P");
    
    fitFunc->SetLineColor(kBlack); 
    fitFunc->SetLineWidth(3);
    fitFunc->Draw("same");

    TF1 *g1 = new TF1("g1", "[0]*TMath::Gaus(x,[1],[2],1)", xmin_ns, xmax_ns);
    g1->SetParameters(fitFunc->GetParameter(0), fitFunc->GetParameter(1), fitFunc->GetParameter(2));
    g1->SetLineColor(TColor::GetColor("#C91f16"));
    g1->SetLineWidth(4);
    g1->SetLineStyle(7);
    g1->Draw("same");
    
    TF1 *g2 = new TF1("g2", "[0]*TMath::Gaus(x,[1],[2],1)", xmin_ns, xmax_ns);
    g2->SetParameters(fitFunc->GetParameter(3), fitFunc->GetParameter(4), fitFunc->GetParameter(5));
    g2->SetLineColor(TColor::GetColor("#717581"));
    g2->SetLineWidth(4);
    g2->SetLineStyle(7);
    g2->Draw("same");
    
    double amp1 = fitFunc->GetParameter(0);
    double mean1 = fitFunc->GetParameter(1);
    double sigma1 = fitFunc->GetParameter(2);
    double amp1_err = fitFunc->GetParError(0);
    double mean1_err = fitFunc->GetParError(1);
    double sigma1_err = fitFunc->GetParError(2);

    double amp2 = fitFunc->GetParameter(3);
    double mean2 = fitFunc->GetParameter(4);
    double sigma2 = fitFunc->GetParameter(5);
    double amp2_err = fitFunc->GetParError(3);
    double mean2_err = fitFunc->GetParError(4);
    double sigma2_err = fitFunc->GetParError(5);
    
    // Method 1: Weighted RMS (current method)
    double totalWeight = amp1 + amp2;
    double rms = sqrt(
        (amp1 * (sigma1*sigma1 + mean1*mean1) + amp2 * (sigma2*sigma2 + mean2*mean2)) / totalWeight -
        pow((amp1 * mean1 + amp2 * mean2) / totalWeight, 2)
    );
    
    // Method 2: Variance composition method
    double sigma_composition = sqrt((amp1 * sigma1 * sigma1 + amp2 * sigma2 * sigma2) / totalWeight);
    
    // Method 3: FWHM method
    double maxValue = fitFunc->GetMaximum(xmin_ns, xmax_ns);
    double halfMax = maxValue / 2.0;
    double x1 = fitFunc->GetX(halfMax, xmin_ns, mean1);
    double x2 = fitFunc->GetX(halfMax, mean1, xmax_ns);
    double fwhm = x2 - x1;
    double sigma_fwhm = fwhm / 2.355;  // FWHM = 2.355 * sigma for Gaussian

    // std::cout << "Weighted RMS: " << rms << " ns" << std::endl;
    // std::cout << "Composition Sigma: " << sigma_composition << " ns" << std::endl;
    // std::cout << "FWHM Sigma: " << sigma_fwhm << " ns" << std::endl;
    
    TLegend *legend = new TLegend(0.5, 0.55, 0.84, 0.81);
    legend->SetTextSize(0.04);
    legend->AddEntry(h_timing, "#font[22]{#color[1]{Data}}", "p");
    legend->AddEntry(fitFunc, "#font[22]{#color[1]{Double Gaussian Fit}}", "l");
    // legend->AddEntry((TObject*)0, Form("#font[22]{#color[1]{#chi^{2}/ndf (prob): %.1f/%d (%.3f)}}", chi2, ndf, prob), "");
    legend->AddEntry(g1, Form("#font[22]{#color[%d]{Sigma   %.1f #pm %.1f ps}}", TColor::GetColor("#C91f16"), sigma1*1000, sigma1_err*1000), "l");
    legend->AddEntry(g2, Form("#font[22]{#color[%d]{Sigma   %.1f #pm %.1f ps}}", TColor::GetColor("#717581"), sigma2*1000, sigma2_err*1000), "l");
    // legend->AddEntry((TObject*)0, Form("#font[22]{#color[1]{Weighted RMS: %.1f ps}}", rms*1000), "");
    // legend->AddEntry((TObject*)0, Form("#font[22]{#color[1]{Composition Sigma: %.1f ps}}", sigma_composition*1000), "");
    // legend->AddEntry((TObject*)0, Form("#font[22]{#color[1]{FWHM Sigma: %.1f ps}}", sigma_fwhm*1000), "");
    
    legend->SetBorderSize(0);
    legend->Draw();
    
    canvas->SaveAs((TString)("../../output/250630_BFieldScan/TimeRes_Run_" + std::to_string(runNumber) + ".pdf"));
    
    TFile *outputFile = new TFile((TString)("../../output/250630_BFieldScan/TimeRes_Run_" + std::to_string(runNumber) + ".root"), "RECREATE");
    h_timing->Write();
    fitFunc->Write();
    g1->Write();
    g2->Write();
    canvas->Write();
    outputFile->Close();
    
    delete outputFile;
    delete legend;
    delete g2;
    delete g1;
    delete canvas;
    delete fitFunc;
    delete h_timing;
} 