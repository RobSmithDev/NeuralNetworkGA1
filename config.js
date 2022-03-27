// @RobSmithDev 
// Lifeform simulation experiment

// The number of life forms in each generation
const POPULATION_SIZE = 30;

// The amount of time the generation should run for (number of iterations)
const GENERATION_LIFETIME = 6000;

// Water type
const RESOURCE_WATER = 1;
const RESOURCE_FOOD = 2;

// Maximum a lifeform can carry
const MAX_FOOD = 10000.0;
const MAX_WATER = 10000.0;

// How much food and water is lost on each move
const FOODUSED_PER_STEP = 1;
const WATERUSED_PER_STEP = 1;

// How much food and water is gained by eating and drinking
const WATERDRANK_PER_STEP = WATERUSED_PER_STEP * 3;  // Drink 3 steps worth
const FOODATE_PER_DOT = FOODUSED_PER_STEP * 250;    // Eat 50 steps worth  

// Number of steps allowed on startup before dieing (used to calculate food and water level)
const START_STEPS = 1500;
// How far you can turn in degrees maximum in one go
const MAX_TURN_SPEED = 0.3;
// Number of alphas to push forward to the next generation
const NUM_ALPHAS = 4;
// Number of food blobs on screen
const NUM_FOOD_ON_BOARD = 20;
// The cross-over rate used in the GA
const CROSS_OVER_RATE = 0.7;
// Mutation rate
const MUTATION_RATE = 0.1;
// The amount of mutation that can happen
const MUTATION_AMOUNT = 0.3;