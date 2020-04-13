
path_lhe="../../../HH_lhe_samples"
input_file_lhe=$path_lhe"/GF_HH_2_cmsgrid_final.lhe"

path="../../../test_yyWW_cms_SM"
input_file_delphes=$path"/delphes_sm.root"


root -l -b -q "../make_interface.C(\""$input_file_delphes"\")"
python ../turn_off_unused_branches.py
root -l -b -q "analyser_yyWW_ATLAS.C(\""$input_file_delphes"\", \""$input_file_lhe"\", true)"
