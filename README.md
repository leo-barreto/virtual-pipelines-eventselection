# Example analysis for the analysis preservation bootcamp

The example is an educational version of a Higgs to two tau lepton analysis based on NanoAOD-like samples from the CERN Open Data portal. To fit the computational effort of the analysis in the scope of the workshop, the samples are reduced beforehand. See the documentation below for the single steps.

Features:
- The analysis has the common steps of skimming, histogramming and plotting/fitting
- The skimming and histogramming workflow is parallelized on file basis and all results are just merged for the plotting or fitting.
- No experiment specific software needed, no experiment policies attached to the data or code.
- Minimal software dependencies (ROOT and Python), running on all major CERN infrastructure (lxplus, SWAN, all CVMFS enabled machines)
- The analysis workflow is scripted in bash so we can transfer this during the workshop to any workflow management software such as ReANA.

## Links

Indico agenda of the event: https://indico.cern.ch/event/854880/
Record on the CERN Open Data portal used as baseline: http://opendata.web.cern.ch/record/12350

## Dependencies

The analysis needs only ROOT (6.16 or later) and Python (2 and 3 should work). You can run on any CVMFS enabled machine sourcing the following LCG release (don't forget to select the correct platform):

```bash
source /cvmfs/sft.cern.ch/lcg/views/LCG_95/x86_64-slc6-gcc8-opt/setup.sh
```

## Preprocessing: Reducing the initial samples

To reduce the inital samples to a fraction of the size. The bash script `reduce.sh` calls the Python script `reduce.py` for all relevant samples with a constant reduction factor.

## Step 1: Skimming

The first analysis step skims the NanoAOD-like samples with a baseline selection and finds valid muon-tau pairs. The output is written as a flat ntuple for further processing. Run `skim.sh /path/to/dir/with/samples` to skim all (reduced) samples.

## Step 2: Histograms

Next, we make histograms of all variables and physics processes for later plotting. Call `histograms.sh /path/to/dir/with/skims` to run the workflow.

## Step 3: Plotting

Finally, we make the physics results by combining the histograms. Run `python plot.py /path/to/dir/with/histograms` for this step.

## Step 4: Fit
Optionally, we can fit the cross-section of any process using the histograms also used for plotting.

TODO: Add the fitting script
