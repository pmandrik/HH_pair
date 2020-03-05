


path=/home/pmandrik/work/analyses/3_FCNC_HH_anomal/test_bbWW/
path_lhe=/home/pmandrik/work/analyses/3_FCNC_HH_anomal/HH_lhe_samples/

input_file_delphes=$path"/delphes_1.root"
input_file_lhe=$path_lhe"/GF_HH_1_cmsgrid_final.lhe"

path=/home/pmandrik/work/analyses/3_FCNC_HH_anomal/anom_couplings/generate/bbWW_0/card_0/HH_bbWW/Events/run_01/
path_lhe=/home/pmandrik/work/analyses/3_FCNC_HH_anomal/anom_couplings/generate/bbWW_0/card_0/HH_bbWW/Events/run_01/

input_file_delphes=$path"/tag_1_delphes_events.root"
input_file_lhe=$path_lhe"/unweighted_events.lhe"

root -l -b -q "make_interface.C(\""$input_file_delphes"\")"
python turn_off_unused_branches.py
root -l -b -q "analyser_WWbb.C(\""$input_file_delphes"\", \""$input_file_lhe"\")"
