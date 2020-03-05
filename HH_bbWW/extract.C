// source /cvmfs/sft.cern.ch/lcg/releases/gcc/6.2.0/x86_64-slc6/setup.sh
// source /cvmfs/sft.cern.ch/lcg/releases/ROOT/6.08.06-b32f2/x86_64-slc6-gcc62-opt/ROOT-env.sh
// export PATH=/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.08.06-b32f2/x86_64-slc6-gcc62-opt/bin:$PATH

#include "Delphes.C"
#include "LHEReader.C"

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

void extract(
	string data = "/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/SM/tag_2_delphes_events.root",
	string events = "/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/SM/unweighted_events_2.lhe",
	int JES = 0,
	int JER = 0
) {
	
	TFile file(data.c_str());
	TTree * tree = (TTree*) file.Get("Delphes");

	Delphes * reader = new Delphes (tree);

	TFile fff("out.root", "RECREATE");
	TH1D e_pt("pt", "pt", 100, 0, 100);

	file.cd();

	Long64_t entries = tree->GetEntries();
	vector<double> higgs;
	for(ULong64_t entry = 0; entry < entries; ++entry) {
		if(entry % 1000 == 0)
			cerr << entry << '/' << entries << endl;

		reader->GetEntry(entry);

		if(reader->Muon_ == 0 and reader->Electron_ == 0) continue;

		double highest_pt = 27;
		ULong64_t highest_lepton = 0;
		for(ULong64_t i = 0; i < reader->Muon_; ++i) {
			e_pt.Fill(reader->Muon_PT[i]);
		}
		for(ULong64_t i = 0; i < reader->Electron_; ++i) {
			e_pt.Fill(reader->Electron_PT[i]);
		}
		continue;
		if(highest_pt <= 27) continue;
		TLorentzVector P_l = make_lepton(reader, highest_lepton);

		vector<ULong64_t> b_tags;
		vector<ULong64_t> light;
		for(ULong64_t i = 0; i < reader->Jet_; ++i) {
			if(reader->Jet_PT[i] < 20) continue;
			if(fabs(reader->Jet_Eta[i]) > 2.5) continue;
			if(not reader->Jet_BTag[i]) light.push_back(i);
			else b_tags.push_back(i);
		}

		if(b_tags.size() != 2) continue;

		if(light.size() < 2) continue;
		
		// W -> bb
		TLorentzVector P_H2 = make_jet(reader, b_tags[0], JES, JER) + make_jet(reader, b_tags[1], JES, JER);
		double m_bb = P_H2.M();
		if(m_bb < 105 || m_bb > 135) continue;

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

		double m_h = 125;
		TLorentzVector P_w = make_jet(reader, light[0], JES, JER) + make_jet(reader, light[1], JES, JER);
		
		TLorentzVector P_wl, P_n;
		P_wl = P_w + P_l;

		P_n.SetPtEtaPhiM(reader->MissingET_MET[0], 0, reader->MissingET_Phi[0], 0.001);

		double C = (m_h - P_wl.M()) * (m_h + P_wl.M()) * 0.5 + P_wl.Px() * P_n.Px() + P_wl.Py() * P_n.Py();
		double a = P_wl.E() * P_wl.E() - P_wl.Pz() * P_wl.Pz();
		double b = -2 * C * P_wl.Pz();
		double c = P_wl.E() * P_wl.E() * P_n.Pt() * P_n.Pt() - C * C;

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

		TLorentzVector P_H1 = P_n + P_wl;
		TLorentzVector P_HH = P_H1 + P_H2;

		if (P_n.Pt() <= 25
		 || P_H1.M() >= 130
		 || P_H2.Pt() <= 300
		 || P_H1.Pt() <= 250
		) continue;
	}

	fff.cd();
	fff.Write();
}
