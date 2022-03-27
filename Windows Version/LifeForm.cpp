/*********************************************************************
 * Neural Network with Genetic Algorithms Demonstration              *
 * Copyright (C) 2002 RobSmithDev                                    *
 * https://robsmithdev.co.uk                                         *
 *                                                                   *
 * For more information about this project please see the video at:  *
 * https://www.youtube.com/watch?v=bq3FdlUeOTU                       *
 *********************************************************************/

#include "Simulation.h"
#include "NeuralNetwork.h"
#include "LifeForm.h"


LifeForm::LifeForm(NeuralNetwork* brain, Simulation* simulation, int index) : m_brain(brain), m_simulation(simulation), m_index(index)  {
	reset();
}

// Reset the position, age and resources without affecting the brain
void LifeForm::resetAge() {
	m_angle = (float)((rand() * M_PI * 2.0f) / (float)RAND_MAX);
	m_lastMovement.x = (float)cos(m_angle);
	m_lastMovement.y = (float)sin(m_angle);

	// Choose a starting position where there are no resources under it
	m_simulation->getRandomPosition(m_position);
	m_lastPosition = m_position;
	m_targetCell.target = m_position;
#ifdef USE_SOLAR
	m_targetSun.target = m_position;
	m_resources.sun = SUN_USED_PER_STEP * INITIAL_STEPS;
#endif
#ifdef HAS_QUICKSAND
	m_targetSand.target = m_position;
#endif
#ifdef TRACK_OTHERS
	m_resourceIndex = -1;				
	m_otherCompetitorFound = false;    
	m_wasShieldActive = false;
#endif

	m_lifeSpan = 0;
	m_resources.cell = CELL_USED_PER_STEP * INITIAL_STEPS;
}

// Resets the lifeform back to totally random initial state
void LifeForm::reset() {
	resetAge();
	m_brain->randomize();
}

// Calculate a score on how well this lifeform did.
// It is rewarded on its age, and then by available resources
void LifeForm::calculateFitness() {
#ifdef USE_SOLAR	
#ifdef USE_MULTIPLY_FUNCTION
	m_fitnessValue = (m_lifeSpan * 2.0f / GENERATION_LIFESPAN) + (((float)m_resources.sun / (float)MAX_SUN) * ((float)m_resources.cell / (float)MAX_CELL));
#else
	m_fitnessValue = (m_lifeSpan * 2.0f / GENERATION_LIFESPAN) + (((float)m_resources.sun / (float)MAX_SUN) + ((float)m_resources.cell / (float)MAX_CELL));
#endif
#else
	m_fitnessValue = (m_lifeSpan * 2.0f / GENERATION_LIFESPAN) + ((float)m_resources.cell / (float)MAX_CELL);
#endif
}

// Return the current fitness value
float LifeForm::getFitness() {
	return m_fitnessValue < 0 ? 0 : m_fitnessValue;
}

