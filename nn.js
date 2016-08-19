// Simple Javascript Nural Network with no feedback system
"use strict";

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define an input as a simple Neuron which just copies the input to the output.  Just makes the Neuron class easier to write
function inputNeuron() {
	this.value = 0;
}

// Set the input value
inputNeuron.prototype.setInput = function(value) {
	this.value = value;
}

// Just copy it through
inputNeuron.prototype.getOutput = function() {
	return this.value;
}	



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define a neuron
function Neuron() {
	this.input = [{neuron: 0, weight: 1}];
	this.output = 0;
}
	
// Add an input as a neuron
Neuron.prototype.addNeuronInput = function(neuron) {
	this.input.push({neuron: neuron, weight: 1});
}

// Calculate the output
Neuron.prototype.calculate = function() {
	this.output = 0;
	var len = this.input.length;
	for (var a=1; a<len; a++)
		this.output+=this.input[a].weight * this.input[a].neuron.getOutput();
	this.output*=this.input[0].weight;
	this.output = this.sigmoid(this.output);
}

// Basic sigmoid activation function 
Neuron.prototype.sigmoid = function(t) {
	return 1/(1+Math.pow(Math.E, -t));
}

// Return the current output	
Neuron.prototype.getOutput = function() {
	return this.output;
}	

// Randomizes the input weights
Neuron.prototype.randomizeWeights = function() {
	var len = this.input.length;
	for (var a=0; a<len; a++)
		this.input[a].weight = Math.random();
}

// Returns the current weights
Neuron.prototype.getWeights = function() {
	var ret = [];
	var len = this.input.length;
	for (var a=0; a<len; a++)
		ret.push(this.input[a].weight);
	return ret;
}

// Sets the current weights
Neuron.prototype.setWeights = function(weights, startPosition) {
	this.output = 0;
	var len = this.input.length;
	for (var a=0; a<len; a++) 
		this.input[a].weight = weights[startPosition++]
	return startPosition;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Creates a network.  networkLayers is an array of network level sizes
function NeuralNetwork(networkLayers) {
	this.network = [];
	this.fitness = 0;
	this.networkLayers = networkLayers;
	this.init();
}

	
// Reset the network
NeuralNetwork.prototype.init = function() {
	// Prepare levels	
	for (var layr=0; layr<this.networkLayers.length; layr++)  this.network.push([]);
		
	for (var layr=0; layr<this.networkLayers.length; layr++) {
		// Create the neurons and build the network
		for (var n=0; n<this.networkLayers[layr]; n++) {
			var neuron;  
			if (layr==0) 
				neuron = new inputNeuron();
			else {
				neuron = new Neuron();				
				for (var nn=0; nn<this.networkLayers[layr-1]; nn++)
					neuron.addNeuronInput(this.network[layr-1][nn]);
			}
			this.network[layr].push(neuron);
		}
	}
	
	// Create intial crazy random net
	this.resetRandom();
}

// Set the input value
NeuralNetwork.prototype.setInputValue = function(index, value) {
	this.network[0][index].setInput(value);
}

// Creates a random network
NeuralNetwork.prototype.resetRandom = function() {
	// Level 0 is the input, nothing can be randomized here
	for (var a=1; a<this.network.length; a++)
		for (var b=0; b<this.network[a].length; b++)
			this.network[a][b].randomizeWeights();
}

// Returns all of the weight values used by this network
NeuralNetwork.prototype.getWeights = function() {
	var ret = [];
	for (var a=1; a<this.network.length; a++)
		for (var b=0; b<this.network[a].length; b++) {
			var w = this.network[a][b].getWeights();
			for (var g=0; g<w.length; g++) ret.push(w[g]);
		}
	return ret;
}

// Sets the weights on the network
NeuralNetwork.prototype.setWeights = function(weights) {
	var pos = 0;
	for (var a=1; a<this.network.length; a++)
		for (var b=0; b<this.network[a].length; b++) 
			pos=this.network[a][b].setWeights(weights,pos);
}

// Calculates the latest output
NeuralNetwork.prototype.calculate = function() {
	var len = this.network.length;
	for (var a=1; a<len; a++) {
		var len2 = this.network[a].length;
		for (var b=0; b<len2; b++) 
			this.network[a][b].calculate();
	}
}

// Set the fitness this achieved
NeuralNetwork.prototype.setFitness = function(fitness) {
	this.fitness = fitness;
}

// Returns an output value
NeuralNetwork.prototype.getOutputValue = function(index) {
	return this.network[this.network.length-1][index].getOutput();
}

// Returns a fitness value
NeuralNetwork.prototype.getFitness = function() {
	return this.fitness;
}

// Returns the number of inputs
NeuralNetwork.prototype.numInputs = function() {
	return this.network[0].length;
}

// Returns the number of outputs
NeuralNetwork.prototype.numOutputs = function() {
	return this.network[this.network.length-1].length;
}

// Returns all the outputs
NeuralNetwork.prototype.getAllOutputs = function() {
	var ret = [];
	for (var a=0; a<this.network[this.network.length-1].length; a++)
		ret.push(this.network[this.network.length-1][a].getOutput());
	return ret;
}

// Set all inputs
NeuralNetwork.prototype.setAllInputs = function(inputs) {
	for (var a=0; a<this.network[0].length; a++)
		this.network[0].setInput(inputs[a]);
}

