


path="../../../test_bbWW"
input_file_delphes=$path"/delphes_1.root"

#path="../../../test_bbWW_fix"
#input_file_delphes=$path"/delphes_1_fix.root"

path_lhe="../../../HH_lhe_samples"
input_file_lhe=$path_lhe"/GF_HH_1_cmsgrid_final.lhe"

root -l -b -q "../make_interface.C(\""$input_file_delphes"\")"
python ../turn_off_unused_branches.py
root -l -b -q "analyser_WWbb.C(\""$input_file_delphes"\", \""$input_file_lhe"\")"
