
TLorentzVector make_jet(Delphes * reader, int & index){
  TLorentzVector vec;
  vec.SetPtEtaPhiM(reader->Jet_PT[index], reader->Jet_Eta[index], reader->Jet_Phi[index], reader->Jet_Mass[index] );
  return vec;
}

TLorentzVector make_muon(Delphes * reader, int & index){
  TLorentzVector vec;
  vec.SetPtEtaPhiM(reader->Muon_PT[index], reader->Muon_Eta[index], reader->Muon_Phi[index], 0.105 );
  return vec;
}

TLorentzVector make_electron(Delphes * reader, int & index){
  TLorentzVector vec;
  vec.SetPtEtaPhiM(reader->Electron_PT[index], reader->Electron_Eta[index], reader->Electron_Phi[index], 0.005 );
  return vec;
}

TLorentzVector make_photon(Delphes * reader, int & index){
  TLorentzVector vec;
  vec.SetPtEtaPhiM(reader->Photon_PT[index], reader->Photon_Eta[index], reader->Photon_Phi[index], 0 );
  return vec;
}

TLorentzVector make_genjet(Delphes * reader, int & index){
  TLorentzVector vec;
  vec.SetPtEtaPhiM(reader->GenJet_PT[index], reader->GenJet_Eta[index], reader->GenJet_Phi[index], reader->GenJet_Mass[index] );
  return vec;
}

TLorentzVector get_nearest_vec(const TLorentzVector & dir, const vector<TLorentzVector> & vecs){
  double dR = dir.DeltaR( vecs.at(0) );
  int best_index = 0;

  for(int i = 1; i < vecs.size(); i++){
    double alt_dR = dir.DeltaR( vecs.at(i) );
    if( alt_dR < dR ){
      best_index = i;
      dR = alt_dR;
    }
  }
  
  return vecs.at( best_index );
}

TLorentzVector make_tlv(Delphes * reader, int & index, string & type){
  TLorentzVector vec;
  if(type == "jet")      return make_jet(reader, index);
  if(type == "photon")   return make_photon(reader, index);
  if(type == "muon")     return make_muon(reader, index);
  if(type == "electron") return make_electron(reader, index);
  cout << "Unknow type " << type << endl;
  return vec;
} 


