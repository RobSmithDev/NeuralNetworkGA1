/*********************************************************************
 * Neural Network with Genetic Algorithms Demonstration              *
 * Copyright (C) 2002 RobSmithDev                                    *
 * https://robsmithdev.co.uk                                         *
 *                                                                   *
 * For more information about this project please see the video at:  *
 * https://www.youtube.com/watch?v=bq3FdlUeOTU                       *
 *********************************************************************/

#pragma once

#define _USE_MATH_DEFINES // for C++
#include <cmath>
#include <math.h>
#include <stdlib.h> 
#include <vector>

// A simple abstract class defining that "value" output must exist
class AbstractNeuronOutput {
public:
	virtual float value() abstract;
};

// A simple input neuron - does nothing.
class InputNeuron : public AbstractNeuronOutput {
private:
	float m_value = 0;
public:
	// Get value
	virtual float value() override { 
		return m_value; 
	};

	// Set value
	void setValue(const float value) { 
		m_value = value; 
	};
};

// Normal neuron
class Neuron : public AbstractNeuronOutput {
private:
	// Neuron weight list
	struct NeuronWeight {
		AbstractNeuronOutput* neuron = nullptr;
		float weight = 1;
	};

	// Input neurons
	std::vector<NeuronWeight> m_inputNeurons;

	// Current output weight
	float m_outputWeight = 1;

	// The current cached output value of the neuron
	float m_output = 0;
public:
	Neuron(const std::vector<AbstractNeuronOutput*> inputNeurons) {
		// Build up the neuron list
		for (AbstractNeuronOutput* neuron : inputNeurons)
			m_inputNeurons.push_back({ neuron, 1 });
	}

	// Sigmoid activation function
	static float Sigmoid(float input) {
		return 1.0f / (1.0f + (float)pow(M_E, -input));
	}

	// Get value
	virtual float value() override {
		return m_output;
	}

	// Update the neuron with the latest value
	void update() {
		m_output = 0;
		for (const NeuronWeight& neuronWeight : m_inputNeurons)
			m_output += neuronWeight.weight * neuronWeight.neuron->value();
		m_output = Neuron::Sigmoid(m_output);
	}

	// Randomize all the weights in the neuron
	void randomize() {
		m_outputWeight = (float)rand() / (float)RAND_MAX;
		for (NeuronWeight& neuronWeight : m_inputNeurons)
			neuronWeight.weight = (float)rand() / (float)RAND_MAX;
	}

	// Adds to the supplied vector all the weights in this neuron
	void getWeights(std::vector<float>& weights) const {
		weights.push_back(m_outputWeight);

		for (const NeuronWeight& neuronWeight : m_inputNeurons)
			weights.push_back(neuronWeight.weight);
	}

	// Set the weights, position is the start and end position (on exit)
	void setWeights(const std::vector<float> weights, size_t& position) {
		m_outputWeight = weights[position++];
		for (NeuronWeight& neuronWeight : m_inputNeurons)
			neuronWeight.weight = weights[position++];
	}
};

// Simple neural network with no feedback
class NeuralNetwork {
private:
	// The input neuron layer
	std::vector<InputNeuron*> m_inputNeurons;
	// All the subsequent layers
	std::vector<
		std::vector<Neuron*>
	> m_layers;

public:
	//  Rather than mess around, disable the copy methods
	NeuralNetwork(const NeuralNetwork&) = delete;
	NeuralNetwork& operator=(NeuralNetwork&) = delete;

	// Create a network.  Vector contains the number of neurons in each layer
	NeuralNetwork(std::vector<size_t> layerSizes) {
		// Add the inputs
		std::vector<AbstractNeuronOutput*> previousLayer;
		while (m_inputNeurons.size() < layerSizes[0]) {
			InputNeuron* neuron = new InputNeuron();
			m_inputNeurons.push_back(neuron);
			previousLayer.push_back(neuron);
		}
		
		// Now populate
		std::vector<AbstractNeuronOutput*> nextLayer;
		for (size_t layer = 1; layer < layerSizes.size(); layer++) {
			nextLayer.clear();
			std::vector<Neuron*> layerGenerated;

			// Build up the layer
			for (size_t count = 0; count < layerSizes[layer]; count++) {
				Neuron* neuron = new Neuron(previousLayer);
				layerGenerated.push_back(neuron);
				nextLayer.push_back(neuron);
			}

			// Save
			m_layers.push_back(layerGenerated);
			previousLayer = nextLayer;
		}
	}

	// Free up memory
	~NeuralNetwork() {
		for (InputNeuron* neuron : m_inputNeurons) delete neuron;
		for (std::vector<Neuron*>& layer : m_layers)
			for (Neuron* neuron : layer)
				delete neuron;
	}

	// Randomize all the weighting in the layer
	void randomize() {
		for (std::vector<Neuron*>& layer : m_layers)
			for (Neuron* neuron : layer)
				neuron->randomize();
	}

	// Set an inputs value
	void setInput(size_t inputNumber, const float value) {
		m_inputNeurons[inputNumber]->setValue(value);
	}

	// Get all weights that make up this network
	void getWeights(std::vector<float>& weights) const {
		for (const std::vector<Neuron*>& layer : m_layers)
			for (Neuron* neuron : layer)
				neuron->getWeights(weights);
	}

	// Set all weights that make up this network.  Returns the weights used
	size_t setWeights(const std::vector<float>& weights) {
		size_t position = 0;

		for (std::vector<Neuron*>& layer : m_layers)
			for (Neuron* neuron : layer)
				neuron->setWeights(weights, position);
		return position;
	}

	// Get the output from a specific neuron
	float value(size_t outputNeuron) const {
		return m_layers.back().at(outputNeuron)->value();
	}

	// Calculates the latest output from the network
	void update() {
		for (std::vector<Neuron*>& layer : m_layers)
			for (Neuron* neuron : layer)
				neuron->update();		
	}
};