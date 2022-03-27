/*********************************************************************
 * Neural Network with Genetic Algorithms Demonstration              *
 * Copyright (C) 2002 RobSmithDev                                    *
 * https://robsmithdev.co.uk                                         *
 *                                                                   *
 * For more information about this project please see the video at:  *
 * https://www.youtube.com/watch?v=bq3FdlUeOTU                       *
 *********************************************************************/

#pragma once


enum class ResourceType { rtNone, rtCell, rtSunlight, rtQuickSand };

// Struct to hold two floats
struct FloatPair {
	float x = 0;
	float y = 0;
};

struct Resources {
#ifdef USE_SOLAR
	int sun = 0;
#endif
	int cell = 0;
};

// Details about where a resource is and how far away
struct ResourceTarget {
	FloatPair target;
	FloatPair direction;
	bool available;
};

// Information used when displaying it
struct LifeformStatus {
	// Position and direction of the lifeform
	int positionX, positionY;
	float angleFacing;

	// Available resources 
	Resources resources;

#ifdef USE_SOLAR
	FloatPair sunTarget;
	bool sunTargetAvailable;
#endif

	FloatPair cellTarget;
	bool cellTargetAvailable;

#ifdef HAS_QUICKSAND
	FloatPair sandTarget;
	bool sandTargetAvailable;
#endif

#ifdef TRACK_OTHERS
	bool otherCompetitorFound;
	FloatPair otherCompetitor;
#endif

	// Status!
	bool alive;
};

// We'll define these elsewhere!
class Simulation;
class NeuralNetwork;

// Main lifeform class
class LifeForm {
private:
	Simulation* m_simulation;				// The simulation this is part of
	NeuralNetwork* m_brain;					// Neural Network brain

	FloatPair m_position;					// Current position
	FloatPair m_lastPosition;				// Last position
	FloatPair m_lastMovement;				// Last movement
	float m_angle = 0;						// Current angle
	int m_index;							// Our index
	
	unsigned int m_lifeSpan = 0;			// How long this has been alive for in 'iterations'

	float m_fitnessValue = 0;				// Last calculated fitness value

	Resources m_resources;					// What resources they have

#ifdef USE_SOLAR
	ResourceTarget m_targetSun;				// Where the sun is thats closest
#endif
#ifdef HAS_QUICKSAND
	ResourceTarget m_targetSand;			// Where the quicksand is
	bool m_quickSandUnderLifeform = false;  // Is there quicksand under us
#endif
#ifdef TRACK_OTHERS
	int m_resourceIndex = -1;				// Index of the resource that's nearest
	bool m_otherCompetitorFound = false;    // If another competitor is valid
	bool m_wasShieldActive = false;
	ResourceTarget m_otherCompetitor;
#endif
	ResourceTarget m_targetCell;			// Where the cell is thats closest

public:
	LifeForm(NeuralNetwork* brain, Simulation* simulation, int index);

#ifdef TRACK_OTHERS
	// Returns TRUE where this resource is being targetted by this lifeform
	bool isTargetingResource(const int index) { return index == m_resourceIndex; };
#endif

	// Return the position of this lifeform
	const FloatPair& position() { return m_position; };

	// Reset the position, age and resources without affecting the brain
	void resetAge();

	// Resets the lifeform back to totally random initial state
	void reset();

	// Calculate a score on how well this lifeform did.
	// It is rewarded on its age, and then by available resources
	void calculateFitness();

	// Return the current fitness value
	float getFitness();

	// Run the lifeform 1 entire iteration.  Returns TRUE if the lifeform is still living
	bool step();

	// Capture some data about the lifeform for drawing to the screen
	void getDrawDetails(LifeformStatus& status);

	// Returns TRUE if the lifeform is still alive
	bool isAlive();
};