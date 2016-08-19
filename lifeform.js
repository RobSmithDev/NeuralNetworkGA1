// @RobSmithDev 
// Lifeform simulation experiment - a single lifeform
"use strict";


// A single lifeform
function lifeform(width, height, app, lifeformIndex) {
	// NN controlling this lifeform
	//this.brain = new NeuralNetwork([8,12,  2]);
	this.brain = new NeuralNetwork([8,14,12,  2]);	
	this.position = {x:0, y:0, angle: 0, lastMoveX:0, lastMoveY:0};
	this.size = 0;
	this.app = app;
	this.boxSize = {w:0,h:0};
	this.energyStatus = {water: 0, food: 0};
	this.lifeSpan = 0;
	this.targetFoodXY = {x:0,y:0};
	this.targetWaterXY = {x:0,y:0};
	this.reset(width, height);	
}	
	
	
// Return the energy we have left
lifeform.prototype.getEnergy = function() {
	return this.energyStatus;
}

// Calculate the fitness of this lifeform
lifeform.prototype.calculateFitness = function() {
	// Water and energy left over is important, but lifespan is of greater value
	this.brain.setFitness((this.energyStatus.water/MAX_WATER) * (this.energyStatus.food/MAX_FOOD) + (this.lifeSpan/GENERATION_LIFETIME));
}

// Return the fitness of this 
lifeform.prototype.getFitness = function() {
	return this.brain.getFitness();
}

// Reset the lifeform and wipe its memory
lifeform.prototype.fullReset = function() {
	this.reset(this.box.w, this.box.h);
	this.brain.resetRandom();
}
	
// Trigger the reset
lifeform.prototype.reset = function(width, height) {
	this.position.angle = Math.random() * Math.PI * 2.0;
	// Position randomly, but away from the screen edges and not on any food or water
	do {
		this.position.x = (width * 0.1) + (Math.random() * width * 0.8);
		this.position.y = (height * 0.1) + (Math.random() * height * 0.8);
	} while (this.app.resourceAtPosition(this.position.x,this.position.y)>=0);
	this.box = {w:width, h:height};
	this.lifeSpan = 0;
	this.energyStatus = {water: WATERUSED_PER_STEP * START_STEPS, food: FOODUSED_PER_STEP * START_STEPS};
	this.size = Math.min(width,height) * 0.01;
}

// Get the genome for this
lifeform.prototype.getWeights = function() {
	return this.brain.getWeights();
}

// Set the genome for this 
lifeform.prototype.setWeights = function(geneome) {
	this.brain.setWeights(geneome);
	this.reset(this.box.w, this.box.h);
}

// Draw this into the canvas
lifeform.prototype.drawTo = function(canvas, isAlpha) {
	canvas.beginPath();
	canvas.arc(this.position.x, this.position.y, this.size, 0, 2 * Math.PI, false);
	if ((this.energyStatus.water>0) && (this.energyStatus.food>0)) {
		if (isAlpha) canvas.fillStyle = 'yellow';  else canvas.fillStyle = 'red'; 
	} else canvas.fillStyle = 'rgba(128,128,128,0.5)'; 
	canvas.fill();
	canvas.lineWidth = this.box.w * 0.001;
	canvas.strokeStyle = '#000000';
	canvas.stroke();
	canvas.beginPath();
	canvas.lineWidth = this.box.w * 0.005;
	canvas.moveTo(this.position.x, this.position.y);
	canvas.lineTo(this.position.x + Math.cos(this.position.angle) * this.size*1.5, this.position.y + Math.sin(this.position.angle) * this.size * 1.5);
	canvas.strokeStyle = '#000000';
	canvas.stroke();
	// Now draw destination markers
	canvas.beginPath();
	canvas.lineWidth = this.box.w * 0.001;
	canvas.moveTo(this.position.x, this.position.y);
	canvas.lineTo(this.targetFoodXY.x , this.targetFoodXY.y);
	canvas.strokeStyle = 'rgba(0,255,0,0.5)';
	canvas.stroke();
	// Now draw destination markers
	canvas.beginPath();
	canvas.lineWidth = this.box.w * 0.001;
	canvas.moveTo(this.position.x, this.position.y);
	canvas.lineTo(this.targetWaterXY.x , this.targetWaterXY.y);
	canvas.strokeStyle = 'rgba(0,255,255,0.5)';
	canvas.stroke();
	
}

