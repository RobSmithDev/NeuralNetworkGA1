// @RobSmithDev 
// Lifeform simulation experiment

// The screen!
class Simulation {
	constructor (canvasID, textOutput, scoreOutput) {
		this._canvasObject = document.getElementById(canvasID);
		this._canvas = this._canvasObject.getContext('2d');
		this._canvasWidth = parseInt(this._canvasObject.width);
		this._canvasHeight = parseInt(this._canvasObject.height);
		let canvas = document.createElement("canvas");
		canvas.width = this._canvasWidth;
    	canvas.height = this._canvasHeight;
		this._offScreenCanvas = {
			canvas: canvas.getContext('2d'),
			container: canvas
		}
		this._loggingPanel = document.getElementById(textOutput);
		this._scoreOutput = document.getElementById(scoreOutput);
		this._species = [];
		this._graphEntries = [];
		this._iterations = 0;
		this._iterationCounter = 0;
		this._ga = new GeneticAlgorithm();
		this._resources = [];
		this._speed = 1;
		this._aliveNow = 0;
		this._longestTime = 0;
		this._lifeformSize = Math.min(this._canvasWidth,this._canvasHeight) * 0.01;
		this._lineSize = this._canvasWidth * 0.001;

		// Prepare initial species
		for (let a=0; a<POPULATION_SIZE; a++) 
			this._species.push(new Lifeform(this));

		this.resetResources();	
		this.setTitle("Running first generation with "+POPULATION_SIZE+" life forms");
		this.buildScoreBoard();

		var that = this;
		setTimeout(function() {that.firstRun(); },1);
	}

	get width() {
		return this._canvasWidth;
	}
	get height() {
		return this._canvasHeight;
	}
	get lineWidth() {
		return this._lineSize;
	}

	get lifeformSize() {
		return this._lifeformSize;
	}

	get resources() {
		return this._resources;
	}

	// Create the scoreboard 
	buildScoreBoard() {
		let t = '';
		for (let a=0; a<POPULATION_SIZE; a++) {
			t+='<div class="container"><div class="containerText" id="text'+a+'">'+(a+1)+'</div>';
			t+='<div class="foodClass" id="food'+a+'" style="width: 10px"></div>';
			t+='<div class="waterClass" id="water'+a+'" style="width: 10px"></div>';
			t+='</div>';
		}
		this._scoreOutput.innerHTML = t;
	}	
		
	// Set the title section
	setTitle(message) {
		this._loggingPanel.innerHTML = message;
	}

	// Returns the index of a resource at the position specified
	resourceAtPosition(x, y) {
		const resourceCount = this._resources.length;

		for (var a=0; a<resourceCount; a++) {
			var xx = x - this._resources[a].x;
			var yy = y - this._resources[a].y;
			if ( (xx*xx)+(yy*yy) <= this._resources[a].radiusSquared*2)
				return a;
		}
		return -1;
	}

	// Respawn the food
	respawnFood(foodIndex) {
		this._resources[foodIndex] = this._createRandomFoodParticle();
	}

	// Creates a random food particle
	_createRandomFoodParticle() {
		let obj = {
				type: RESOURCE_FOOD, 
				x: (this._canvasWidth * 0.1) + (Math.random() * this._canvasWidth * 0.8), 
				y: (this._canvasHeight * 0.1) + (Math.random() * this._canvasHeight * 0.8), 
				w: 0.01 * Math.min(this._canvasWidth, this._canvasHeight), radiusSquared: 0
			};

		// Ensure it doesn't overlap another food particle		
		while (this.resourceAtPosition(obj.x, obj.y)>=0) { 
			obj.x = (this._canvasWidth * 0.1) + (Math.random() * this._canvasWidth * 0.8);
			obj.y = (this._canvasHeight * 0.1) + (Math.random() * this._canvasHeight * 0.8);
		}
		
		// Push it	
		obj.radiusSquared = obj.w * obj.w;
		return obj;
	}

	// Prepare the resource list
	resetResources() {
		// Add water
		let obj = {
			type: RESOURCE_WATER, 
			x: 0.3 * this._canvasWidth, 
			y: 0.2 * this._canvasHeight, 
			w: 0.1 * Math.min(this._canvasWidth, this._canvasHeight), 
			radiusSquared: 0
		};
	
		obj.radiusSquared = obj.w * obj.w; 
		this._resources.push(obj);
	
		obj = {
			type: RESOURCE_WATER, 
			x:0.9 * this._canvasWidth, 
			y:0.9 * this._canvasHeight, 
			w: 0.2 * Math.min(this._canvasWidth, this._canvasHeight), 
			radiusSquared: 0
		};
		obj.radiusSquared = obj.w * obj.w; 
		this._resources.push(obj);

		// Add food
		for (let a=0; a<NUM_FOOD_ON_BOARD; a++)
			this._resources.push(this._createRandomFoodParticle());
	}

