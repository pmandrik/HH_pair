

## turn_off_unused_branches.py

used_branches_txt = """
GenJet
GenJet.PT
GenJet.Eta
GenJet.Phi
GenJet.Mass
GenJet.Flavor
GenJet.FlavorAlgo
GenJet.FlavorPhys
GenJet.BTag
GenJet.BTagAlgo
GenJet.BTagPhys
GenJet.Constituents
GenMissingET
GenMissingET.MET
GenMissingET.Eta
GenMissingET.Phi
GenMissingET_size
Jet
Jet.PT
Jet.Eta
Jet.Phi
Jet.Mass
Jet.DeltaEta
Jet.DeltaPhi
Jet.Flavor
Jet.FlavorAlgo
Jet.FlavorPhys
Jet.BTag
Jet.BTagAlgo
Jet.BTagPhys
Jet.Constituents
Electron
Electron.PT
Electron.Eta
Electron.Phi
Electron.Charge
Electron.Particle
Electron.IsolationVar
Electron.IsolationVarRhoCorr
Photon
Photon.PT
Photon.Eta
Photon.Phi
Photon.E
Photon.EhadOverEem
Photon.Particles
Photon.IsolationVar
Photon.IsolationVarRhoCorr
Muon
Muon.PT
Muon.Eta
Muon.Phi
Muon.Charge
Muon.Particle
Muon.IsolationVar
Muon.IsolationVarRhoCorr
MissingET.MET
MissingET.Eta
MissingET.Phi
"""

used_branches = []
for val in used_branches_txt.split("\n"):
  used_branches += [ val ]

input_file = open("Delphes.h")

input = input_file.read()
output = ""

for line in input.split("\n") :
  if not "fChain->SetBranchAddress(" in line : 
    output += line + "\n"
    continue

  line_split = line.split("\"")
  branch_name = line_split[1]

  if branch_name not in used_branches :
    line = line + " fChain->SetBranchStatus(\"" + branch_name + "\", 0);"
    # line = line + " fChain->SetBranchStatus(\"*\", 0);"
    pass

  output += line + "\n"

input_file.close()

output_file = open("Delphes.h", "w+")
output_file.write( output )
output_file.close()






























