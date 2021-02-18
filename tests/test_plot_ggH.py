#!/usr/bin/env python3

import sys
import ROOT

# TODO check ROOT file structure, cp from tutorial for now
f = ROOT.TFile.Open('hist_ggH.root')
ks = [k.GetName() for k in f.GetListOfKeys()]

req_ks = ['ggH_pt_1', 'ggH_pt_2']

print('\n'.join(ks))
for k in req_ks:
    if not k in ks:
        print(f'Error: Required key not foun in ROOT file! {k}')
        sys.exit(1)


integral = f.ggH_pt_1.Integral()
if abs(integral - 222.88716647028923) > 0.0001:
    print(f'Error: Integral of ggH_pt_1 is different: {integral}')
    sys.exit(1)
