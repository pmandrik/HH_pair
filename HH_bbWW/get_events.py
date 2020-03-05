import os
import json

"""
source /cvmfs/sft.cern.ch/lcg/releases/gcc/6.2.0/x86_64-slc6/setup.sh
source /cvmfs/sft.cern.ch/lcg/releases/ROOT/6.08.06-b32f2/x86_64-slc6-gcc62-opt/ROOT-env.sh
export PATH=/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.08.06-b32f2/x86_64-slc6-gcc62-opt/bin:$PATH
"""

def analyse_bbWW( index, eff_fcnc = 0.000035, err_fcnc = 0.00001 ):
  N_observed = 22
  N_back_exp = 21
  err_back   = 8

  command  = "./ns_0_HH_bbbb "
  command += str(N_observed) + " " + str( eff_fcnc * 10000. ) + " "
  command += str(N_back_exp) + " " + str( err_back ) + " "
  command += str( err_fcnc * 10000. )

  os.chdir( "/beegfs/lfi.mipt.su/scratch/shta/build_2/" )
  myCmd = os.popen( command ).read()
  print(myCmd)

  def get_limit( lines ):
    for line in lines.split("\n") : 
      if "limits:" in line : 
        items = line.split(" ")
        return float( items[-1] )

  limit = get_limit( myCmd )
  print "95% limit = ", limit, limit * eff_fcnc # 1545.64 / 36.1 = xsec = 
  
  lumi            = 36.1 # fb-1
  XSEC_x_Br_limit = limit / lumi * 10000. / 1000.
  print index, "XSEC_x_Br_limit = ", XSEC_x_Br_limit, "pb"
  os.chdir( "/beegfs/lfi.mipt.su/scratch/shta/" )

  return XSEC_x_Br_limit
  
with open("/nfs/lfi.mipt.su/user/s/shirshov/MadGraph/output.json") as f:
  res = json.load(f)
  for k, v in res.items():
    if k.startswith("SM"):
      continue
    eff = v["result"]
    err = max(-v["Error %"][0], v["Error %"][1])
    err *= eff
    print int(k), eff, err
