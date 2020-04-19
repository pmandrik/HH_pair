
/*
source /cvmfs/sft.cern.ch/lcg/releases/ROOT/6.12.04-13971/x86_64-slc6-gcc62-opt/ROOT-env.sh
export PATH=/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.12.04-13971/x86_64-slc6-gcc62-opt/bin:$PATH
source /cvmfs/sft.cern.ch/lcg/releases/gcc/6.2.0/x86_64-slc6/setup.sh
cd /beegfs/lfi.mipt.su/scratch/MadGraph

source /cvmfs/sft.cern.ch/lcg/releases/gcc/6.2.0/x86_64-slc6/setup.sh
source      /cvmfs/sft.cern.ch/lcg/releases/ROOT/6.08.06-b32f2/x86_64-slc6-gcc62-opt/ROOT-env.sh
export PATH=/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.08.06-b32f2/x86_64-slc6-gcc62-opt/bin:$PATH

*/


#include "Delphes.C"
#include "LHEReader.C"

TRandom rgen;

TLorentzVector make_jet(Delphes* reader, Long64_t index){
    TLorentzVector vec;
    vec.SetPtEtaPhiM(reader->Jet_PT[index], reader->Jet_Eta[index], reader->Jet_Phi[index], reader->Jet_Mass[index] );
    return vec;
}

TLorentzVector make_lepton(Delphes* reader, Long64_t index, bool electron) {
    TLorentzVector vec;
    if (electron)
        vec.SetPtEtaPhiM(
                reader->Electron_PT[index],
                reader->Electron_Eta[index],
                reader->Electron_Phi[index],
                0.000510998928);
    else
        vec.SetPtEtaPhiM(
                reader->Muon_PT[index],
                reader->Muon_Eta[index],
                reader->Muon_Phi[index],
                0.113428913073);
    return vec;
}


