
// source /cvmfs/sft.cern.ch/lcg/releases/gcc/6.2.0/x86_64-slc6/setup.sh
// source /cvmfs/sft.cern.ch/lcg/releases/ROOT/6.08.06-b32f2/x86_64-slc6-gcc62-opt/ROOT-env.sh
// export PATH=/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.08.06-b32f2/x86_64-slc6-gcc62-opt/bin:$PATH

#include "Delphes.C"
#include "../LHEReader.C"

using namespace LHEF;

TRandom rgen;
const double JES_unc = 0.02;
const double JER_unc = 0.18;

TLorentzVector make_jet(Delphes* reader, Long64_t index, int JES, int JER) {
	TLorentzVector vec;
	vec.SetPtEtaPhiM(reader->Jet_PT[index], reader->Jet_Eta[index], reader->Jet_Phi[index], reader->Jet_Mass[index]);

	// correction
	double corr = (1 + JES * JES_unc) * (1 + JER * JER_unc * rgen.Gaus());
	vec.SetXYZM(vec.Px() * corr, vec.Py() * corr, vec.Pz() * corr, vec.M());

	return vec;
}

TLorentzVector make_lepton(Delphes* reader, Long64_t index) {
	TLorentzVector vec;
	if(index < 0) {
		index = ~index;
		vec.SetPtEtaPhiM(
			reader->Electron_PT[index],
			reader->Electron_Eta[index],
			reader->Electron_Phi[index],
			0.000510998928
		);
	} else {
		vec.SetPtEtaPhiM(
			reader->Muon_PT[index],
			reader->Muon_Eta[index],
			reader->Muon_Phi[index],
			0.1056583715
		);
	}
	return vec;
}

void GetPDFError(double& pdf_err, HEPEUP& hepeup, const std::vector<unsigned>& pdf_indices) {
	double mean = 0;
	for(size_t i: pdf_indices)
		mean += hepeup.weight(i);
	mean /= pdf_indices.size();

	pdf_err = 0;
	for(size_t i: pdf_indices)
		pdf_err += pow(mean - hepeup.weight(i), 2);
	
	pdf_err /= (pdf_indices.size() - 1);
	pdf_err = sqrt(pdf_err);
}

