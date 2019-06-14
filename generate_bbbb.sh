
set +x

SEED=10
NEVENTS=100
process="bbbb"

mg_dir=/home/pmandrik/soft/MG5_aMC_v2_6_1/
mg_dir=/home/pmandrik/soft/MG5_aMC_v2_6_3_2/

sdir=`pwd`
tag="_0"
wdir=$process$tag
mkdir -p $wdir; cd $wdir

run_mg () {
  card_index=$1
  mkdir "card_"$card_index; cd "card_"$card_index
  
  #cp $sdir/customizecards_$card_index.dat customizecards.dat
  cp $sdir/$process"_proc_card_"$card_index".dat" proc_card.dat

  echo "WORKDIR GENERATION ++++++++++++++++++++++++++++++++ ++++++++++++++++++++++++++++++++ ++++++++++++++++++++++++++++++++"
  $mg_dir/bin/mg5_aMC proc_card.dat

  echo "EVENTS GENERATION ++++++++++++++++++++++++++++++++ ++++++++++++++++++++++++++++++++ ++++++++++++++++++++++++++++++++"
  cp $sdir/$process"_run_card.dat"  "HH_"$process/Cards/run_card.dat
  sed -i 's/%SEED%/'$SEED'/g'       "HH_"$process/Cards/run_card.dat
  sed -i 's/%NEVENTS%/'$NEVENTS'/g' "HH_"$process/Cards/run_card.dat

  cp $sdir/$process"_pythia_card.dat"  "HH_"$process/Cards/pythia_card.dat
  cp $sdir/$process"_delphes_card.dat" "HH_"$process/Cards/delphes_card.dat

  ls -lat "HH_"$process/Cards/*

  echo "run_mode = 2" >> "HH_"$process/Cards/me5_configuration.txt
  echo "nb_core = 2"  >> "HH_"$process/Cards/me5_configuration.txt
  echo "automatic_html_opening = False"  >> "HH_"$process/Cards/me5_configuration.txt

  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/pmandrik/soft/MG5_aMC_v2_6_3_2/HEPTools/lhapdf6/lib/
  LHAPDFCONFIG="/home/pmandrik/soft/MG5_aMC_v2_6_3_2/HEPTools/lhapdf6/bin/lhapdf-config"
  echo "pythia8_path = /home/pmandrik/soft/pythia8235 #  "  >> "HH_"$process/Cards/me5_configuration.txt
  echo "mg5amc_py8_interface_path = /home/pmandrik/soft/MG5_aMC_v2_6_3_2/HEPTools/MG5aMC_PY8_interface #  "  >> "HH_"$process/Cards/me5_configuration.txt
  echo "hepmc_path = /home/pmandrik/soft/MG5_aMC_v2_6_3_2/HEPTools/hepmc #  "  >> "HH_"$process/Cards/me5_configuration.txt
  echo "delphes_path = /home/pmandrik/soft/MG5_aMC_v2_6_3_2/Delphes #  "  >> "HH_"$process/Cards/me5_configuration.txt
  echo "lhapdf = $LHAPDFCONFIG #  "  >> "HH_"$process/Cards/me5_configuration.txt
  echo "cluster_local_path = `${LHAPDFCONFIG} --datadir`"  >> $PROCESS/Cards/me5_configuration.txt
  
  ls -lat "HH_"$process/Cards/*
  ./"HH_"$process/bin/generate_events $run_tag
  ls -lat "HH_"$process/Cards/*

  #cat "run_cmd.txt" | ./"HH_"$process/bin/madevent

  #echo "launch" > "run_cmd.txt"
  #cat "run_cmd.txt" | ./"HH_"$process/bin/madevent
  
}

if [ "$1" == "sm" ] ; then
  echo SM
  run_mg 0 
fi

if [ "$1" == "bsm" ] ; then
  echo BSM
fi



