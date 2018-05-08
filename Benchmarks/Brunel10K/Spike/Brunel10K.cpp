// Brunel 10,000 Neuron Network with Plasticity
// Author: Nasir Ahmad (Created: 03/05/2018)


/*
	This network has been created to benchmark Spike

	Publications:

*/


#include "Spike/Models/SpikingModel.hpp"
#include "Spike/Simulator/Simulator.hpp"
#include "Spike/Neurons/LIFSpikingNeurons.hpp"
#include "Spike/Neurons/PoissonInputSpikingNeurons.hpp"
#include "Spike/Synapses/VoltageSpikingSynapses.hpp"
#include "Spike/Plasticity/VogelsSTDPPlasticity.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <iomanip>
#include <vector>

int main (int argc, char *argv[]){
	// Getting options:
	float simtime = 20.0;
	bool fast = false;
	int num_timesteps_min_delay = 1;
	int num_timesteps_max_delay = 1;
	const char* const short_opts = "";
	const option long_opts[] = {
		{"simtime", 1, nullptr, 0},
		{"fast", 0, nullptr, 1},
		{"num_timesteps_min_delay", 1, nullptr, 2},
		{"num_timesteps_max_delay", 1, nullptr, 3}
	};
	// Check the set of options
	while (true) {
		const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

		// If none
		if (-1 == opt) break;

		switch (opt){
			case 0:
				printf("Running with a simulation time of: %ss\n", optarg);
				simtime = std::stof(optarg);
				break;
			case 1:
				printf("Running in fast mode (no spike collection)\n");
				fast = true;
				break;
			case 2:
				printf("Running with minimum delay: %s timesteps\n", optarg);
				num_timesteps_min_delay = std::stoi(optarg);
				if (num_timesteps_max_delay < num_timesteps_min_delay)
					num_timesteps_max_delay = num_timesteps_min_delay;
				break;
			case 3:
				printf("Running with maximum delay: %s timesteps\n", optarg);
				num_timesteps_max_delay = std::stoi(optarg);
				if (num_timesteps_max_delay < num_timesteps_min_delay){
					std::cerr << "ERROR: Max timestep shouldn't be smaller than min!" << endl;
					exit(1);
				}	
				break;
		}
	};
	
	// TIMESTEP MUST BE SET BEFORE DATA IS IMPORTED. USED FOR ROUNDING.
	// The details below shall be used in a SpikingModel
	SpikingModel * BenchModel = new SpikingModel();
	float timestep = 0.0001f; // 50us for now
	BenchModel->SetTimestep(timestep);

	// Create neuron, synapse and stdp types for this model
	LIFSpikingNeurons * lif_spiking_neurons = new LIFSpikingNeurons();
	PoissonInputSpikingNeurons * poisson_input_spiking_neurons = new PoissonInputSpikingNeurons();
	VoltageSpikingSynapses * voltage_spiking_synapses = new VoltageSpikingSynapses();
	// Add my populations to the SpikingModel
	BenchModel->spiking_neurons = lif_spiking_neurons;
	BenchModel->input_spiking_neurons = poisson_input_spiking_neurons;
	BenchModel->spiking_synapses = voltage_spiking_synapses;

	// Set up Neuron Parameters
	lif_spiking_neuron_parameters_struct * EXC_NEURON_PARAMS = new lif_spiking_neuron_parameters_struct();
	lif_spiking_neuron_parameters_struct * INH_NEURON_PARAMS = new lif_spiking_neuron_parameters_struct();

	EXC_NEURON_PARAMS->somatic_capacitance_Cm = 200.0f*pow(10.0, -12);  // pF
	INH_NEURON_PARAMS->somatic_capacitance_Cm = 200.0f*pow(10.0, -12);  // pF

	EXC_NEURON_PARAMS->somatic_leakage_conductance_g0 = 10.0f*pow(10.0, -9);  // nS
	INH_NEURON_PARAMS->somatic_leakage_conductance_g0 = 10.0f*pow(10.0, -9);  // nS

	EXC_NEURON_PARAMS->resting_potential_v0 = 0.0f*pow(10.0, -3);	// -74mV
	INH_NEURON_PARAMS->resting_potential_v0 = 0.0f*pow(10.0, -3);	// -82mV

	EXC_NEURON_PARAMS->absolute_refractory_period = 2.0f*pow(10, -3);  // ms
	INH_NEURON_PARAMS->absolute_refractory_period = 2.0f*pow(10, -3);  // ms

	EXC_NEURON_PARAMS->threshold_for_action_potential_spike = 20.0f*pow(10.0, -3);
	INH_NEURON_PARAMS->threshold_for_action_potential_spike = 20.0f*pow(10.0, -3);

	EXC_NEURON_PARAMS->background_current = 0.0f*pow(10.0, -2); //
	INH_NEURON_PARAMS->background_current = 0.0f*pow(10.0, -2); //

	/*
		Setting up INPUT NEURONS
	*/
	// Creating an input neuron parameter structure
	poisson_input_spiking_neuron_parameters_struct* input_neuron_params = new poisson_input_spiking_neuron_parameters_struct();
	// Setting the dimensions of the input neuron layer
	input_neuron_params->group_shape[0] = 1;		// x-dimension of the input neuron layer
	input_neuron_params->group_shape[1] = 10000;		// y-dimension of the input neuron layer
	input_neuron_params->rate = 20.0f; // Hz
	int input_layer_ID = BenchModel->AddInputNeuronGroup(input_neuron_params);
	poisson_input_spiking_neurons->set_up_rates();

	/*
		Setting up NEURON POPULATION
	*/
	vector<int> EXCITATORY_NEURONS;
	vector<int> INHIBITORY_NEURONS;
	// Creating a single exc and inh population for now
	EXC_NEURON_PARAMS->group_shape[0] = 1;
	EXC_NEURON_PARAMS->group_shape[1] = 8000;
	INH_NEURON_PARAMS->group_shape[0] = 1;
	INH_NEURON_PARAMS->group_shape[1] = 2000;
	EXCITATORY_NEURONS.push_back(BenchModel->AddNeuronGroup(EXC_NEURON_PARAMS));
	INHIBITORY_NEURONS.push_back(BenchModel->AddNeuronGroup(INH_NEURON_PARAMS));

	/*
		Setting up SYNAPSES
	*/
	voltage_spiking_synapse_parameters_struct * EXC_OUT_SYN_PARAMS = new voltage_spiking_synapse_parameters_struct();
	voltage_spiking_synapse_parameters_struct * INH_OUT_SYN_PARAMS = new voltage_spiking_synapse_parameters_struct();
	voltage_spiking_synapse_parameters_struct * INPUT_SYN_PARAMS = new voltage_spiking_synapse_parameters_struct();
	// Setting delays
	EXC_OUT_SYN_PARAMS->delay_range[0] = num_timesteps_min_delay*timestep;
	EXC_OUT_SYN_PARAMS->delay_range[1] = num_timesteps_max_delay*timestep;
	INH_OUT_SYN_PARAMS->delay_range[0] = num_timesteps_min_delay*timestep;
	INH_OUT_SYN_PARAMS->delay_range[1] = num_timesteps_max_delay*timestep;
	INPUT_SYN_PARAMS->delay_range[0] = num_timesteps_min_delay*timestep;
	INPUT_SYN_PARAMS->delay_range[1] = num_timesteps_max_delay*timestep;
	// Set Weight Range (in mVs)
  float weight_multiplier = 0.02*pow(10.0, -3);
  float weight_val = 0.1f;
  float gamma = 5.0f;
	EXC_OUT_SYN_PARAMS->weight_range_bottom = weight_val * weight_multiplier;
	EXC_OUT_SYN_PARAMS->weight_range_top = weight_val * weight_multiplier;
	INH_OUT_SYN_PARAMS->weight_range_bottom = -gamma * weight_val * weight_multiplier;
	INH_OUT_SYN_PARAMS->weight_range_top = -gamma * weight_val * weight_multiplier;
	INPUT_SYN_PARAMS->weight_range_bottom = weight_val * weight_multiplier;
	INPUT_SYN_PARAMS->weight_range_top = weight_val * weight_multiplier;

	// Biological Scaling factors (ensures that voltage is in mV)
	//EXC_OUT_SYN_PARAMS->biological_conductance_scaling_constant_lambda = 1.0f*pow(10.0,-3);
	//INH_OUT_SYN_PARAMS->biological_conductance_scaling_constant_lambda = 1.0f*pow(10.0,-3);
	//INPUT_SYN_PARAMS->biological_conductance_scaling_constant_lambda = 1.0f*pow(10.0,-3);

	// Creating Synapse Populations
	EXC_OUT_SYN_PARAMS->connectivity_type = CONNECTIVITY_TYPE_RANDOM;
	INH_OUT_SYN_PARAMS->connectivity_type = CONNECTIVITY_TYPE_RANDOM;
	INPUT_SYN_PARAMS->connectivity_type = CONNECTIVITY_TYPE_RANDOM;
	EXC_OUT_SYN_PARAMS->plasticity_vec.push_back(nullptr);
	INH_OUT_SYN_PARAMS->plasticity_vec.push_back(nullptr);
	INPUT_SYN_PARAMS->plasticity_vec.push_back(nullptr);
	EXC_OUT_SYN_PARAMS->random_connectivity_probability = 0.1; // 2%
	INH_OUT_SYN_PARAMS->random_connectivity_probability = 0.1; // 2%
	INPUT_SYN_PARAMS->random_connectivity_probability = 0.1; // 1%

	// Connect all of the populations
	BenchModel->AddSynapseGroup(EXCITATORY_NEURONS[0], EXCITATORY_NEURONS[0], EXC_OUT_SYN_PARAMS);
	BenchModel->AddSynapseGroup(EXCITATORY_NEURONS[0], INHIBITORY_NEURONS[0], EXC_OUT_SYN_PARAMS);
	BenchModel->AddSynapseGroup(INHIBITORY_NEURONS[0], EXCITATORY_NEURONS[0], INH_OUT_SYN_PARAMS);
	BenchModel->AddSynapseGroup(INHIBITORY_NEURONS[0], INHIBITORY_NEURONS[0], INH_OUT_SYN_PARAMS);
	BenchModel->AddSynapseGroup(input_layer_ID, EXCITATORY_NEURONS[0], INPUT_SYN_PARAMS);
	BenchModel->AddSynapseGroup(input_layer_ID, INHIBITORY_NEURONS[0], INPUT_SYN_PARAMS);

	/*
		COMPLETE NETWORK SETUP
	*/
	BenchModel->finalise_model();


	// Create the simulator options
	Simulator_Options* simoptions = new Simulator_Options();
	simoptions->run_simulation_general_options->presentation_time_per_stimulus_per_epoch = simtime;
	if (!fast){
		simoptions->recording_electrodes_options->collect_neuron_spikes_recording_electrodes_bool = true;
		simoptions->file_storage_options->save_recorded_neuron_spikes_to_file = true;
		//simoptions->recording_electrodes_options->collect_neuron_spikes_optional_parameters->human_readable_storage = true;
		simoptions->recording_electrodes_options->collect_input_neuron_spikes_recording_electrodes_bool = true;
	}


	Simulator * simulator = new Simulator(BenchModel, simoptions);
	clock_t starttime = clock();
	simulator->RunSimulation();
	clock_t totaltime = clock() - starttime;
	if ( fast ){
		std::ofstream timefile;
		timefile.open("timefile.dat");
		timefile << std::setprecision(10) << ((float)totaltime / CLOCKS_PER_SEC);
		timefile.close();
	}
	//cudaProfilerStop();
	return(0);
}
