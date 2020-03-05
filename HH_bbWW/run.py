from subprocess import Popen, PIPE
from math import sqrt
import json


def SM(x):
    return {
        "data": "/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/SM/tag_{0}_delphes_events.root".format(x),
        "events": "/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/SM/unweighted_events_{0}.lhe".format(x)
    }


def BSM(i):
    return {
        "data": "/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/test_bbWW/delphes_{0}.root".format(i),
        "events": "/beegfs/lfi.mipt.su/scratch/MadGraph/HH_bbWW/HH_lhe_samples/GF_HH_{0}_cmsgrid_final.lhe".format(i)
    }

def run(res, JES = 0, JER = 0):
    print("Processing {}".format(res["data"]))
    output, _ = Popen(
        ["root", "-l", "-b", "-q",
            "analyser_WWbb.C(\"{0}\", \"{1}\", {2}, {3})".format(
                res["data"],
                res["events"],
                JES, JER
            )
        ],
        stdout=PIPE,
        stderr=None
    ).communicate()
    output = output.decode('utf-8')
    output = output.split('\n')
    output = output[2:-1]

    for i in output:
        print(i)
        a, b = tuple(map(lambda x: x.strip(), i.split("=>")))
        n, b = b.split()
        if a == "Total":
            res[a] = int(n)
            continue
        b = float(b)
        b *= 0.766
        res[a] = b
    
    return res

def max_diff(seq, val):
    max_p, max_n = 0, 0

    for w in seq:
        d = w - val
        
        if d > max_p:
            max_p = d
        elif d < max_n:
            max_n = d
    
    return (max_n, max_p), (100 * max_n / val, 100 * max_p / val)


def analyse(inp):
    data = dict()

    es, er = [], []

    data = run(inp, 0, 0)
#    data = {
#        "data": "Data",
#        "events": "Events",
#        "Non-resonant criteria (weighted)": 3E-5,
#        "MUR": 0.9,
#        "MUF": 1.2,
#        "Total": 100000,
#        "PDF up": 1.2,
#        "PDF down": 0.95
#    }

    es.append(run(inp, 1, 0)["Non-resonant criteria (weighted)"])

    er.append(run(inp, 0, 1)["Non-resonant criteria (weighted)"])

    res = dict()

    res["data"] = data["data"]
    res["events"] = data["events"]
    w = data["Non-resonant criteria (weighted)"]
    res["result"] = w
    stat_err = 100 / sqrt(w * data["Total"])
    
    res["MUR MUF Error"], res["MUR MUF Error %"] = max_diff((i for k, i in data.items() if k.find("mu_r") != -1), w)

    res["PDF Error"] = (
        data["PDF down"] - w,
        data["PDF up"] - w
    )
    res["PDF Error %"] = (
        100 * (data["PDF down"] - w) / w,
        100 * (data["PDF up"] - w) / w
    )

    res["JES Error"], res["JES Error %"] = max_diff(es, w)
    res["JER Error"], res["JER Error %"] = max_diff(er, w)

    res["Stat Error %"] = (-stat_err, stat_err)

    e_n, e_p = 0, 0

    for i in ("MUR MUF Error %", "PDF Error %", "JES Error %", "JER Error %", "Stat Error %"):
        x = res[i]
        e_p += x[0] * x[0]
        e_n += x[1] * x[1]

    e_n, e_p = sqrt(e_n), -sqrt(e_p)

    res["Error %"] = (e_p, e_n)
    
    return res

def main():
    data = {}

    try:
        for i in range(1, 3):
            data["SM{}".format(i)] = analyse(SM(i))

        for i in range(1, 13):
            data[i] = analyse(BSM(i))
    finally:
        with open("output.json", "w") as output:
            json.dump(data, output, indent=4)

if __name__ == '__main__':
    main()
