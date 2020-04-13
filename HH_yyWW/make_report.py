

def get_eff(file_name):
  file = open( file_name)
  data = file.read()
  dic = {}
  for line in data.split("\n"):
    if "dic" not in line : continue
    exec( line )
  
  eff = dic["Selected_X_Weight"][2]
  err = pow( float( dic["Selected"][0] ), 0.5) / float( dic["Selected"][0] )
  return float(eff), err, dic["Selected"][0]

effs = []
corr_factor = 1. / 0.2882464 / 0.0208004877314935
for i in xrange(1, 13):
  eff, err, n = get_eff( "cms_results_v0/log_EFT_" + str(i) + ".log" )
  effs += [ ["EFT_" + str(i), eff * corr_factor, eff * err * corr_factor, n] ]

eff, err, n = get_eff( "cms_results_v0/log_SM.log" )
effs += [ ["SM", eff * corr_factor, eff * err * corr_factor, n] ]

print "ATLAS Efficiency (Geant4) = 8.5%  "
print "CMS Efficiency (Delphes) +- uncertantie due to finit MC samples, number of selected events from 500k sample (non-filtered):"
for (name, eff, err, n) in effs:
  print name, "%.2f +- %.2f " % (eff*100, err*100), n, "  "
print "Number of event to generate (HH>WWgg, WW>elnujj) in order to reach sys uncertantie due to finit MC samples at the level of 0.5*lumi unc & expected number of selected events :  "
for (name, eff, err, n) in effs:
  unc_error = 0.5 * 3.2 / 100.
  print name, int(1. / pow(unc_error, 2) / eff), int(1. / pow(unc_error, 2)), "  "



