// Simple Javascript Neural Network with no feedback system

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define an input as a simple Neuron which just copies the input to the output.  Just makes the Neuron class easier to write
class InputNeuron {
	constructor () {
		this._value = 0;
	}
	
	// Set the value of the input neuron
	set value(value) {
		this._value = value; 
	}

	// Return the current value of the input neuron
	get value() {
		return this._value;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define a single neuron
class Neuron {
	// A list of input neurons are passed in the constructor
	constructor (inputNeurons) {
		this._input = [];
		this._outputWeight = 1;
		this._output = 0;

		// Populate the list
		for (let neuron of inputNeurons)
			this._input.push( { neuron: neuron, weight: 1});
	}

	
	// Basic sigmoid activation function 
	static Sigmoid = function(t) {
		return 1/(1+Math.pow(Math.E, -t));
	}

	// return the current output value
	get value() {
		return this._output;
	}

	// Update the output value
	update() {
		this._output = 0;
		for (const inputNeuron of this._input)
			this._output += inputNeuron.weight * inputNeuron.neuron.value;
		this._output = Neuron.Sigmoid( this._output * this._outputWeight );
	}

	// Randomize all the weights on the inputs
	randomize() {
		for (let inputNeuron of this._input)
			inputNeuron.weight = Math.random();
	}

	// Returns all the weights as an array including the input weight
	get weights() {
		let list = [this._outputWeight];
		for (let inputNeuron of this._input)
			list.push(inputNeuron.weight);
		return list;
	}	

	// Set the weights from the supplied weights array starting at startPosition an returning the new position
	applyWeights(weights, startPosition) {
		this._outputWeight = weights[startPosition++];
		for (let inputNeuron of this._input)
			inputNeuron.weight = weights[startPosition++];
		return startPosition;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Creates a simple neural network.  networkLayers is an array of the number of neurons at each layer 
class NeuralNetwork {
	constructor (networkLayers) {
		this._network = [];

		// Prepare the layers	
		for (const count of networkLayers)
			this._network.push( [] );

		// Prepare the input layer
		for (let count=0; count < networkLayers[0]; count++) 
			this._network[0].push(new InputNeuron());
		
		// Prepare the rest of the layers
		for (let layer=1; layer < networkLayers.length; layer++) {
			// Create the neurons and build the network
			for (let n=0; n<networkLayers[layer]; n++) {
				// Create neuron, supplied with a list of the neurons in the previous layer
				this._network[layer].push(new Neuron( this._network[layer-1] ));
			}
		}

		// Randomize all weights in the network
		this.randomize();
	}
	
	// Randomize all weights in the network "erase" the brain
	randomize() {
		const maxLayer = this._network.length;
		// Level 0 is the input, nothing can be randomized there
		for (let layer=1; layer<maxLayer; layer++)
			for (let neuron of this._network[layer])
				neuron.randomize();
	}

	// Set the value of an input neuron
	setInput(inputNumber, value) {
		this._network[0][inputNumber].value = value;
	}

	// Set the values of all the inputs
	set inputs(values) {
		const maxInput = this._network[0].length;
		for (let index = 0; index<maxInput; index++)
			this._network[0][index].set(values[index]);
	}

	// Returns the number of inputs
	get numInputs() {
		return this._network[0].length;
	}

	// Returns all of the weights used to describe this network. The "Brain"
	get weights() {
		let values = [];
		const maxLayer = this._network.length;
		for (let layer=1; layer<maxLayer; layer++)
			for (let neuron of this._network[layer])
				values.push(...neuron.weights);
		return values;
	}

	// Set all the weights for the network (Apply the brain)
	set weights(weights) {
		let position = 0;
		const maxLayer = this._network.length;
		for (let layer=1; layer<maxLayer; layer++)
			for (let neuron of this._network[layer])
				position = neuron.applyWeights(weights, position);
	}

	// Calculates the latest output from the network
	update() {
		const maxLayer = this._network.length;

		for (let layer=1; layer<maxLayer; layer++)
			for (let neuron of this._network[layer])
				neuron.update();
	}

	// Return an output from the network
	getOutput(outputNumber) {
		return this._network[this._network.length - 1][outputNumber].value;
	}

	// Returns the values from all the outputs
	get outputs() {
		let output = [];
		for (let neuron of this._network[this._network.length - 1])
			output.push(neuron.value);
		return output;
	}

	// Returns the number of outputs
	get numOutputs() {
		return this._network[this._network.length - 1].length;
	}
}