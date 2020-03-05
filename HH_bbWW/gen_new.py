from run import *


def BSM2(i):
    return {
        "data": "/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/HH_bbWW_samples_2/delphes_{0}.root".format(i),
        "events": "/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/HH_bbWW_samples_2/GF_HH_{0}_cmsgrid_final_decayed.lhe".format(i)
    }


if __name__ == "__main__":
    try:
        sm = []
        bsm = []
        bsm2 = []

        for i in range(1, 3):
            sm.append(run(SM(i)))
            bsm.append(run(BSM(i)))
            bsm2.append(run(BSM2(i)))
            print()

        for i in range(3, 13):
            # sm.append(run(SM(i)))
            bsm.append(run(BSM(i)))
            bsm2.append(run(BSM2(i)))
            print()
        
        m = lambda x: x["Non-resonant criteria"]

        print("SM: ", list(map(m, sm)))
        print("BSM: ", list(map(m, bsm)))
        print("BSM2: ", list(map(m, bsm2)))
    finally:
        with open("gen_new_output.json", "w") as output:
            json.dump({
                "sm": sm,
                "bsm": bsm,
                "bsm2": bsm2
            }, output, indent=4)
    



