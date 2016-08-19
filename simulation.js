// @RobSmithDev 
// Lifeform simulation experiment
"use strict";



// The screen!
function Simulation(canvasID, textOutput, scoreOutput) {
	this.canvasObject = document.getElementById(canvasID);
	this.canvas = getCanvas(this.canvasObject);
	this.canvasWidth = parseInt(this.canvasObject.width);
	this.canvasHeight = parseInt(this.canvasObject.height);
	this.offScreenCanvas = createHiddenCanvas(this.canvasWidth, this.canvasHeight);
	this.loggingPanel = document.getElementById(textOutput);
	this.scoreOutput = document.getElementById(scoreOutput);
	this.species = [];
	this.graphEntries = [];
	this.iterations = 0;
	this.iterationCounter = 0;
	this.ga = new GeneticAlgorithm();
	this.resources = [];
	this.speed = 1;
	this.aliveNow = 0;
	this.longestTime = 0;

	// Prepare initial species
	for (var a=0; a<POPULATION_SIZE; a++) this.species.push(new lifeform(this.canvasWidth, this.canvasHeight, this, a));
	this.resetResources();	
	this.setTitle("Running first generation with "+POPULATION_SIZE+" life forms");
	this.buildScoreBoard();

	var that = this;
	setTimeout(function() {that.firstRun(); },1);
}


// Create the scoreboard 
Simulation.prototype.buildScoreBoard = function() {
	var t = '';
	for (var a=0;a<POPULATION_SIZE; a++) {
		t+='<div class="container"><div class="containerText" id="text'+a+'">'+(a+1)+'</div>';
		t+='<div class="foodClass" id="food'+a+'" style="width: 10px"></div>';
		t+='<div class="waterClass" id="water'+a+'" style="width: 10px"></div>';
		t+='</div>';
	}
	this.scoreOutput.innerHTML = t;
}	
	
// Set the title section
Simulation.prototype.setTitle = function(message) {
	this.loggingPanel.innerHTML = message;
}

// Returns the index of a resource at the position specified
Simulation.prototype.resourceAtPosition = function(x, y) {
	var resourceCount = this.resources.length;
	for (var a=0; a<resourceCount; a++) {
		var xx = x - this.resources[a].x;
		var yy = y - this.resources[a].y;
		if ( (xx*xx)+(yy*yy) <= this.resources[a].radiusSquared*2)
			return a;
	}
	return -1;
}

// Respawn the food
Simulation.prototype.respawnFood = function(foodIndex) {
	this.resources[foodIndex] = this.createRandomFoodParticle();
}

// Creates a random food particle
Simulation.prototype.createRandomFoodParticle = function() {
	var obj = {type: RESOURCE_FOOD, x:(this.canvasWidth * 0.1) + (Math.random() * this.canvasWidth * 0.8), y:(this.canvasHeight * 0.1) + (Math.random() * this.canvasHeight * 0.8), w: 0.01 * Math.min(this.canvasWidth, this.canvasHeight), radiusSquared: 0};

	// Ensure it doesnt overlap another food particle		
	while (this.resourceAtPosition(obj.x, obj.y)>=0) 
		obj = {type: RESOURCE_FOOD, x:(this.canvasWidth * 0.1) + (Math.random() * this.canvasWidth * 0.8), y:(this.canvasHeight * 0.1) + (Math.random() * this.canvasHeight * 0.8), w: 0.01 * Math.min(this.canvasWidth, this.canvasHeight)};

	// Push it	
	obj.radiusSquared = obj.w * obj.w;
	return obj;
}

// Prepare the resource list
Simulation.prototype.resetResources = function() {
	// Add water
	var obj = {type: RESOURCE_WATER, x:0.3 * this.canvasWidth, y:0.2 * this.canvasHeight, w: 0.1 * Math.min(this.canvasWidth, this.canvasHeight), radiusSquared: 0};
	obj.radiusSquared = obj.w * obj.w; 
	this.resources.push(obj);
	
	var obj = {type: RESOURCE_WATER, x:0.9 * this.canvasWidth, y:0.9 * this.canvasHeight, w: 0.2 * Math.min(this.canvasWidth, this.canvasHeight), radiusSquared: 0};
	obj.radiusSquared = obj.w * obj.w; 
	this.resources.push(obj);

	// Add food
	for (var a=0; a<NUM_FOOD_ON_BOARD; a++)
		this.resources.push(this.createRandomFoodParticle());
}

// Draw the resources on the map
Simulation.prototype.drawMapResources = function() {
	var resourceCount = this.resources.length;
	for (var a=0; a<resourceCount; a++) {
		this.offScreenCanvas.canvas.beginPath();
		this.offScreenCanvas.canvas.arc(this.resources[a].x, this.resources[a].y, this.resources[a].w, 0, 2 * Math.PI, false);
		switch (this.resources[a].type) {
			case RESOURCE_WATER: this.offScreenCanvas.canvas.fillStyle = 'blue'; break;
			case RESOURCE_FOOD: this.offScreenCanvas.canvas.fillStyle = 'lime'; break;
		}			
		this.offScreenCanvas.canvas.fill();
		this.offScreenCanvas.canvas.lineWidth = this.canvasWidth * 0.001;
		this.offScreenCanvas.canvas.strokeStyle = '#404040';
		this.offScreenCanvas.canvas.stroke();
	}
}

