
#include "Delphes.C"
#include "../LHEReader.C"
#include "../Delphes_help_func.C"

void filter_objects(vector<int> & vec_to_filter, const vector<int> & vec_ref, string type_1, string type_2, double delta_R, Delphes * reader){
  vector<int> buffer;
  for(auto i : vec_to_filter){
    TLorentzVector vec_1 = make_tlv(reader, i, type_1);
    for(auto j : vec_ref){
      TLorentzVector vec_2 = make_tlv(reader, j, type_2);
      if( vec_2.DeltaR( vec_1 ) > delta_R ) continue;
      // cout << "Filter !!! " << type_1 << " " << type_2 << " " << i << " " << j << endl;
      goto exit_mark;
    }
    buffer.push_back( i );
    exit_mark: continue;
  }

  vec_to_filter = buffer;
}


void analyser_yyWW_ATLAS( string delphes_file, string lhe_file, bool lhe_format = false, string mode = ""){
  vector<string> delphes_files = { delphes_file };
  vector<string> lhe_files     = { lhe_file     };

  TH1D * selections = new TH1D("selections", "selections", 100, 0, 100);
  selections->Fill("Total", 0);
  selections->Fill("Total_X_Weight", 0);
  selections->Fill("Selected", 0);
  selections->Fill("Selected_X_Weight", 0);

  vector<int> pdf_indexes;
  if(lhe_format)   for(int i = 9 ; i < 40 ; i++) pdf_indexes.push_back( i );
  else             for(int i = 45; i < 145; i++) pdf_indexes.push_back( i );

  selections->Fill( "Pdf_Up"       , 0 );
  selections->Fill( "Pdf_Down"     , 0 );
  selections->Fill( "muR_05_muF_05", 0 );
  selections->Fill( "muR_05_muF_10", 0 );
  selections->Fill( "muR_10_muF_05", 0 );
  selections->Fill( "muR_10_muF_20", 0 );
  selections->Fill( "muR_20_muF_10", 0 );
  selections->Fill( "muR_20_muF_20", 0 );

  Long64_t total_entrys = 0;
  double total_weight_sum = 0;

  for(int i = 0; i < delphes_files.size(); i++){

    cout << lhe_files.at(i).c_str() << " " << delphes_files.at(i).c_str() << endl;

    TFile * file = TFile::Open( delphes_files.at(i).c_str() );
    TTree * tree = (TTree*) file->Get("Delphes");
    Delphes * reader = new Delphes( tree );

    LHEReader reader_lhe;
    if( lhe_format ) {
      // cout << "!!!!" << endl;
      reader_lhe.weight_open_pattern = "wgt id=\"";
      reader_lhe.weight_exit_pattern = "</wgt>";
      reader_lhe.weight_middle_pattern = "\">";
    }
    reader_lhe.Init( lhe_files.at(i).c_str() );

    Long64_t entrys = tree->GetEntries();
    Long64_t entry = 0;
    double weight_sum = 0;
    for(;entry < entrys;entry++){

      reader->GetEntry(entry);
      selections->Fill("Total", 1);

      reader_lhe.ReadEvent();
      LHEEvent * lhe_info = & reader_lhe.event;
      double weight = 1;
      if(lhe_format) weight = lhe_info->weights_v[0];
      else           weight = lhe_info->weights_v[44];

      // OBJECT SELECTIONS ==============================================
      selections->Fill("Total_X_Weight", weight);
      weight_sum += weight;

      // photon candidates --- --- --- --- --- --- --- 
      vector<int> photon_candidates;
      for(int i = 0; i < reader->Photon_; i++){
        if( reader->Photon_PT[i] < 25 ) continue;
        if( TMath::Abs(reader->Photon_Eta[i]) > 2.37 ) continue;
        if( TMath::Abs(reader->Photon_Eta[i]) > 1.37 and TMath::Abs(reader->Photon_Eta[i]) < 1.52 ) continue;
        if( reader->Photon_IsolationVar[i] > 0.05 ) continue;
        photon_candidates.push_back(i);
      }

      // muon candidates --- --- --- --- --- --- --- 
      vector<int> muon_candidates;
      for(int i = 0; i < reader->Muon_; i++){
        if( TMath::Abs(reader->Muon_Eta[i]) > 2.7 ) continue;
        if( reader->Muon_PT[i] < 10 ) continue;
        if( reader->Muon_IsolationVar[i] > 0.05 ) continue; 
        muon_candidates.push_back(i);
      }

      // electron candidates --- --- --- --- --- --- --- 
      vector<int> electron_candidates;
      for(int i = 0; i < reader->Electron_; i++){
        if( reader->Electron_PT[i] < 10 ) continue;
        if( TMath::Abs(reader->Electron_Eta[i]) > 2.47 ) continue;
        if( TMath::Abs(reader->Electron_Eta[i]) > 1.37 and TMath::Abs(reader->Electron_Eta[i]) < 1.52 ) continue;
        if( reader->Electron_IsolationVar[i] > 0.05 ) continue; 
        electron_candidates.push_back(i);
      }

      // jet candidates --- --- --- --- --- --- --- 
      vector<int> jet_candidates;
      for(int i = 0; i < reader->Jet_; i++){
        if( reader->Jet_PT[i] < 25 ) continue;
        if( TMath::Abs(reader->Jet_Eta[i]) > 2.5 ) continue;
        jet_candidates.push_back(i);
      }

      // objects cleaning --- --- --- --- --- --- --- 
      // cout << "Delta R(e, y) > 0.4" << endl;
      filter_objects(electron_candidates, photon_candidates, "electron", "photon", 0.4, reader);
      // cout << "Delta R(jet, y) > 0.4" << endl;
      filter_objects(jet_candidates, photon_candidates, "jet", "photon", 0.4, reader);
      // cout << "Delta R(jet, e) > 0.2" << endl;
      filter_objects(jet_candidates, electron_candidates, "jet", "electron", 0.2, reader);
      // cout << "Delta R(e, jet) > 0.4" << endl;
      filter_objects(electron_candidates, jet_candidates, "electron", "jet", 0.4, reader);
      // cout << "Delta R(mu, y) > 0.4" << endl;
      filter_objects(muon_candidates, photon_candidates, "muon", "photon", 0.4, reader);
      // cout << "Delta R(mu, jet) > 0.4" << endl;
      filter_objects(muon_candidates, jet_candidates, "muon", "jet", 0.4, reader);

      vector<int> ljet_candidates;
      vector<int> bjet_candidates;
      for(auto i : jet_candidates){
        if( not reader->Jet_BTag[i] ) ljet_candidates.push_back(i);
        else bjet_candidates.push_back(i);
      }

      // EVENT SELECTIONS ==============================================
      // No b-jets:
      if( bjet_candidates.size() ) continue;
      selections->Fill("b-jets veto", 1);

      // At least two light jets
      if( ljet_candidates.size() < 2) continue;
      selections->Fill("at least two light jets", 1);

      // At least one lepton
      if( electron_candidates.size() + muon_candidates.size() < 1) continue;
      selections->Fill("At least one lepton", 1);

      // at least two photons
      if( photon_candidates.size() < 2) continue;
      selections->Fill("at least two photons", 1);

      // E_T / m_yy > 0.35 (0.25)
      int leading_photon_i = photon_candidates.at(0);
      int second_photon_i  = photon_candidates.at(1);

      auto leading_photon_tlv = make_photon(reader, leading_photon_i);
      auto  second_photon_tlv = make_photon(reader, second_photon_i);

      // cout << entry << " " << leading_photon_tlv.Pt() << endl;

      if( leading_photon_tlv.Et() < 35 or second_photon_tlv.Et() < 25 ) continue;
      selections->Fill("leading_photon_tlv.Et() > 35 and second_photon_tlv.Et() > 25", 1);

      auto H_yy_tlv = leading_photon_tlv + second_photon_tlv;
      double m_yy   = (leading_photon_tlv + second_photon_tlv).M();
      
      if( leading_photon_tlv.Et() < m_yy * 0.35) continue;
      selections->Fill("leading_photon_tlv.Et() > m_yy * 0.35", 1);

      if( second_photon_tlv.Et() < m_yy * 0.25) continue;
      selections->Fill("second_photon_tlv.Et() > m_yy * 0.25", 1);

      // pT yy > 100
      if( H_yy_tlv.Pt() < 100 ) continue;
      selections->Fill("H_yy_tlv.Pt() < 100", 1);

      // M yy in [105, 160]
      if( H_yy_tlv.M() < 105 or H_yy_tlv.M() > 160 ) continue;
      selections->Fill("H_yy_tlv.M() < 105 or H_yy_tlv.M() > 160", 1);

      // M in 125 +- 2 * 1.7
      // if( H_yy_tlv.M() < 121.6 or H_yy_tlv.M() > 128.4 ) continue;
      // selections->Fill("M in 125 +- 2 * 1.7", 1);

      // DONE !!! 
      selections->Fill("Selected", 1);
      selections->Fill("Selected_X_Weight", weight);

      double weight_pdf_up, weight_pdf_down;
      lhe_info->GetPDFErrors(weight_pdf_up, weight_pdf_down, weight, pdf_indexes );

      selections->Fill("Pdf_Up", weight_pdf_up );
      selections->Fill("Pdf_Down", weight_pdf_down );

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

      selections->Fill( "muR_05_muF_05", weights_muR_05_muF_05 );
      selections->Fill( "muR_05_muF_10", weights_muR_05_muF_10 );
      selections->Fill( "muR_10_muF_05", weights_muR_10_muF_05 );
      selections->Fill( "muR_10_muF_20", weights_muR_10_muF_20 );
      selections->Fill( "muR_20_muF_10", weights_muR_20_muF_10 );
      selections->Fill( "muR_20_muF_20", weights_muR_20_muF_20 );

    }

    total_entrys     += entrys;
    total_weight_sum += weight_sum;
    file->Close();
  }

	for(int i = 1; i < 100; ++i) {
		double passed = selections->GetBinContent(i);
		string label = selections->GetXaxis()->GetBinLabel(i);

		if(label.size() < 1) break;
		cout << "dic[\"" + label + "\"]" << "=[" << passed << ", " << passed  / total_entrys << ", " << passed / total_weight_sum << "]" << endl;
	}

}



