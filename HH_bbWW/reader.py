

import json
import ROOT

file = open("gen_new_output.json")
text = file.read()

person_dict = json.loads( text )

print person_dict

bsm  = person_dict["bsm"]
bsm2 = person_dict["bsm2"]

graph = ROOT.TGraphErrors(12);

points = 0
aver = 0
for dataset, dataset2 in zip(bsm, bsm2):
  root_file = dataset["data"].split("/")[-1]
  root_file2 = dataset2["data"].split("/")[-1]

  def get_point( d ):
    eff = d["Non-resonant criteria"]
    eff_err = pow( d["Non-resonant criteria"] * d["Total"], 0.5 ) / d["Total"]
    return eff, eff_err

  p1, p1_err = get_point( dataset  )
  p2, p2_err = get_point( dataset2 )

  coff     = p2 / p1
  coff_err = coff * ( p1_err / p1 + p2_err / p2 )

  graph.SetPoint( points, points+1, coff )
  graph.SetPointError( points, 0.5, coff_err )

  points += 1
  print root_file==root_file2, coff, coff_err
  aver += coff
print aver / points

canv = ROOT.TCanvas("canv", "canv")
graph.Draw("APL")
canv.Update()

raw_input()



