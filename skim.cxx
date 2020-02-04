/*
 * Implementation of the skimming step of the analysis
 *
 * The skimming step reduces the inital generic samples to a dataset optimized
 * for this specific analysis. Most important, the skimming removes all events
 * from the initial dataset, which are not of interest for our study and builds
 * from the reconstructed muons and taus a valid pair, which may originate from
 * the decay of a Higgs boson.
 */

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "Math/Vector4D.h"
#include "TStopwatch.h"

#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>



/*
 * Perform a selection on the minimal requirements of an event
 */
template <typename T>
auto MinimalSelection(T &df) {
    return df.Filter("HLT_IsoMu17_eta2p1_LooseIsoPFTau20 == true", "Passes trigger")
             .Filter("nMuon > 0", "nMuon > 0")
             .Filter("nTau > 0", "nTau > 0");
}


/*
 * Find the interesting muons in the muon collection
 */
template <typename T>
auto FindGoodMuons(T &df) {
    return df.Define("goodMuons", "abs(Muon_eta) < 2.1 && Muon_pt > 17 && Muon_tightId == true");
}


/*
 * Find the interesting taus in the tau collection
 *
 * The tau candidates in this collection represent hadronic decays of taus, which
 * means that the tau decays to combinations of pions and neutrinos in the final
 * state.
 */
template <typename T>
auto FindGoodTaus(T &df) {
    return df.Define("goodTaus", "Tau_charge != 0 && abs(Tau_eta) < 2.3 && Tau_pt > 20 &&\
                                  Tau_idDecayMode == true && Tau_idIsoTight == true && \
                                  Tau_idAntiEleTight == true && Tau_idAntiMuTight == true");
}


/*
 * Reduce the dataset to the interesting events containing at least one interesting
 * muon and tau candidate.
 */
template <typename T>
auto FilterGoodEvents(T &df) {
    return df.Filter("Sum(goodTaus) > 0", "Event has good taus")
             .Filter("Sum(goodMuons) > 0", "Event has good muons");
}


/*
 * Helper function to compute the difference in the azimuth coordinate taking
 * the boundary conditions at 2 * pi into account.
 */
namespace Helper {
template <typename T>
float DeltaPhi(T v1, T v2, const T c = M_PI)
{
    auto r = std::fmod(v2 - v1, 2.0 * c);
    if (r < -c) {
        r += 2.0 * c;
    }
    else if (r > c) {
        r -= 2.0 * c;
    }
    return r;
}
}


/*
 * Select a muon-tau pair from the collections of muons and taus passing the
 * initial selection. The selected pair represents the candidate for this event
 * for a Higgs boson decay to two tau leptons of which one decays to a hadronic
 * final state (most likely a combination of pions) and one decays to a muon and
 * a neutrino.
 */
template <typename T>
auto FindMuonTauPair(T &df) {
    using namespace ROOT::VecOps;
    auto build_pair = [](RVec<int>& goodMuons, RVec<float>& pt_1, RVec<float>& eta_1, RVec<float>& phi_1,
                         RVec<int>& goodTaus, RVec<float>& iso_2, RVec<float>& eta_2, RVec<float>& phi_2)
            {
                 // Get indices of all possible combinations
                 auto comb = Combinations(pt_1, eta_2);
                 const auto numComb = comb[0].size();

                 // Find valid pairs based on delta r
                 std::vector<int> validPair(numComb, 0);
                 for(size_t i = 0; i < numComb; i++) {
                     const auto i1 = comb[0][i];
                     const auto i2 = comb[1][i];
                     if(goodMuons[i1] == 1 && goodTaus[i2] == 1) {
                         const auto deltar = sqrt(
                                 pow(eta_1[i1] - eta_2[i2], 2) +
                                 pow(Helper::DeltaPhi(phi_1[i1], phi_2[i2]), 2));
                         if (deltar > 0.5) {
                             validPair[i] = 1;
                         }
                     }
                 }

                 // Find best muon based on pt
                 int idx_1 = -1;
                 float maxPt = -1;
                 for(size_t i = 0; i < numComb; i++) {
                     if(validPair[i] == 0) continue;
                     const auto tmp = comb[0][i];
                     if(maxPt < pt_1[tmp]) {
                         maxPt = pt_1[tmp];
                         idx_1 = tmp;
                     }
                 }

                 // Find best tau based on iso
                 int idx_2 = -1;
                 float minIso = 999;
                 for(size_t i = 0; i < numComb; i++) {
                     if(validPair[i] == 0) continue;
                     if(int(comb[0][i]) != idx_1) continue;
                     const auto tmp = comb[1][i];
                     if(minIso > iso_2[tmp]) {
                         minIso = iso_2[tmp];
                         idx_2 = tmp;
                     }
                 }

                 return std::vector<int>({idx_1, idx_2});
            };

    return df.Define("pairIdx", build_pair,
                     {"goodMuons", "Muon_pt", "Muon_eta", "Muon_phi",
                      "goodTaus", "Tau_relIso_all", "Tau_eta", "Tau_phi"})
             .Define("idx_1", "pairIdx[0]")
             .Define("idx_2", "pairIdx[1]")
             .Filter("idx_1 != -1", "Valid muon in selected pair")
             .Filter("idx_2 != -1", "Valid tau in selected pair");
}