// Run the lifeform 1 entire iteration.  Returns TRUE if the lifeform is still living
bool LifeForm::step() {
	// Stop of they're 'dead'
	if (!isAlive()) return false;

	// Track usage
#ifdef USE_SOLAR
	m_resources.sun -= SUN_USED_PER_STEP;
#endif
	m_resources.cell -= CELL_USED_PER_STEP;
	m_lifeSpan++;

	// Get some basic input from the simulation about available resources
	m_lastPosition = m_position;
	m_simulation->findDirectionToResources(m_lastPosition, m_targetCell
#ifdef USE_SOLAR 
		, m_targetSun
#endif	
#ifdef HAS_QUICKSAND 
		, m_targetSand
#endif	
#ifdef TRACK_OTHERS
		, m_resourceIndex
		, m_index
#endif
	);

	// Pass in the direction we actually moved last time
	m_brain->setInput(0, m_lastMovement.x);
	m_brain->setInput(1, m_lastMovement.y);

	// Update the 'brain' with this information.  The Directions are basically normalised vector directions
	m_brain->setInput(2, m_targetCell.direction.x);
	m_brain->setInput(3, m_targetCell.direction.y);

	// Pass in details about what resources it has left
	m_brain->setInput(4, (float)m_resources.cell / (float)MAX_CELL);

#ifdef USE_SOLAR
	m_brain->setInput(5, m_targetSun.direction.x);
	m_brain->setInput(6, m_targetSun.direction.y);
	float sun = ((float)m_resources.sun / (float)MAX_SUN)/2;
	float battery = ((float)m_resources.cell / (float)MAX_CELL)/2;
	if (sun>battery)
		m_brain->setInput(4, 0.5f + sun); else m_brain->setInput(4, 0.5f - battery);
	
#else
#ifdef HAS_QUICKSAND
	m_brain->setInput(5, m_targetSand.direction.x);
	m_brain->setInput(6, m_targetSand.direction.y);
	m_brain->setInput(7, m_quickSandUnderLifeform ? 1.0f : 0.0f);
#else
#ifdef TRACK_OTHERS
	// Get the direction to the person also after the same resource as us
	m_brain->setInput(5, m_simulation->isResourceTargettedByAnother(this, m_resourceIndex) ? 1.0f : 0.0f);
	m_brain->setInput(6, m_wasShieldActive ? 1.0f : 0.0f);
#endif
#endif
#endif

	// Make the brain update
	m_brain->update();

	// Take the outputs and update our position.  The two outputs represent the speed of the left and right 'feet' 
	float angleChange = m_brain->value(1) - m_brain->value(0);
	if (angleChange < -MAX_TURN_SPEED) angleChange = -MAX_TURN_SPEED;
	if (angleChange > MAX_TURN_SPEED) angleChange = MAX_TURN_SPEED;

	// Update the angle we're facing
	m_angle += angleChange;
	float speedStep = m_brain->value(1) + m_brain->value(0);

	m_lastMovement.x = (float)cos(m_angle);
	m_lastMovement.y = (float)sin(m_angle);

#ifdef TRACK_OTHERS
	m_wasShieldActive = false;
	if ((m_brain->value(2) > m_brain->value(3)) && (m_resources.cell > (float)(MAX_CELL/3))) {
		// Shield reduces power faster
		if (m_simulation->shieldResource(m_resourceIndex, m_index)) {
			m_resources.cell -= 2;
		}
		m_wasShieldActive = true;
	}
	else m_simulation->releaseShield(m_index);
#endif

#ifdef HAS_QUICKSAND
	// It slows us down
	if (m_quickSandUnderLifeform) speedStep *= 0.25;
#endif

	m_position.x += m_lastMovement.x * speedStep;
	m_position.y += m_lastMovement.y * speedStep;

	// The world wraps, if we go off an edge we appear on the opposite side
	m_simulation->wrapCoordinates(m_position);

#ifdef HAS_QUICKSAND
	m_quickSandUnderLifeform = false;
#endif

	// Check the new position for resources.  The last parameter says we want to 'consume' it
	switch (m_simulation->resourceTypeAtPosition(m_position, true, -1, m_index)) {

		// Did we land on oil?
		case ResourceType::rtCell:
			m_resources.cell += CELL_GAINED_WHEN_EATEN;
			if (m_resources.cell > MAX_CELL) m_resources.cell = MAX_CELL;
			break;

#ifdef HAS_QUICKSAND
		case ResourceType::rtQuickSand:
			m_quickSandUnderLifeform = true;
			break;
#endif

#ifdef USE_SOLAR
		// How about sunlight?
		case ResourceType::rtSunlight:
			m_resources.sun += SUN_GAINED_WHEN_DRANK;
			if (m_resources.sun > MAX_SUN) m_resources.sun = MAX_SUN;
			break;
#endif
	}

#ifdef TRACK_OTHERS
	if (!isAlive()) {
		m_simulation->releaseShield(m_index);
		return false;
	}
#endif


	return true;
}

// Returns TRUE if the lifeform is still alive
bool LifeForm::isAlive() {
	return (m_resources.cell > 0) 
#ifdef USE_SOLAR
		   && (m_resources.sun > 0)
#endif		
		;
}

// Capture some data about the lifeform for drawing to the screen
void LifeForm::getDrawDetails(LifeformStatus& status) {
	status.positionX = (int)m_lastPosition.x;
	status.positionY = (int)m_lastPosition.y;
	status.angleFacing = m_angle;
	status.alive = isAlive();
	status.resources = m_resources;
#ifdef USE_SOLAR
	status.sunTarget = m_targetSun.target;
	status.sunTargetAvailable = m_targetSun.available;
#endif
#ifdef HAS_QUICKSAND
	status.sandTarget = m_targetSand.target;
	status.sandTargetAvailable = m_targetSand.available;
#endif
#ifdef TRACK_OTHERS
	status.otherCompetitorFound = m_otherCompetitorFound;
	status.otherCompetitor = m_otherCompetitor.target;
	ResourceTarget otherCompetitor;
#endif
	status.cellTarget = m_targetCell.target;
	status.cellTargetAvailable = m_targetCell.available;
}