void analyser_WWWW(
        string delphes_file = ""/*"/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/tag_1_delphes_events.root" */,
        string lhe_file = ""/*"/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/unweighted_events.lhe"*/,
        bool lhe_format = false, string mode = "" ) {

    TRandom rgen;

    TFile *file = TFile::Open(delphes_file.c_str());
    TTree *tree = (TTree *) file->Get("Delphes");
    Delphes *reader = new Delphes(tree);

    LHEReader reader_lhe;
    if (lhe_format) {
        reader_lhe.weight_open_pattern = "wgt id=\"";
        reader_lhe.weight_exit_pattern = "</wgt>";
        reader_lhe.weight_middle_pattern = "\">";
    }
    reader_lhe.Init(lhe_file.c_str());

    TH1D *selections = new TH1D("selections", "selections", 100, 0, 100);
    selections->Fill("Total", 0);
    selections->Fill("Total_X_Weight", 0);
    selections->Fill("Selected", 0);
    selections->Fill("Selected_X_Weight", 0);

    vector<int> pdf_indexes;
    if (lhe_format) for (int i = 9; i < 40; i++) pdf_indexes.push_back(i);
    else for (int i = 45; i < 145; i++) pdf_indexes.push_back(i);


    selections->Fill("Pdf_Up", 0);
    selections->Fill("Pdf_Down", 0);
    selections->Fill("muR_05_muF_05", 0);
    selections->Fill("muR_05_muF_10", 0);
    selections->Fill("muR_10_muF_05", 0);
    selections->Fill("muR_10_muF_20", 0);
    selections->Fill("muR_20_muF_10", 0);
    selections->Fill("muR_20_muF_20", 0);

    Long64_t entries = tree->GetEntries();
    double weight_sum = 0;

    for (Long64_t entry = 0; entry < entries; entry++) {
        reader->GetEntry(entry);
        selections->Fill("Total", 1);

        if ((1 + entry) % 5000 == 0) {
            cerr << entry << '/' << entrys << endl;
            return 0;
        }

        reader_lhe.ReadEvent();
        LHEEvent *lhe_info = &reader_lhe.event;
        double weight = 1;
        if (lhe_format) weight = lhe_info->weights_v[0];
        else weight = lhe_info->weights_v[44];

        weight_sum += weight;

        // OBJECT SELECTIONS ==============================================

        selections->Fill("Total_X_Weight", weight);
        selections->Fill("Total (weighted)", weight);
        reader->GetEntry(entry);

        vector<ULong64_t> muons_indexes, electrons_indexes;
        for (ULong64_t i = 0; i < reader->Muon_; ++i) {
            if (reader->Muon_PT[i] <= 10) continue;
            if (TMath::Abs(reader->Muon_Eta[i]) > 2.5) continue;
            if(reader->Muon_IsolationVar[i] > 0.06) continue;
            // Add about 4 lepton channel and Delta_R???
            muons_indexes.push_back(i);
            selections->Fill("Selected muons", 1);
        }

        for (ULong64_t i = 0; i < reader->Electron_; ++i) {
            double eta = TMath::Abs(reader->Electron_Eta[i]);
            if (reader->Electron_PT[i] <= 10) continue;  // not sure, E_t???
            if (eta > 2.47 or (eta > 1.37 and eta < 1.52)) continue;
            if(reader->Electron_IsolationVar[i] > 0.06) continue;
            // Add about 4 lepton channel and Delta_R???
            electrons_indexes.push_back(i);
            selections->Fill("Selected electrons", 1);
        }

        selections->Fill("Minimum lepton p_t > 10 GeV", 1); // Maybe not needed

        vector <ULong64_t> jets_indexes;
        bool b_tag = false;
        for (ULong64_t i = 0; i < reader->Jet_; ++i) {
            if (reader->Jet_PT[i] < 25) continue;
            if (TMath::Abs(reader->Jet_Eta[i]) > 2.5) continue;
            for (ULong64_t i = 0; i < reader->Jet_; ++i)
            if (reader->Jet_BTag[i]) {
                b_tag = true;
                break;
            }
            jets_indexes.push_back(i);
            selections->Fill("Selected jets", 1);
        }

        // пройтись по электронам и мюонам, отобрать те, у которых дельта R больше 0.04 + таблица стр. 12
        // EVENTS SELECTION =========================================================

        ULong64_t number_leptons = electrons_indexes.size() + muons_indexes.size()
        if (number_leptons < 2 and number_leptons > 4) continue;
        selections->Fill("At least 2 electrons or muons", 1);
        selections->Fill("Events with " + to_string(number_leptons) + "electrons or muons", 1); // Maybe not needed

        if (b_tag) continue;
        selections->Fill("No b tags", 1); // Maybe not needed

        Long64_t sum_leptons_charge = 0, min_pt = 1000, max_pt = 0;
        for (ULong64_t i = 0; i < electrons_indexes.size(); i++) {
            sum_leptons_charge += reader->Electron_Charge[i];
            double pt = reader->Electron_PT[electrons_indexes[i]]
            if (pt < min_pt) min_pt = pt;
            if (pt > max_pt) max_pt = pt;
        }
        for (ULong64_t i = 0; i < muons_indexes.size(); i++) {
            sum_leptons_charge += reader->Muon_Charge[i];
            double pt = reader->Electron_PT[electrons_indexes[i]]
            if (pt < min_pt) min_pt = pt;
            if (pt > max_pt) max_pt = pt;
        }

        if (number_leptons == 2) {
            if (TMath::Abs(sum_leptons_charge) != 2) continue;
            if (jets_indexes.size() < 3) continue;
            if (electrons_indexes.size() == 2) {
                TLorentzVector sum = make_lepton(reader, electrons_indexes[0], true)
                        + make_lepton(reader, electrons_indexes[1], true);
                if (TMath::Abs((sum.P - 91.1876) < 10) continue; // ??May be not true????  Comparison with m_Z
            }
            if (min_pt <= 20 or max_pt <= 30) continue;
        }

        else if (number_leptons == 3) {
            if (TMath::Abs(sum_leptons_charge) != 1) continue;
            if (jets_indexes.size() < 2) continue;


            vector<ULong64_t> negative_charge,  positive_charge;
            vector<bool> is_electron_positive, is_electron_negative;
            for (ULong64_t i = 0; i < electrons_indexes.size(); i++) {
                if (reader->Electron_Charge[i] > 0) {
                    positive_charge.push_back(i);
                    is_electron_positive.push_back(true);
                }
                else {
                    negative_charge.push_back(i);
                    is_electron_negative.push_back(true);
                }
            }

            for (ULong64_t i = 0; i < muons_indexes.size(); i++) {
                if (reader->Muon_Charge[i] > 0) {
                    positive_charge.push_back(i);
                    is_electron_positive.push_back(false);
                }
                else {
                    negative_charge.push_back(i);
                    is_electron_negative.push_back(false);
                }
            }

            TLorentzVector sum;
            if (negative_charge.size() == 2) {
                for (ULong64_t i = 0; i < 2, i++) {
                    if (is_electron_negative[i])
                        sum += make_lepton(reader, electrons_indexes[negative_charge[i]], true);
                    else sum += make_lepton(reader, muons_indexes[negative_charge[i]], false);
                }
            }
            else {  // If there are two positive leptons
                for (ULong64_t i = 0; i < 2, i++) {
                    if (is_electron_positive[i])
                        sum += make_lepton(reader, electrons_indexes[positive_charge[i]], true);
                    else sum += make_lepton(reader, muons_indexes[positive_charge[i]], false);
                }
            }

            if (TMath::Abs((sum.P - 91.1876) < 10) continue; // ??May be not true????  Comparison with m_Z

            bool too_low_pt = false;
            for (ULong32_t i = 0; i < 2; i++) {
                if (negative_charge.size() == 2) {
                    if (is_electron_negative[i]) double pt = reader->Electron_PT[negative_charge[i]];
                    else double pt = reader->Muon_PT[negative_charge[i]];
                }
                if (positive_charge.size() == 2) {
                    if (is_electron_negative[i]) double pt = reader->Electron_PT[positive_charge[i]];
                    else double pt = reader->Muon_PT[positive_charge[i]];
                }
                if (pt <= 20) {
                    too_low_pt = true;
                    break;
                }
            }
            if (too_low_pt) continue;

        }

        else {  //  FOUR LEPTONS CHANNEL
            if (TMath::Abs(sum_leptons_charge) != 0) continue;
            if (jets_indexes.size() < 2) continue;
            if (max_pt <= 22) continue;
        }

        selections->Fill("Selected events", 1);














        // W -> bb
        /*
        TLorentzVector P_H2 = make_jet(reader, b_tags[0]) + make_jet(reader, b_tags[1]);
        double m_bb = P_H2.M();
        if(m_bb < 105 || m_bb > 135) continue;
        selections->Fill("Correct Higgs mass", 1);

        if(light.size() > 2) {
            TLorentzVector v1 = make_jet(reader, light[0]);
            TLorentzVector v2 = make_jet(reader, light[1]);
            TLorentzVector v3 = make_jet(reader, light[2]);

            double dR12 = v1.DeltaR(v2);
            double dR23 = v2.DeltaR(v3);
            double dR31 = v3.DeltaR(v1);

            if(dR12 <= dR23) {
                if(dR31 <= dR12) swap(light[1], light[2]); // take dR31
                else ; // take dR12
            } else if(dR31 <= dR23) swap(light[1], light[2]); // take dR31
            else swap(light[0], light[2]); // take dR23
            light.resize(2);
        }

        double m_h = 125;
        TLorentzVector P_w = make_jet(reader, light[0]) + make_jet(reader, light[1]);
        TLorentzVector P_l = make_lepton(reader, highest_lepton);

        TLorentzVector P_wl, P_n;
        P_wl = P_w + P_l;

        P_n.SetPtEtaPhiM(reader->MissingET_MET[0], 0, reader->MissingET_Phi[0], 0.0);



    cout << " =================== > " << endl;
    bool check_higgs_mass = false;
        if(D > 0) {
            long double k = sqrt(D) / 2. / a;

      long double x1 = (-b+ sqrt(D)) / 2. /a;
      long double x2 = (-b- sqrt(D)) / 2. /a;

      cout << x1 << " " << x2 << endl;

            TLorentzVector p1;
            p1.SetXYZM(P_n.Px(), P_n.Py(), x - k, P_n.M());
            TLorentzVector p2;
            p2.SetXYZM(P_n.Px(), P_n.Py(), x + k, P_n.M());

      if( p1.DeltaR(P_l) < p2.DeltaR(P_l) ) P_n = p1;
            else P_n = p2;

    double x_chosen = P_n.Pz();
// =================================
    double x_math = x_chosen - 1.;
    double x_step = 1;
    double x_max  = x_chosen + 1.;

    double C1 = (m_h*m_h - P_wl.M()*P_wl.M())*0.5 + ( P_wl.Px() * P_n.Px() + P_wl.Py() * P_n.Py() );
    cout << "C1 = " << C1 << " " << C << endl;
    double delta_best  = 999999;
    double x_math_best = 0;
    for(; x_math < x_max; x_math += x_step){
      // m_h^2 = (P_wl + P_n) * (P_wl + P_n) = m_wl^2 + 2 * E_wl * sqrt(p_n_t^2 + p_n_z^2) - 2 * (p_n_x * p_wl_x + p_n_y * p_wl_y + p_n_z * p_wl_z)
      // m_h^2 - m_wl^2 + 2 * (p_n_x * p_wl_x + p_n_y * p_wl_y) = 2 * E_wl * sqrt(p_n_t^2 + p_n_z^2) - 2 * p_n_z * p_wl_z
      double C2_p = P_wl.E() * TMath::Sqrt( P_n.Pt()*P_n.Pt() + x_math*x_math ) - x_math * P_wl.Pz();
      double C2_m = C2_p + 2 * x_math * P_wl.Pz();

      cout << P_wl.E() * TMath::Sqrt( P_n.Pt()*P_n.Pt() + x_math*x_math ) << endl;
      cout << x_math * P_wl.Pz() << endl;

      cout << "C2_p = " << C2_p << endl;
      cout << "C2_m = " << C2_m << endl;

      double delta;
      delta = abs( C2_p - C1 );
      // cout << delta << endl;
      if( delta < delta_best ){
        delta_best = delta;
        x_math_best = x_math;
      }
      delta = abs( C2_m - C1 );
      if(delta < delta_best ){
        delta_best = delta;
        x_math_best = -x_math;
      }
    }
    cout << "math x best = " << x_math_best << " " << delta_best << endl;

    TLorentzVector P_n_alt;
    P_n_alt.SetXYZM(P_n.Px(), P_n.Py(), x_math_best, P_n.M());
        TLorentzVector P_H1_alt = P_n_alt + P_wl;
    cout << "m_h_alt = " << P_H1_alt.M() << endl;
// =================================
        }
        else{
      P_n.SetXYZM(P_n.Px(), P_n.Py(), x, P_n.M());
      check_higgs_mass = true;
    }

        selections->Fill("System is reconstructible", 1);

        TLorentzVector P_H1 = P_n + P_wl;
        TLorentzVector P_HH = P_H1 + P_H2;

    // P_n.Print();
    cout << "m_h_def = " << P_H1.M() << endl;

        if( P_n.Pt() <= 25   ) continue;
    selections->Fill(" P_n.Pt() <= 25 ", 1);

    if(check_higgs_mass)
        if( P_H1.M() >= 130  ) continue;
    selections->Fill(" P_H1.M() >= 130 ", 1);

        if( P_H2.Pt() <= 300 ) continue;
    selections->Fill(" P_H2.Pt() <= 300 ", 1);

        if( P_H1.Pt() <= 250 ) continue;
    selections->Fill(" P_H1.Pt() <= 250 ", 1);

        selections->Fill("Non-resonant criteria", 1);
        selections->Fill("Non-resonant criteria (weighted)", weight);
  } // end of the selection loop

  for(int i = 1; i < 100; i++){
    double content = selections->GetBinContent(i);
    string label = selections->GetXaxis()->GetBinLabel(i);

    if(label.size() < 1) break;
    cout << label << " | " << content << " " << content  / entrys << " " << content / weight_sum << " " << content / weight_sum * 0.766 << endl;
  } */

    }
}