/*
 * Declare all variables which we want to study in the analysis
 */
template <typename T>
auto DeclareVariables(T &df) {
    auto add_p4 = [](float pt, float eta, float phi, float mass)
    {
        return ROOT::Math::PtEtaPhiMVector(pt, eta, phi, mass);
    };

    using namespace ROOT::VecOps;
    auto get_first = [](RVec<float> &x, RVec<int>& g)
    {
        if (Sum(g) >= 1) return x[g][0];
        return -999.f;
    };

    auto get_second = [](RVec<float> &x, RVec<int>& g)
    {
        if (Sum(g) >= 2) return x[g][1];
        return -999.f;
    };

    auto compute_mjj = [](ROOT::Math::PtEtaPhiMVector& p4, RVec<int>& g)
    {
        if (Sum(g) >= 2) return float(p4.M());
        return -999.f;
    };

    auto compute_ptjj = [](ROOT::Math::PtEtaPhiMVector& p4, RVec<int>& g)
    {
        if (Sum(g) >= 2) return float(p4.Pt());
        return -999.f;
    };

    auto compute_jdeta = [](float x, float y, RVec<int>& g)
    {
        if (Sum(g) >= 2) return x - y;
        return -999.f;
    };

    auto compute_mt = [](float pt_1, float phi_1, float pt_met, float phi_met)
    {
        const auto dphi = Helper::DeltaPhi(phi_1, phi_met);
        return std::sqrt(2.0 * pt_1 * pt_met * (1.0 - std::cos(dphi)));
    };

    return df.Define("pt_1", "Muon_pt[idx_1]")
             .Define("eta_1", "Muon_eta[idx_1]")
             .Define("phi_1", "Muon_phi[idx_1]")
             .Define("m_1", "Muon_mass[idx_1]")
             .Define("iso_1", "Muon_pfRelIso03_all[idx_1]")
             .Define("q_1", "Muon_charge[idx_1]")
             .Define("pt_2", "Tau_pt[idx_2]")
             .Define("eta_2", "Tau_eta[idx_2]")
             .Define("phi_2", "Tau_phi[idx_2]")
             .Define("m_2", "Tau_mass[idx_2]")
             .Define("iso_2", "Tau_relIso_all[idx_2]")
             .Define("q_2", "Tau_charge[idx_2]")
             .Define("dm_2", "Tau_decayMode[idx_2]")
             .Define("pt_met", "MET_pt")
             .Define("phi_met", "MET_phi")
             .Define("p4_1", add_p4, {"pt_1", "eta_1", "phi_1", "m_1"})
             .Define("p4_2", add_p4, {"pt_2", "eta_2", "phi_2", "m_2"})
             .Define("p4", "p4_1 + p4_2")
             .Define("mt_1", compute_mt, {"pt_1", "phi_1", "pt_met", "phi_met"})
             .Define("mt_2", compute_mt, {"pt_2", "phi_2", "pt_met", "phi_met"})
             .Define("m_vis", "float(p4.M())")
             .Define("pt_vis", "float(p4.Pt())")
             .Define("npv", "PV_npvs")
             .Define("goodJets", "Jet_puId == true && abs(Jet_eta) < 4.7 && Jet_pt > 30")
             .Define("njets", "Sum(goodJets)")
             .Define("jpt_1", get_first, {"Jet_pt", "goodJets"})
             .Define("jeta_1", get_first, {"Jet_eta", "goodJets"})
             .Define("jphi_1", get_first, {"Jet_phi", "goodJets"})
             .Define("jm_1", get_first, {"Jet_mass", "goodJets"})
             .Define("jbtag_1", get_first, {"Jet_btag", "goodJets"})
             .Define("jpt_2", get_second, {"Jet_pt", "goodJets"})
             .Define("jeta_2", get_second, {"Jet_eta", "goodJets"})
             .Define("jphi_2", get_second, {"Jet_phi", "goodJets"})
             .Define("jm_2", get_second, {"Jet_mass", "goodJets"})
             .Define("jbtag_2", get_second, {"Jet_btag", "goodJets"})
             .Define("jp4_1", add_p4, {"jpt_1", "jeta_1", "jphi_1", "jm_1"})
             .Define("jp4_2", add_p4, {"jpt_2", "jeta_2", "jphi_2", "jm_2"})
             .Define("jp4", "jp4_1 + jp4_2")
             .Define("mjj", compute_mjj, {"jp4", "goodJets"})
             .Define("ptjj", compute_ptjj, {"jp4", "goodJets"})
             .Define("jdeta", compute_jdeta, {"jeta_1", "jeta_2", "goodJets"});
}


