# SNNSimulatorComparison
Comparison of Spiking Neural Network Simulator Performance

__Note: Tests are still being conducted on the benchmark model for each simulator and the results may change as the models are corrected__

Simulators are included as git submodules.
Currently the simulators being compared are:
- [NEST Simulator (using pyNEST)](https://github.com/nest/nest-simulator/)
- [Brian Simulator (v2)](https://github.com/brian-team/brian2)
- [Auryn](https://github.com/fzenke/auryn)
- [ANNarchy](https://github.com/ANNarchy/ANNarchy)
- [Spike](https://github.com/nasiryahm/Spike)
- [Brian2 with GeNN (General Neural Network Simulator)](https://github.com/brian-team/brian2genn)

## Results
### Vogels Abbott Benchmark
Simulations were all single-threaded and run on machine with:
CPU: Intel i7-4770K
GPU: NVIDIA GTX 1070 founders edition
OS: Ubuntu 16.04 LTS

All simulations load the same set of weights (ee/ei/ie/ii.wmat located in the benchmark folder) and therefore allow a direct simulation comparison. This benchmark simulation was inspired by the benchmarks used for the [Auryn](https://github.com/fzenke/auryn) simulator. See their github repository for further details.

The installed python (conda) environment details are located in the conda\_req.txt file.

Results of a simulation of the VogelsAbbott Benchmark with ALL synaptic delays set to 0.1ms
![0.1ms Delay Figure](Benchmarks/VogelsAbbott/_results/single_delay_comparison.png)

Results of a simulation of the VogelsAbbott Benchmark with ALL synaptic delays set to 0.8ms
![0.8ms Delay Figure](Benchmarks/VogelsAbbott/_results/eight_delay_comparison.png)

A comparison of the ISI distributions, firing rasters, and firing rates is present in an [iPython notebook](Benchmarks/VogelsAbbott/_results/SimulatorComparisons.ipynb). These results were produced from files which are automatically dumped when the "--fast" option is not used in simulation execution.


## Getting all submodules:
In order to clone this repository with all submodules:
```
git clone --recurse-submodules -j8 https://github.com/nasiryahm/SNNSimulatorComparison.git
```

## Installation
Spike, Auryn and Nest are auto compiled (using make). Ensure that the dependencies for these libraries are pre-installed. To see these, please visit the github pages for these projects.

ANNarchy and Brian can be installed using `pip install Simulators/ANNarchy/` and `pip install Simulators/brian2` from this location. Again, ensure that the dependencies are pre-installed.
The dependencies and packages required for the installation of these two packages can be found in the requirements.txt file in the root of this repo.

Brian with GeNN must be installed separately. If using conda, `conda install -c brian-team brian2genn`. Otherwise, you must set the `CUDA\_PATH` and `GENN\_PATH` variables and add `${GENN\_PATH}/lib/bin` to `PATH`. Following this, it can be installed as above with pip: 'pip install Simulators/brian2genn'

## Running a speed test
"run\_benchmark.sh" is a bash script that outlines the simulation length and delay style to simulate in each simulator. Edit this to run a given style of benchmark (at the moment VogelsAbbott benchmark only). This will produce a timefile.dat file in each folder under "Benchmarks/VogelsAbbott/SIMULATOR". This timefile.dat contains a single ASCII float which is the time required to run the simulation (simulation run time only, not for the rest of the script). The "Benchmarks/VogelsAbbott/\_results" folder contains a plotting.py file with the matplotlib commands used to produce the results plots.

## Testing ranges of delays:
Spike, Brian2, and Nest support ranges of delays. ANNarchy informs us that it can handle uniform delays (experimentally) though does not compile in this case.

Auryn and Brian2GeNN do not seem to support ranges of delays.