// Runs the NN one step
lifeform.prototype.step = function( resources) {
	// While there is food and water
	if ((this.energyStatus.water>0) && (this.energyStatus.food>0)) {
		// Water is lost faster than food - food is only needed once every 24 cycles
		this.energyStatus.water-=WATERUSED_PER_STEP;
		this.energyStatus.food-=FOODUSED_PER_STEP;
		
		// Increase a lifespan
		this.lifeSpan++;
				
		// Find the nearest water and food
		var nearestFoodIndex = -1;
		var nearestWaterIndex = -1;
		var nearestFoodValue=0;
		var nearestWaterValue=0;
		var resourceCount = resources.length;
		for (var a=0; a<resourceCount; a++) {
			// Calculate the distance away
			var distanceX = this.position.x - resources[a].x;
			var distanceY = this.position.y - resources[a].y;
			var distance = ((distanceX*distanceX) + (distanceY*distanceY)) - resources[a].radiusSquared;
			if (distance<0) distance = 0;
			
			// Check the resource type and keep the nearest
			switch (resources[a].type) {
				case RESOURCE_FOOD:
					 if ((distance<nearestFoodValue) || (nearestFoodIndex==-1)) {
						 nearestFoodIndex = a;
						 nearestFoodValue = distance;
					 }
					 break;
				case RESOURCE_WATER:
					 if ((distance<nearestWaterValue) || (nearestWaterIndex==-1)) {
						 nearestWaterIndex = a;
						 nearestWaterValue = distance;
					 }
					 break;
			}
		}

		this.targetFoodXY = {x:resources[nearestFoodIndex].x,y:resources[nearestFoodIndex].y};
		this.targetWaterXY = {x:resources[nearestWaterIndex].x,y:resources[nearestWaterIndex].y};
		var x = this.position.x - this.targetFoodXY.x;
		var y = this.position.y - this.targetFoodXY.y;
		var z = Math.sqrt((x*x)+(y*y));
		
		// Set were food is
		this.brain.setInputValue(0, x/z);
		this.brain.setInputValue(1, y/z);
		
		// Set where water is
		var x = this.position.x - this.targetWaterXY.x;
		var y = this.position.y - this.targetWaterXY.y;
		var z = Math.sqrt((x*x)+(y*y));
		this.brain.setInputValue(2, x/z);
		this.brain.setInputValue(3, y/z);			
		
		// Current directions
		this.brain.setInputValue(4, this.position.lastMoveX);
		this.brain.setInputValue(5, this.position.lastMoveY);
		
		// how much of each is left
		this.brain.setInputValue(6, this.energyStatus.food/MAX_FOOD);
		this.brain.setInputValue(7, this.energyStatus.water/MAX_WATER);
	
		// Update the brain calculations
		this.brain.calculate();
		
		var output = this.brain.getAllOutputs();
	
		
		// Run the output, the two outputs are left and right wheel speed.  They are used to work out the angle change
		var angleChange = output[0] - output[1];
		if (angleChange<-MAX_TURN_SPEED) angleChange=-MAX_TURN_SPEED;
		if (angleChange>MAX_TURN_SPEED) angleChange = MAX_TURN_SPEED;

		// Change the angle we're facing
		this.position.angle+=angleChange;
		var speedStep = output[0] + output[1];
		
		this.position.lastMoveX = Math.cos(this.position.angle);
		this.position.lastMoveY = Math.sin(this.position.angle);
		
		// And move in this direction
		this.position.x += this.position.lastMoveX * speedStep;
		this.position.y += this.position.lastMoveY * speedStep;
		
		if (this.position.x<0) this.position.x=this.box.w-1;
		if (this.position.y<0) this.position.y=this.box.h-1;
		if (this.position.x>=this.box.w) this.position.x=0;
		if (this.position.y>=this.box.h) this.position.y=0;		

		// See what happened			
		var i = this.app.resourceAtPosition(this.position.x,this.position.y);
		if (i>=0) {
			switch (resources[i].type) {
				case RESOURCE_FOOD:
					 this.energyStatus.food+=FOODATE_PER_DOT;  // EAT! - 4 cycles of food
					 if (this.energyStatus.food>MAX_FOOD) this.energyStatus.food=MAX_FOOD;
					 // now move the food
					 this.app.respawnFood(i);  
					 break;
				case RESOURCE_WATER:
					 this.energyStatus.water+=WATERDRANK_PER_STEP;    // DRINK
					 if (this.energyStatus.water>MAX_WATER) this.energyStatus.water=MAX_WATER;
					 break;
			}
		}
		return true;
	} else return false;
}