/*
 * Add the event weight to the dataset as the column "weight"
 */
template <typename T>
auto AddEventWeight(T &df, const std::string& sample, const float numEvents, const float xsec, const float lumi, const float scale) {
    if (sample.find("Run2012") != std::string::npos) {
        return df.Define("weight", [&](){ return 1.0; });
    } else {
        return df.Define("weight", [&](){ return xsec / numEvents * lumi * scale; });
    }
}


/*
 * Check that the generator particles matched to the identified taus are
 * actually taus and add this information to the dataset.
 *
 * This information is used to estimate the fraction of events that are falsely
 * identified as taus, e.g., electrons or jets that could fake such a particle.
 */
template <typename T>
auto CheckGeneratorTaus(T &df, const std::string& sample) {
    if (sample.find("Run2012") != std::string::npos) {
        return df.Define("gen_match", "false");
    } else {
        return df.Define("gen_match",
                         "abs(GenPart_pdgId[Muon_genPartIdx[idx_1]]) == 15 && \
                          abs(GenPart_pdgId[Tau_genPartIdx[idx_2]]) == 15");
    }
}


/*
 * Declare all variables which shall end up in the final reduced dataset
 */
const std::vector<std::string> finalVariables = {
    "njets", "npv",
    "pt_1", "eta_1", "phi_1", "m_1", "iso_1", "q_1", "mt_1",
    "pt_2", "eta_2", "phi_2", "m_2", "iso_2", "q_2", "mt_2", "dm_2",
    "jpt_1", "jeta_1", "jphi_1", "jm_1", "jbtag_1",
    "jpt_2", "jeta_2", "jphi_2", "jm_2", "jbtag_2",
    "pt_met", "phi_met", "m_vis", "pt_vis", "mjj", "ptjj", "jdeta",
    "gen_match", "run", "weight"
};


/*
 * Main function of the skimming step of the analysis
 *
 * The function loops over the input sample, reduces the content to the
 * interesting events and writes them to new files.
 */
int main(int argc, char **argv) {
    if(argc != 6) {
        std::cout << "Use executable with following arguments: ./skim input output cross_section integrated_luminosity scale" << std::endl;
        return -1;
    }
    std::string input = argv[1];
    std::cout << ">>> Process input: " << input << std::endl;

    TStopwatch time;
    time.Start();

    ROOT::RDataFrame df("Events", input);
    const auto numEvents = *df.Count();
    std::cout << "Number of events: " << numEvents << std::endl;

    const auto xsec = atof(argv[3]);
    std::cout << "Cross-section: " << xsec << std::endl;

    const auto lumi = atof(argv[4]);
    std::cout << "Integrated luminosity: " << lumi << std::endl;

    const auto scale = atof(argv[5]);
    std::cout << "Global scaling: " << scale << std::endl;

    auto df2 = MinimalSelection(df);
    auto df3 = FindGoodMuons(df2);
    auto df4 = FindGoodTaus(df3);
    auto df5 = FilterGoodEvents(df4);
    auto df6 = FindMuonTauPair(df5);
    auto df7 = DeclareVariables(df6);
    auto df8 = CheckGeneratorTaus(df7, input);
    auto df9 = AddEventWeight(df8, input, numEvents, xsec, lumi, scale);

    auto dfFinal = df9;
    auto report = dfFinal.Report();
    const std::string output = argv[2];
    std::cout << "Output name: " << output << std::endl;
    dfFinal.Snapshot("Events", output, finalVariables);
    time.Stop();

    report->Print();
    time.Print();
}
