// @RobSmithDev 
// Lifeform simulation experiment - a single lifeform

// A single lifeform
class Lifeform{

	constructor(simulation) { 
		// NN controlling this lifeform
		//this.brain = new NeuralNetwork([8,12,  2]);

		// The Neural Network Brain
		this._brain = new NeuralNetwork([8,14,12,  2]);	
		
		// The position of this lifeform
		this._position = {x:0, y:0, angle: 0, lastMoveX:0, lastMoveY:0};
		
		// The simulation (world)
		this._simulation = simulation;
		
		// Life form Health
		this._energyStatus = {water: 0, food: 0};
		
		// How long its been alive
		this._lifeSpan = 0;

		// A score for how well this did
		this._fitnessScore = 0;

		// The target of the nearest food 
		this.targetFoodXY = {x:0,y:0};

		// The target coordinates of the nearest water
		this.targetWaterXY = {x:0,y:0};

		// Reset
		this.resetLifeform();	
	}

	// Return the brain weights
	get weights() {
		return this._brain.weights;
	}

	// Set the brain weights
	set weights(weights) {
		this._brain.weights = weights;
		this.resetAge();
	}

	// Reset the status of the lifeform back to age=0 without affecting its brain	
	resetAge() {
		this._position.angle = Math.random() * Math.PI * 2.0;
		// Position randomly, but away from the screen edges and not on any resources
		do {
			this._position.x = (this._simulation.width * 0.1) + (Math.random() * this._simulation.width * 0.8);
			this._position.y = (this._simulation.height * 0.1) + (Math.random() * this._simulation.height * 0.8);
		} while (this._simulation.resourceAtPosition(this._position.x,this._position.y)>=0);
	
		this._lifeSpan = 0;
		this._energyStatus = { water: WATERUSED_PER_STEP * START_STEPS, 
			                   food: FOODUSED_PER_STEP * START_STEPS};
	}

	// Reset age and wipe the brain
	resetLifeform() {
		this.resetAge();
		this._brain.randomize();
	}

	// Calculate a score representing the fitness of the lifeform
	calculateFitness() {
		this._fitnessScore = ((this._energyStatus.water/MAX_WATER) * (this._energyStatus.food/MAX_FOOD)) + (this._lifeSpan/GENERATION_LIFETIME);
	}

	// Return the current fitness score
	get fitness() {
		return this._fitnessScore;
	}