	// Draw the resources on the map
	drawMapResources() {
		for (const resource of this._resources) {
			this._offScreenCanvas.canvas.beginPath();
			this._offScreenCanvas.canvas.arc(resource.x, resource.y, resource.w, 0, 2 * Math.PI, false);
			
			switch (resource.type) {
				case RESOURCE_WATER: this._offScreenCanvas.canvas.fillStyle = 'blue'; break;
				case RESOURCE_FOOD: this._offScreenCanvas.canvas.fillStyle = 'lime'; break;
			}			
			this._offScreenCanvas.canvas.fill();
			this._offScreenCanvas.canvas.lineWidth = this.canvasWidth * 0.001;
			this._offScreenCanvas.canvas.strokeStyle = '#404040';
			this._offScreenCanvas.canvas.stroke();
		}
	}

	// Create the next generation
	createChildPopulation() {
		// Step 1: Prepare the fitness
		for (let lifeform of this._species) 
			lifeform.calculateFitness();

		// Step 2: Produce child race
		this._ga.setParents(this._species);
		this._ga.produceNextGeneration(NUM_ALPHAS, CROSS_OVER_RATE, MUTATION_RATE, MUTATION_AMOUNT);
		this._species = this._ga.getChildren();		
	}

	// First run
	firstRun() {
		for (let a=0; a<POPULATION_SIZE; a++) {
			let elements = {
				food:document.getElementById('food'+a).style,
				water:document.getElementById('water'+a).style,
				text:document.getElementById('text'+a).style
			}
			this._graphEntries.push(elements);
		}
		this.run();	
	}

	// Updates the scoreboard 
	updateScoreboard() {
		for (let a=0; a<POPULATION_SIZE; a++) {
			let e = this._species[a].energy;
			this._graphEntries[a].food.width = parseInt(550 * e.food / MAX_FOOD)+'px';
			this._graphEntries[a].water.width = parseInt(550 * e.water / MAX_FOOD)+'px';
			this._graphEntries[a].text.backgroundColor = ((a<NUM_ALPHAS) && (this.iterations)) ? '#FFFF00' : '#E0E0E0';
		}
	}

	run() {
		// Clear it		
		this._offScreenCanvas.canvas.clearRect(0, 0, this._canvasWidth, this._canvasHeight);

		// 	run the system for "speed" times
		this.iterate();
		
		// Draw the current map resources onto the back buffer
		this.drawMapResources();	
		
		for (let a=0; a<POPULATION_SIZE; a++) 
			this._species[a].drawTo(this._offScreenCanvas.canvas, (a<NUM_ALPHAS) && (this._iterations));
		
		// Finally output it
		this._canvas.clearRect(0, 0, this._canvasWidth, this._canvasHeight);
		this._canvas.drawImage(this._offScreenCanvas.container,0,0);
		this.updateScoreboard();

		// Trigger again, faster if runnign at full speed
		if (this._speed<0) 
			setTimeout(()=>{this.run();},0); 
		else 
			requestAnimationFrame(()=>{this.run();});	
	}

	// Handle a single iteration
	iterate(drawIt) {
		// Check for new generation
		if (++this._iterationCounter >= GENERATION_LIFETIME) {
			this._iterations++;		
			this.createChildPopulation();
			this._iterationCounter=0;
			if (this._aliveNow>0) 
				this.setTitle('Running Generation '+this._iterations+' ('+this._aliveNow+' survived last round)');
			else this.setTitle('Running Generation '+this._iterations+' (last round lasted '+this._longestTime+' iterations)');
			
			if (typeof this._canvas.isContextLost !== 'undefined')
				if (this._canvas.isContextLost()) 				
					this._canvas = this._canvasObject.getContext('2d');
		}
			
		// Update every single lifeform
		const iterationsToDo = this._speed>0 ? this._speed : GENERATION_LIFETIME;
		let alive = 0;
		for (let g=0; g<iterationsToDo; g++) {
			alive = 0;
			for (let lifeform of this._species) 
				if (lifeform.step(this.resources)) alive++;
			this._iterationCounter++;
			if (alive<1) break;
		}
		
		// Some statistics
		if (alive<1) {
			this._longestTime = this._iterationCounter;
			this._iterationCounter=GENERATION_LIFETIME;			
		}
		this._aliveNow  = alive;
	}

	// Switch the speed
	set speed(speed) {
		this._speed = speed;
	}

	// Switch to data from a saved generation
	switchGenerations(generation, weights) {
		for (let a=0; a<weights.length; a++)
			this._species[a].weights = weights[a];
		this._iterationCounter = 0;
		this._iterations = generation;
		this.setTitle('Running Generation '+generation+' (with preloaded weights)');
	}

	// Upgrade the data on a single lifeform
	upgradeEntitiy(weights) {
		this._species[0].weights = weights[0];
		this._species[1].weights = weights[1];
		this._species[2].weights = weights[2];
		this._species[3].weights = weights[3];
	}

	// Restart the simulation
	resetToZero() {
		this._iterationCounter = 0;
		this._iterations = 0;
		for (let lifeform of this._species) lifeform.resetLifeform();		
		this.setTitle('Running first generation with '+POPULATION_SIZE+' life forms');
	}
}



