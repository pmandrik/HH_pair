
path_lhe="../../../HH_lhe_samples"
path="../../../test_yyWW_cms_EFT_12"

# path="../../../test_yyWW_cms_EFT_12"
# input_file_delphes=$path"/delphes_2.root"
wdir=`pwd`
adir="cms_results"
mkdir $adir

run_anal(){
  postfix=$1
  input_file_delphes=$2
  input_file_lhe=$3

  root -l -b -q "../make_interface.C(\""$input_file_delphes"\")"
  python ../turn_off_unused_branches.py
  root -l -b -q "analyser_yyWW_CMS.C(\""$input_file_delphes"\", \""$input_file_lhe"\", true)" > $adir/"log_"$postfix".log"
}

for index in 1 2 3 4 5 6 7 8 9 10 11 12; do
  run_anal "EFT_"$index $path"/delphes_"$index".root" $path_lhe"/GF_HH_"$index"_cmsgrid_final.lhe"
done

run_anal "SM" "../../../test_yyWW_cms_SM/delphes_sm.root" $path_lhe/"GF_HH_1_cmsgrid_final.lhe"