	// Run the neural network once 
	step() {
		const resources = this._simulation.resources;
		// While there is food and water
		if ((this._energyStatus.water<=0) || (this._energyStatus.food<=0)) return false;

		// Water is lost faster than food - food is only needed once every 24 cycles
		this._energyStatus.water -= WATERUSED_PER_STEP;
		this._energyStatus.food -= FOODUSED_PER_STEP;
		
		// Increase a lifespan
		this._lifeSpan++;
				
		// Find the nearest water and food
		let nearestFoodIndex = -1;
		let nearestWaterIndex = -1;
		let nearestFoodValue = 0;
		let nearestWaterValue = 0;
		let resourceCount = resources.length;
		const halfW = this._simulation.width / 2;
		const halfH = this._simulation.height / 2;

		for (let a=0; a<resourceCount; a++) {
			// Calculate the distance away
			let distanceX = Math.abs(this._position.x - resources[a].x);
			let distanceY = Math.abs(this._position.y - resources[a].y);
			if (distanceX > halfW) distanceX = this._simulation.width - distanceX;
			if (distanceY > halfH) distanceY = this._simulation.height - distanceY;
			
			const distance = Math.max(0, ((distanceX*distanceX) + (distanceY*distanceY)) - resources[a].radiusSquared);
			
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

		this._targetFoodXY =  { x:resources[nearestFoodIndex].x,  y:resources[nearestFoodIndex].y  };
		this._targetWaterXY = { x:resources[nearestWaterIndex].x, y:resources[nearestWaterIndex].y };

		let x = this._position.x - this._targetFoodXY.x;
		let y = this._position.y - this._targetFoodXY.y;
		if (Math.abs(x) > halfW) {
			// To far.  Go for the wrap-around approach
			this._targetFoodXY.x = x>0 ? (this._targetFoodXY.x + this._simulation.width) : (this._targetFoodXY.x - this._simulation.width) ;
			x = this._position.x - this._targetFoodXY.x;
		}
		if (Math.abs(y) > halfH) {
			// To far.  Go for the wrap-around approach
			this._targetFoodXY.y = y>0 ? (this._targetFoodXY.y + this._simulation.height) : (this._targetFoodXY.y - this._simulation.height) ;
			y = this._position.y - this._targetFoodXY.y;
		}

		let z = Math.sqrt((x*x)+(y*y));
		
		// Set where food is as a vector
		this._brain.setInput(0, x/z);
		this._brain.setInput(1, y/z);
		
		// Set where water is as a vector
		x = this._position.x - this._targetWaterXY.x;
		y = this._position.y - this._targetWaterXY.y;
		if (Math.abs(x) > halfW) {
			// To far.  Go for the wrap-around approach
			this._targetWaterXY.x = x>0 ? (this._targetWaterXY.x + this._simulation.width) : (this._targetWaterXY.x - this._simulation.width) ;
			x = this._position.x - this._targetWaterXY.x;
		}
		if (Math.abs(y) > halfH) {
			// To far.  Go for the wrap-around approach
			this._targetWaterXY.y = y>0 ? (this._targetWaterXY.y + this._simulation.height) : (this._targetWaterXY.y - this._simulation.height) ;
			y = this._position.y - this._targetWaterXY.y;
		}

		z = Math.sqrt((x*x)+(y*y));
		this._brain.setInput(2, x/z);
		this._brain.setInput(3, y/z);			
		
		// Current directions
		this._brain.setInput(4, this._position.lastMoveX);
		this._brain.setInput(5, this._position.lastMoveY);
		
		// how much of each is left
		this._brain.setInput(6, this._energyStatus.food/MAX_FOOD);
		this._brain.setInput(7, this._energyStatus.water/MAX_WATER);
	
		// Update the brain calculations
		this._brain.update();
		
		const output = this._brain.outputs;
	
		// Run the output, the two outputs are left and right wheel speed.  They are used to work out the angle change
		let angleChange = output[0] - output[1];
		if (angleChange < -MAX_TURN_SPEED) angleChange =- MAX_TURN_SPEED;
		if (angleChange >  MAX_TURN_SPEED) angleChange =  MAX_TURN_SPEED;

		// Change the angle we're facing
		this._position.angle += angleChange;
		const speedStep = output[0] + output[1];
		
		this._position.lastMoveX = Math.cos(this._position.angle);
		this._position.lastMoveY = Math.sin(this._position.angle);
		
		// And move in this direction
		this._position.x += this._position.lastMoveX * speedStep;
		this._position.y += this._position.lastMoveY * speedStep;
		
		if (this._position.x<0) this._position.x=this._simulation.width-1;
		if (this._position.y<0) this._position.y=this._simulation.height-1;
		if (this._position.x>=this._simulation.width) this._position.x=0;
		if (this._position.y>=this._simulation.height) this._position.y=0;		

		// See what happened			
		const i = this._simulation.resourceAtPosition(this._position.x,this._position.y);
		if (i>=0) {
			switch (resources[i].type) {
				case RESOURCE_FOOD:
					 this._energyStatus.food+=FOODATE_PER_DOT;  // EAT! - 4 cycles of food
					 if (this._energyStatus.food > MAX_FOOD) this._energyStatus.food = MAX_FOOD;
					 // now move the food
					 this._simulation.respawnFood(i);  
					 break;
				case RESOURCE_WATER:
					 this._energyStatus.water+=WATERDRANK_PER_STEP;    // DRINK
					 if (this._energyStatus.water > MAX_WATER) this._energyStatus.water = MAX_WATER;
					 break;
			}
		}
		return true;
	}

		
	// Draw this into the canvas.  isAlpha is set if this is one of the original lifeforms selected
	drawTo(canvas, isAlpha) {
		canvas.beginPath();
		canvas.arc(this._position.x, this._position.y, this._simulation.lifeformSize, 0, 2 * Math.PI, false);
		
		if ((this._energyStatus.water>0) && (this._energyStatus.food>0)) {
			if (isAlpha) 
				canvas.fillStyle = 'yellow';  
			else canvas.fillStyle = 'red'; 
		} else 
			canvas.fillStyle = 'rgba(128,128,128,0.5)'; 
		
		canvas.fill();
		canvas.lineWidth = this._simulation.lineWidth;
		canvas.strokeStyle = '#000000';
		canvas.stroke();
		canvas.beginPath();
		canvas.lineWidth = this._simulation.lineWidth*5;
		canvas.moveTo(this._position.x, this._position.y);
		canvas.lineTo(this._position.x + Math.cos(this._position.angle) *  this._simulation.lifeformSize*1.5, this._position.y + Math.sin(this._position.angle) *  this._simulation.lifeformSize * 1.5);
		canvas.strokeStyle = '#000000';
		canvas.stroke();
		// Now draw destination markers
		canvas.beginPath();
		canvas.lineWidth = this._simulation.lineWidth;
		this.wrapLine(canvas, this._position.x, this._position.y, this._targetFoodXY.x , this._targetFoodXY.y)
		canvas.strokeStyle = 'rgba(0,255,0,0.5)';
		canvas.stroke();
		// Now draw destination markers
		canvas.beginPath();
		canvas.lineWidth = this._simulation.lineWidth;
		this.wrapLine(canvas, this._position.x, this._position.y, this._targetWaterXY.x , this._targetWaterXY.y)
		
		canvas.strokeStyle = 'rgba(0,255,255,0.5)';
		canvas.stroke();
	}

	// Return the energy we have left
	get energy() {
		return this._energyStatus;
	}

	wrapLine (canvas,x,y,dx,dy) {
		canvas.moveTo(x,y);
		canvas.lineTo(dx,dy);
	}
}	
	
	
