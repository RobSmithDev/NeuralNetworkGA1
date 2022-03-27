/*********************************************************************
 * Neural Network with Genetic Algorithms Demonstration              *
 * Copyright (C) 2002 RobSmithDev                                    *
 * https://robsmithdev.co.uk                                         *
 *                                                                   *
 * For more information about this project please see the video at:  *
 * https://www.youtube.com/watch?v=bq3FdlUeOTU                       *
 *********************************************************************/

#pragma once

#include <algorithm>
#include "NeuralNetwork.h"

// A structure to hold the fitness for a network, and the weights that create that network
struct NetworkWeightFitness {
	NeuralNetwork* network;
	float fitness;
};

// Genetic algorithm main class - implements a basic genetic algorithm.
class GeneticAlgorithm {
	size_t m_numBest = 1;             // The best numBest networks will automatically be output into the next generation as they currently are
	float m_crossOverRate = 0.7f;     // The % of children, that are NOT just a clone of a parent
	float m_mutationRate = 0.1f;      // The % of chomosomes (weights) that will get mutated
	float m_mutationAmount = 0.3f;    // The amount of mutation that may be applied to a weight
	int m_mutationRateForRandom = 0;  // Is calculated as m_mutationRate * RAND_MAX

	// Picks a random parent randomly, but bias slightly based on their fitness
	const NetworkWeightFitness* pickParentByRoulette(const std::vector< NetworkWeightFitness >& parents, const float totalFitness) const {
		float randValue = (float)rand() * totalFitness / (float)RAND_MAX;
		float soFar = 0;

		// Search for the correct one
		for (size_t position = 0; position < parents.size(); position++) {
			soFar += parents[position].fitness;
			if (soFar >= randValue) {				
				return &parents[position];
			}
		}

		// If we get here and didn't find one, then return the fittest - this shouldnt really happen
		return &parents[parents.size()-1];
	}

	// Mutate the supplied weights
	void mutateWeights(std::vector<float>& weights) const {
		for (float& weight : weights)
			if (rand() < m_mutationRateForRandom) // Should we mutate?
				weight += m_mutationAmount * (((rand() * 2.0f) / (float)RAND_MAX) - 1.0f);
	}

	// Create a child from the supplied parents
	void reproduce(const std::vector<float>& parent1Weights, const std::vector<float>& parent2Weights, std::vector<float>& child1Weights, std::vector<float>& child2Weights) const {
		const size_t point = (rand() * parent1Weights.size()) / RAND_MAX;
		for (size_t weight = 0; weight < parent1Weights.size(); weight++) {
			if (weight < point) {
				child1Weights.push_back(parent1Weights[weight]);
				child2Weights.push_back(parent2Weights[weight]);
			}
			else {
				child1Weights.push_back(parent2Weights[weight]);
				child2Weights.push_back(parent1Weights[weight]);
			}
		}
	}

public:
	// Create the genetic algorithm mutation class
	GeneticAlgorithm(const size_t numBest = 1, const float crossOverRate = 0.7f, const float mutationRate = 0.1f, const float mutationAmount = 0.3f)
		: m_numBest(numBest), m_crossOverRate(crossOverRate), m_mutationRate(mutationRate), m_mutationAmount(mutationAmount) {
		m_mutationRateForRandom = (int)(m_mutationRate * (float)RAND_MAX);
	}

	// Takes in the weights and fitness for the current generation, and produces the next one
	void produceNextGeneration(std::vector< NetworkWeightFitness >& generation) const {
		// For the next generation we only need to know their 'brain' - eg the neuron weights
		std::vector<std::vector<float>> nextGeneration;

		// Step 1: Calculate the total fitness of the entire previous generation
		float totalFitness = 0;
		for (const NetworkWeightFitness& network : generation)
			totalFitness += network.fitness;

		// Step 2: Sort the previous generation in worse-to-best order
		std::sort(generation.begin(), generation.end(), [this](const NetworkWeightFitness& a, const NetworkWeightFitness& b) -> bool {
			return a.fitness < b.fitness;
		});

		// Step 3: Output the ones that were best on the previous generation
		for (size_t count = 1; count <= m_numBest; count++) {
			std::vector<float> weights;
			generation[generation.size() - count].network->getWeights(weights);
			nextGeneration.push_back(weights);
		}

		// Step 4: Now we produce the remainder of the new generation by mutating the existing one
		while (nextGeneration.size() < generation.size()) {
			// First, pick two semi-random parents from which to create a child
			const NetworkWeightFitness* parent1 = pickParentByRoulette(generation, totalFitness);
			const NetworkWeightFitness* parent2 = pickParentByRoulette(generation, totalFitness);

			// Produce a child by mixing the weights of the parents at a specific point
			std::vector<float> parent1Weights;
			std::vector<float> parent2Weights;
			std::vector<float> child1Weights, child2Weights;

			// Extract the 'genome'/'dna' weights from the parents
			parent1->network->getWeights(parent1Weights);
			parent2->network->getWeights(parent2Weights);

			// Mix up.  This is a simple cross-over function. There are more complex ways to do this
			reproduce(parent1Weights, parent2Weights, child1Weights, child2Weights);

			// And now mutate the child
			mutateWeights(child1Weights);
			mutateWeights(child2Weights);

			// It's the new generation
			nextGeneration.push_back(child1Weights);
			nextGeneration.push_back(child2Weights);
		}

		// Step 5: Re-program the input with the new 'brains'
		for (size_t network = 0; network < generation.size(); network++)
			generation[network].network->setWeights(nextGeneration[network]);
	}
};