// Create the next generation
Simulation.prototype.createChildPopulation = function() {
	// Step 1: Prepare the fitness
	for (var a=0; a<this.species.length; a++) this.species[a].calculateFitness();
	// Step 2: Produce child race
	this.ga.setParents(this.species);
	this.ga.produceNextGeneration(NUM_ALPHAS, CROSS_OVER_RATE, MUTATION_RATE, MUTATION_AMOUNT);
	this.species = this.ga.getChildren();		
}

// First run
Simulation.prototype.firstRun = function() {
	for (var a=0; a<POPULATION_SIZE; a++) {
		var d = {
			food:document.getElementById("food"+a).style,
			water:document.getElementById("water"+a).style,
			text:document.getElementById("text"+a).style
		}
		this.graphEntries.push(d);
	}
	this.run();	
}

// Updates the scoreboard 
Simulation.prototype.updateScoreboard = function() {
	for (var a=0; a<POPULATION_SIZE; a++) {
		var e = this.species[a].getEnergy();
		this.graphEntries[a].food.width = parseInt(550 * e.food / MAX_FOOD)+"px";
		this.graphEntries[a].water.width = parseInt(550 * e.water / MAX_FOOD)+"px";
		this.graphEntries[a].text.backgroundColor = ((a<NUM_ALPHAS) && (this.iterations))?"#FFFF00":"#E0E0E0";
	}
}

Simulation.prototype.run = function() {
	// Clear it		
	this.offScreenCanvas.canvas.clearRect(0, 0, this.canvasWidth, this.canvasHeight);

	// 	run the system for "speed" times
	this.iterate();
	
	// Draw the current map resources onto the back buffer
	this.drawMapResources();	
	
	for (var a=0; a<POPULATION_SIZE; a++) 
		this.species[a].drawTo(this.offScreenCanvas.canvas, (a<NUM_ALPHAS) && (this.iterations));
	
	
	// Finally output it
	this.canvas.clearRect(0, 0, this.canvasWidth, this.canvasHeight);
	this.canvas.drawImage(this.offScreenCanvas.container,0,0);
	this.updateScoreboard();

	// Cause another re-draw to occur
	var that = this;
	
		// Trigger again, faster if runnign at full speed
	if (this.speed<0) 
		setTimeout(function() {that.run();},0); else 
		requestAnimationFrame(function() {that.run();});	
}

// Handle a single iteration
Simulation.prototype.iterate = function(drawIt) {
	// Check for new generation
	if (++this.iterationCounter>=GENERATION_LIFETIME) {
		this.iterations++;		
		this.createChildPopulation();
		this.iterationCounter=0;
		if (this.aliveNow>0) 
			this.setTitle("Running Generation "+this.iterations+" ("+this.aliveNow+" survived last round)");
		else this.setTitle("Running Generation "+this.iterations+" (last round lasted "+this.longestTime+" iterations)");
		
		if (typeof this.canvas.isContextLost != "undefined")
			if (this.canvas.isContextLost()) 				
				this.canvas = getCanvas(this.canvasObject);
	}				
		
	// Update every single lifeform
	var iterationsToDo = this.speed>0 ? this.speed : GENERATION_LIFETIME;
	var alive = 0;
	for (var g=0; g<iterationsToDo; g++) {
		alive = 0;
		for (var a=0; a<POPULATION_SIZE; a++) 
			if (this.species[a].step(this.resources)) alive++;
		this.iterationCounter++;
		if (alive<1) break;
	}
	
	// Some statistics
	if (alive<1) {
		this.longestTime = this.iterationCounter;
		this.iterationCounter=GENERATION_LIFETIME;			
	}
	this.aliveNow  = alive;
}

// Switch the speed
Simulation.prototype.switchSpeed = function(speed) {
	this.speed = speed;
}

// Switch to data from a saved generation
Simulation.prototype.switchGenerations = function(generation, weights) {
	for (var a=0; a<weights.length; a++)
		this.species[a].setWeights(weights[a]);
	this.iterationCounter = 0;
	this.iterations = generation;
	this.setTitle("Running Generation "+generation+" (with preloaded weights)");
}

// Upgrade the data ona  single lifeform
Simulation.prototype.upgradeEntitiy = function(weights) {
	this.species[0].setWeights(weights[0]);
	this.species[1].setWeights(weights[1]);
	this.species[2].setWeights(weights[2]);
	this.species[3].setWeights(weights[3]);
}

// Restart the simulation
Simulation.prototype.resetToZero = function() {
	this.iterationCounter = 0;
	this.iterations = 0;
	for (var a=0; a<POPULATION_SIZE; a++) this.species[a].fullReset();		
	this.setTitle("Running first generation with "+POPULATION_SIZE+" life forms");
}