void analyser_WWbb(
	string data = "/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/SM/tag_2_delphes_events.root",
	string events = "/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/SM/unweighted_events_2.lhe",
	int JES = 0,
	int JER = 0
) {
	bool lhe_format = data.find("SM") == std::string::npos;
  // lhe_format = false;

	TFile file(data.c_str());
	TTree * tree = (TTree*) file.Get("Delphes");
	Delphes * reader = new Delphes (tree);

  LHEReader reader_lhe;
  if( lhe_format ) {
      reader_lhe.weight_open_pattern = "wgt id=\"";
      reader_lhe.weight_exit_pattern = "</wgt>";
      reader_lhe.weight_middle_pattern = "\">";
  }
  reader_lhe.Init( events.c_str() );

  vector<int> pdf_indexes;
  if(lhe_format)   for(int i = 9 ; i < 40 ; i++) pdf_indexes.push_back( i );
  else             for(int i = 45; i < 145; i++) pdf_indexes.push_back( i );

	std::vector<unsigned> pdf_indices;
  	if(lhe_format)
		for(int i = 9; i < 40; ++i)
			pdf_indices.push_back(i);
  	else
		for(int i = 45; i < 145; ++i)
			pdf_indices.push_back(i);
	std::vector<unsigned> used_weights;

	double pdf_err;

	TH1D selections ("selections", "selections", 100, 0, 100);
	selections.Fill("Total", 0);
	selections.Fill("Total (weighted)", 0);
	selections.Fill("At least electron or muon", 0);
	selections.Fill("Maximum lepton p_t > 27 GeV", 0);
	selections.Fill("Exactly two b tags", 0);
	selections.Fill("At least two light jets", 0);
	selections.Fill("Correct Higgs mass", 0);
	selections.Fill("System is reconstructible", 0);
  selections.Fill("Selected", 1);
  selections.Fill("Selected (weighted)", weight);

  Long64_t entries = tree->GetEntries();
	vector<double> higgs;
	for(ULong64_t entry = 0; entry < entries; ++entry) {
		// if(entry % 1000 == 0)
		// cerr << entry << '/' << entries << endl;
    // if(entry > 100000) break; 

		reader->GetEntry(entry);

    reader_lhe.ReadEvent();
    LHEEvent * lhe_info = & reader_lhe.event;

		double weight;
    if(lhe_format) weight = lhe_info->weights_v[0];
    else           weight = lhe_info->weights_v[44];


		selections.Fill("Total", 1);
		selections.Fill("Total (weighted)", weight);

    // OBJECT SELECTIONS ============================================ ============================================
    vector<int> selected_muons_raw;
		for(ULong64_t i = 0; i < reader->Muon_; ++i) {
			if(reader->Muon_PT[i] < 27) continue;
			double eta = fabs(reader->Muon_Eta[i]);
			if(eta > 2.5) continue;
			if(reader->Muon_IsolationVar[i] > 0.06) continue;
      selected_muons_raw.push_back( i );
		}

    vector<int> selected_electrons_raw;
		for(ULong64_t i = 0; i < reader->Electron_; ++i) {
			if(reader->Electron_PT[i] < 27) continue;
			double eta = fabs(reader->Electron_Eta[i]);
			if(eta > 2.47 or (
				eta > 1.37 and
				eta < 1.52
			)) continue;
			if(reader->Electron_IsolationVar[i] > 0.06) continue;
      selected_electrons_raw.push_back( i );
		}

		vector<ULong64_t> jets_raw;
		for(ULong64_t i = 0; i < reader->Jet_; ++i) {
			if(reader->Jet_PT[i] < 20) continue;
			if(fabs(reader->Jet_Eta[i]) > 2.5) continue;
      jets_raw.push_back(i);
		}

    // remove overlapping objects ============================================ ============================================
    // step 1. remove jets overlapping with electrons
		vector<ULong64_t> jets_1;
    for(auto i : jets_raw){
      bool pass = true;
      auto jet_tlv = make_jet(reader, i, JES, JER);
      for(auto j : selected_electrons_raw){
        auto el_tlv = make_lepton(reader, ~j);
        if( jet_tlv.DeltaR( el_tlv ) > 0.2 ) continue;
        pass = false;
        break;
      }
      if( pass ) jets_1.push_back( i );
    }

    // step 2. remove jets overlapping with muons
		vector<ULong64_t> jets_2;
    for(auto i : jets_1){
      bool pass = true;
      auto jet_tlv = make_jet(reader, i, JES, JER);
      for(auto j : selected_muons_raw){
        auto mu_tlv = make_lepton(reader, j);
        if( jet_tlv.DeltaR( mu_tlv ) > 0.2 ) continue;
        if( jet_tlv.Pt() > mu_tlv.Pt() * 0.5 ) continue;
        pass = false;
        break;
      }
      if( pass ) jets_2.push_back( i );
    }

    // step 3. remove electrons overlapping with selected jets
    vector<int> selected_electrons;
    for(auto j : selected_electrons_raw){
      auto el_tlv = make_lepton(reader, ~j);
      bool pass = true;
      for(auto i : jets_raw){
        auto jet_tlv = make_jet(reader, i, JES, JER);
        if( jet_tlv.DeltaR( el_tlv ) > TMath::Min(0.4, 0.04 + 10. / el_tlv.Pt()) ) continue;
        pass = false;
        break;
      }
      if( pass ) selected_electrons.push_back( j );
    }

    // step 4. remove muons overlapping with selected jets
    vector<int> selected_muons;
    for(auto j : selected_muons_raw){
      auto mu_tlv = make_lepton(reader, j);
      bool pass = true;
      for(auto i : jets_raw){
        auto jet_tlv = make_jet(reader, i, JES, JER);
        if( jet_tlv.DeltaR( mu_tlv ) > TMath::Min(0.4, 0.04 + 10. / mu_tlv.Pt()) ) continue;
        pass = false;
        break;
      }
      if( pass ) selected_muons.push_back( j );
    }
    
    // divide selected jets based on b-tagging
		vector<ULong64_t> b_tags;
		vector<ULong64_t> light;
    for(auto i : jets_2){
			if(not reader->Jet_BTag[i]) light.push_back(i);
			else b_tags.push_back(i);
    }

    // EVENT SELECTIONS ============================================ ============================================
		if(selected_muons.size() == 0 and selected_electrons.size() == 0) continue;
		selections.Fill("At least electron or muon", 1);

    double highest_pt = 0;
		ULong64_t highest_lepton = 0;

    for(auto i : selected_electrons){
      if( reader->Electron_PT[i] < highest_pt ) continue;
      highest_pt = reader->Electron_PT[i];
      highest_lepton = ~i;
    }

    for(auto i : selected_muons){
      if( reader->Muon_PT[i] < highest_pt ) continue;
      highest_pt = reader->Muon_PT[i];
      highest_lepton = i;
    }

		if(highest_pt <= 27) continue;
		selections.Fill("Maximum lepton p_t > 27 GeV", 1);

		if(b_tags.size() != 2) continue;
		selections.Fill("Exactly two b tags", 1);

		if(light.size() < 2) continue;
		selections.Fill("At least two light jets", 1);
		
		// W -> bb
		TLorentzVector P_H2 = make_jet(reader, b_tags[0], JES, JER) + make_jet(reader, b_tags[1], JES, JER);
		double m_bb = P_H2.M();
		if(m_bb < 105 || m_bb > 135) continue;
		selections.Fill("Correct Higgs mass", 1);

		if(light.size() > 2) {
			TLorentzVector v1 = make_jet(reader, light[0], JES, JER);
			TLorentzVector v2 = make_jet(reader, light[1], JES, JER);
			TLorentzVector v3 = make_jet(reader, light[2], JES, JER);

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

		TLorentzVector P_w = make_jet(reader, light[0], JES, JER) + make_jet(reader, light[1], JES, JER);
		TLorentzVector P_l = make_lepton(reader, highest_lepton);
		
		TLorentzVector P_wl, P_n;
		P_wl = P_w + P_l;

		P_n.SetPtEtaPhiM(reader->MissingET_MET[0], 0, reader->MissingET_Phi[0], 0.001);

		double m_h = 125;
		double C = (m_h - P_wl.M()) * (m_h + P_wl.M()) * 0.5 + P_wl.Px() * P_n.Px() + P_wl.Py() * P_n.Py();
		double a = P_wl.E() * P_wl.E() - P_wl.Pz() * P_wl.Pz();
		double b = -2 * C * P_wl.Pz();
		double c = P_wl.E() * P_wl.E() * P_n.Pt() * P_n.Pt() - C * C;

    // cout << "C " << C << ", a " << a << ", b " << b << ", c " << c << endl;
    // cout << "C*C " << C*C << ", a " << a << ", b " << b << ", c+C*C " << P_wl.E() * P_wl.E() * P_n.Pt() * P_n.Pt() << endl;

		if(a == 0) continue;
		b /= a;
		c /= a;
		double D = b * b - 4 * c;
		double x = -b / 2;

		if(D > 0) {
			double k = sqrt(D) / 2;
			
			TLorentzVector p1;
			p1.SetXYZM(P_n.Px(), P_n.Py(), x - k, P_n.M());
			TLorentzVector p2;
			p2.SetXYZM(P_n.Px(), P_n.Py(), x + k, P_n.M());
			
			if(C + P_wl.Pz() * p1.Z() < 0) {
				if(C + P_wl.Pz() * p2.Z() < 0)
					continue;
				P_n = p2;
			}
			else if(C + P_wl.Pz() * p2.Z() < 0
				|| p1.DeltaR(P_l) < p2.DeltaR(P_l)) {
				P_n = p1;
			}
			else P_n = p2;
		}
		else P_n.SetXYZM(P_n.Px(), P_n.Py(), x, P_n.M());

		selections.Fill("System is reconstructible", 1);

		TLorentzVector P_H1 = P_n + P_wl;
		TLorentzVector P_HH = P_H1 + P_H2;

		if (P_n.Pt() <= 25
		 || P_H1.M() >= 130
		 || P_H2.Pt() <= 300
		 || P_H1.Pt() <= 250
		) continue;
		
		selections.Fill("Selected", 1);
		selections.Fill("Selected (weighted)", weight);

      double weight_pdf_up, weight_pdf_down;
      lhe_info->GetPDFErrors(weight_pdf_up, weight_pdf_down, weight, pdf_indexes );

      selections.Fill("Pdf_Up", weight_pdf_up );
      selections.Fill("Pdf_Down", weight_pdf_down );
		
      double weights_muR_05_muF_05, weights_muR_05_muF_10, weights_muR_10_muF_05, weights_muR_10_muF_20, weights_muR_20_muF_10, weights_muR_20_muF_20;
      if(lhe_format){
        weights_muR_05_muF_05 = lhe_info->weights_v[8] ;
        weights_muR_05_muF_10 = lhe_info->weights_v[6] ;
        weights_muR_10_muF_05 = lhe_info->weights_v[2] ;
        weights_muR_10_muF_20 = lhe_info->weights_v[1] ;
        weights_muR_20_muF_10 = lhe_info->weights_v[3] ;
        weights_muR_20_muF_20 = lhe_info->weights_v[4] ;
      } else { 
        weights_muR_05_muF_05 = lhe_info->weights_v[0] ;
        weights_muR_05_muF_10 = lhe_info->weights_v[5] ;
        weights_muR_10_muF_05 = lhe_info->weights_v[15] ;
        weights_muR_10_muF_20 = lhe_info->weights_v[24] ;
        weights_muR_20_muF_10 = lhe_info->weights_v[34] ;
        weights_muR_20_muF_20 = lhe_info->weights_v[39] ;
      }

      selections.Fill( "muR_05_muF_05", weights_muR_05_muF_05 );
      selections.Fill( "muR_05_muF_10", weights_muR_05_muF_10 );
      selections.Fill( "muR_10_muF_05", weights_muR_10_muF_05 );
      selections.Fill( "muR_10_muF_20", weights_muR_10_muF_20 );
      selections.Fill( "muR_20_muF_10", weights_muR_20_muF_10 );
      selections.Fill( "muR_20_muF_20", weights_muR_20_muF_20 );
	}

	double total = selections.GetBinContent(1);
	double total_weighted = selections.GetBinContent(9);

	for(int i = 1; i < 100; ++i) {
		double passed = selections.GetBinContent(i);
		string label = selections.GetXaxis()->GetBinLabel(i);

		if(label.size() < 1) break;
		cout << "dic[\"" + label + "\"]" << "=[" << passed << ", " << passed / total_weighted << "]" << endl;
	}
}




