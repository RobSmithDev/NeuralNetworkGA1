// @RobSmithDev 
// Lifeform simulation experiment
"use strict";

// The number of life forms in each generation
var POPULATION_SIZE = 30;

// The amount of time the generation should run for (number of iterations)
var GENERATION_LIFETIME = 6000;

// Water type
var RESOURCE_WATER = 1;
var RESOURCE_FOOD = 2;

// Maximum a lifeform can carry
var MAX_FOOD = 10000.0;
var MAX_WATER = 10000.0;

// How much food and water is lost on each move
var FOODUSED_PER_STEP = 1;
var WATERUSED_PER_STEP = 1;

// How much food and water is gained by eating and drinking
var WATERDRANK_PER_STEP = WATERUSED_PER_STEP * 2;  // Drink 10 steps worth
var FOODATE_PER_DOT = FOODUSED_PER_STEP * 250;    // Eat 50 steps worth  

// Number of steps allowed on startup before dieing (used to calculate food and water level)
var START_STEPS = 1500;
// How far you can turn in degrees maximum in one go
var MAX_TURN_SPEED = 0.3;
// Number of alphas to push forward to the next generation
var NUM_ALPHAS = 4;
// Number of food blobs on screen
var NUM_FOOD_ON_BOARD = 20;
// The cross-over rate used in the GA
var CROSS_OVER_RATE = 0.7;
// Mutation rate
var MUTATION_RATE = 0.1;
// The amount of mutation that can happen
var MUTATION_AMOUNT = 0.3;