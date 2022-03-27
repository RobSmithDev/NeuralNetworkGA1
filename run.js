// @RobSmithDev 
// Lifeform simulation experiment
"use strict";

// Class for the simulation
var simulation = 0;

// Chyange the speed
function switchSpeed(speed) {
	simulation.speed = speed;
}

// Reset to generation 0
function resetTo0() {
	simulation.resetToZero();
}

// Upgrade one lifeform to one from generation 17120
function resetToFuture() {
	simulation.upgradeEntitiy(weightsForIteration17120);
}
				
// Goto generation 17120
function resetTo17120() {
	simulation.switchGenerations(17120, weightsForIteration17120);
}


document.addEventListener("DOMContentLoaded", function(event) { 
  // Create simulation
  simulation = new Simulation("mycanvas", "textoutput", "scores");
});