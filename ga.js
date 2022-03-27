// @RobSmithDev 
// Basic genetic algorithm library
"use strict";

function GeneticAlgorithm() {
	var networks = [];
}	


// Set the parent generation
GeneticAlgorithm.prototype.setParents = function(parentsList) {
	this.networks = parentsList;
}

GeneticAlgorithm.prototype.getChildren = function() {
	return this.networks;
}
		
// Picks a random parent but uses the weighting to bais the fitness of the results
GeneticAlgorithm.prototype.pickParentViaRoulette = function(totalFitness) {
	var rand = Math.random() * totalFitness;
	var soFar = 0;
	
	var len = this.networks.length;
	for (var a=0; a<len; a++) {
		soFar += this.networks[a].fitness;
		if (soFar >= rand) return a;
	}
	
	// On error, return the first
	return 0;
}

// Do the cross over, the supplied parameters are just the networks
GeneticAlgorithm.prototype.crossOver = function(parent1, parent2, crossOverRate) {
	// These go onwards as clones
	if ((Math.random()>crossOverRate) || (parent1 == parent2)) 
		return [parent1.weights,parent2.weights];
		
	// Else produce offspring
	var w1 = parent1.weights;
	var w2 = parent2.weights;
	
	// This is a semi-uniform crossover with random chunks coming from either parent
	/*
	var a=0;
	var child1 = [];
	var child2 = [];
	var swapMode = false;
	while (a<w1.length) {
		// Pick an amount of chromosomes to copy
		var b = Math.max(1,Math.random() * ((w1.length-a))/2);
		
		// Copy them
		while ((b>0) && (a<w1.length)) {
			if (swapMode) {
				child1.push(w1[a]);
				child2.push(w2[a]);
			} else {
				child1.push(w2[a]);
				child2.push(w1[a]);					
			}
			a++;
			b--;				
		}
		// Now swap which parent they come from for the next round
		swapMode = !swapMode;			
	}
	
	return [child1,child2];
	*/
	// Simple crossover
	var crossPoint = Math.floor(Math.random() * w1.length);
	var child1 = [];
	var child2 = [];
	for (var a=0; a<crossPoint; a++) {
		child1.push(w1[a]);
		child2.push(w2[a]);
	}
	for (var a=crossPoint; a<w1.length; a++) {
		child1.push(w2[a]);
		child2.push(w1[a]);
	}
	return [child1,child2];
}

// Mutates the weights of the child
GeneticAlgorithm.prototype.mutate = function(child, mutationRate, mutationAmount) {
	for (var a=0; a<child.length; a++) 
		if (Math.random()<mutationRate) 
			child[a]+= mutationAmount * (Math.random()-0.5) * 2.0;  
	return child;
}

// Produces the next generation based on this one
// 		numBest			is the number of most fit networks to just pass on anyway (0..1)
//      crossOverRate   is the % of children that are NOT clones of the parents (0..1) - should be around 0.7
//      mutationRate    is the % of chromosomes that will be mutated.  (0..1) - should be around 0.1  
//      dMaxPerturbation is the amount of mutation that could theoretically happen (0..1) - should be around 0.3
GeneticAlgorithm.prototype.produceNextGeneration = function(numBest, crossOverRate, mutationRate, mutationAmount) {
	// First calculate fitness and get a total
	var totalFitness = 0;
	for (var a=0; a<this.networks.length; a++) 
		totalFitness+=this.networks[a].fitness;
	
	// Sort the list by fitness, worst first
	this.networks.sort(function(a,b) {return a.fitness-b.fitness;});
	
	// Output the best ones regardless
	var newNetworks = [];
	while (numBest>0) {
		newNetworks.push(this.networks[this.networks.length-numBest].weights);
		numBest--;
	}
	
	// Now keep outputting new weights until we have all of them
	while (newNetworks.length<this.networks.length) {
		var parent1 = this.networks[ this.pickParentViaRoulette(totalFitness) ];
		var parent2 = this.networks[ this.pickParentViaRoulette(totalFitness) ];
		
		// Cross over
		var children = this.crossOver(parent1,parent2,crossOverRate);
			
		// Mutate and Keep
		newNetworks.push(this.mutate(children[0], mutationRate, mutationAmount));
		newNetworks.push(this.mutate(children[1], mutationRate, mutationAmount));			
	}
	
	// Now reset all the networks with the new values
	for (var a=0; (a<newNetworks.length) && (a<this.networks.length); a++)
		this.networks[a].weights = newNetworks[a];
}



