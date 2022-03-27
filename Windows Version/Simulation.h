/*********************************************************************
 * Neural Network with Genetic Algorithms Demonstration              *
 * Copyright (C) 2002 RobSmithDev                                    *
 * https://robsmithdev.co.uk                                         *
 *                                                                   *
 * For more information about this project please see the video at:  *
 * https://www.youtube.com/watch?v=bq3FdlUeOTU                       *
 *********************************************************************/

#pragma once

//#define FULLSCREEN

#define GENERATION_LIFESPAN				6000				// How many iterations we run for

#define INITIAL_STEPS					1000				// Initial amounts of resources
#define CELL_USED_PER_STEP				1					
#define SUN_USED_PER_STEP				1

#define CELL_GAINED_WHEN_EATEN			(CELL_USED_PER_STEP * 250)
#define SUN_GAINED_WHEN_DRANK			(SUN_USED_PER_STEP * 2)


#define MAX_SUN							10000
#define MAX_CELL						10000
#define MAX_TURN_SPEED					0.4f


// Screen size
#define SIMULATION_WIDTH		600
#define SIMULATION_HEIGHT		600
#define SIMULATION_HALFWIDTH    (SIMULATION_WIDTH/2)
#define SIMULATION_HALFHEIGHT    (SIMULATION_HEIGHT/2)

#define MAX_CELLS				20
#define POPULATION_SIZE			30 

// Number of iterations before we make the next generation (or they all die)
#define MAX_LIFESPAN			6000

// Number of the best *always* cloned into the next generation
#define NUM_ALPHAS				4

// Don't change.  Mode list
#define MODE1					1
#define MODE2					2
#define MODE3					3
#define MODE4					4

// If this is defined then data is written to disk for each generation
//#define SAVE_DATA
#ifdef SAVE_DATA
// If this is >0 then this generation number is loaded on startup
#define LOAD_GENERATION			0
#endif

// Experiment mode
// 1. Looking for Batteries
// 2. Looking for Solar Electricity and Batteries
// 3. Batteries and Quicksand
// 4. Batteries and Shields
#define EXPERIMENT_MODE			MODE1

#if EXPERIMENT_MODE == MODE1
#define NOT_CONSUMABLE		ResourceType::rtNone
#define MAX_FITNESS			3.0f		
#endif

#if EXPERIMENT_MODE == MODE2
//#define USE_MULTIPLY_FUNCTION
#define USE_SOLAR
#define NOT_CONSUMABLE		ResourceType::rtSunlight
#ifdef USE_MULTIPLY_FUNCTION
#define MAX_FITNESS			3.0f
#else
#define MAX_FITNESS			4.0f
#endif
#endif

#if EXPERIMENT_MODE == MODE3
#define HAS_QUICKSAND
#define NOT_CONSUMABLE		ResourceType::rtQuickSand
#define MAX_FITNESS			3.0f
#endif

#if EXPERIMENT_MODE == MODE4
#define NOT_CONSUMABLE		ResourceType::rtNone
#define TRACK_OTHERS
#define MAX_FITNESS			3.0f
#undef MAX_CELLS
#define MAX_CELLS			10
#endif


#ifdef USE_SOLAR
#ifdef HAS_QUICKSAND
"This mode is unsupported. Please do not choose this combination!"
#endif
#endif

// This is almost not worth doing with the small computations we're doing, the larger the brain, the more noticable this is
#ifndef _DEBUG
#define THREADDED
#endif

#include "NeuralNetwork.h"
#include "GeneticAlgorithm.h"
#include "LifeForm.h"
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <fstream>
#include <iostream>

#ifdef THREADDED
#include <windows.h>
#endif

// Output Statistics
struct GenStatistics {
	int numSurvivors		= 0;
	int numIterations		= 0;
	float totalFitness		= 0.0f;
};

// Details about resources available
struct Resource {
	ResourceType resourceType;

	// Position and size
	FloatPair position;
	int radius;
	int radiusSquared;

#ifdef TRACK_OTHERS
	int shieldedBy = -1;
#endif
};

// Tracking each lifeform
struct LifeformData {
	LifeForm* lifeForm;
	NeuralNetwork* brain;
};

class Simulation {
private:
	std::vector<Resource> m_resources;
	std::vector<LifeformData> m_lifeForms;
	GeneticAlgorithm m_geneticAlgorithm;
	int m_ageCounter = 0;

#ifdef THREADDED
	std::thread* m_thread1 = nullptr;
	std::thread* m_thread2 = nullptr;
	HANDLE m_thread1Start = 0;
	HANDLE m_thread2Start = 0;
	HANDLE m_thread1Finished = 0;
	HANDLE m_thread2Finished = 0;
	HANDLE m_terminate = 0;
	std::recursive_mutex m_resourceLock;
#endif

	// Calculate the vlaues required to locate the coordinates (target) in the variable passed
	// This will re-code the position if the nearest is to actually wrap around the edge of the screen
	void calculateBrainDestination(const FloatPair& position, ResourceTarget& resource) {
		if (!resource.available) {
			resource.direction.x = 0;
			resource.direction.y = 0;
			return;
		}

		// If it takes more than half the width its quicker to go the other direction
		int movementX = (int)(position.x - resource.target.x);
		if (abs(movementX) > halfWidth()) {
			resource.target.x = (movementX > 0) ? (resource.target.x + width()) : (resource.target.x - width());
			movementX = (int)(position.x - resource.target.x);
		}

		// Repeat for the 'y' direction
		int movementY = (int)(position.y - resource.target.y);
		if (abs(movementY) > halfHeight()) {
			resource.target.y = (movementY > 0) ? (resource.target.y + height()) : (resource.target.y - height());
			movementY = (int)(position.y - resource.target.y);
		}

		// Calculate direction required
		float length = (float)sqrt((movementX * movementX) + (movementY * movementY));
		if (length < 0.1f) length = 0.1f;
		resource.direction.x = -(float)movementX / length;
		resource.direction.y = -(float)movementY / length;
	}

	// Add a resource into the system
	void addResource(FloatPair position, int radius, ResourceType rt) {
		Resource r;
		r.position = position;
		r.radius = radius;
		r.radiusSquared = radius * radius;
		r.resourceType = rt;
		m_resources.push_back(r);
	}

public:

	// Prepare the simulation with the resources
	Simulation() : m_geneticAlgorithm(NUM_ALPHAS, 0.7f, 0.1f, 0.3f) {	
		int size = width() < height() ? width() : height();

#ifdef USE_SOLAR
		// Add two spots of sunlight
		addResource({ 0.3f * width(), 0.2f * height() }, (int)(0.1f * size), ResourceType::rtSunlight);
		addResource({ 0.9f * width(), 0.9f * height() }, (int)(0.2f * size), ResourceType::rtSunlight);
#endif
#ifdef HAS_QUICKSAND
		// Add some quicksand on the map, one smack bang in the middle
		addResource({ 0.5f * width(), 0.3f * height() }, (int)(0.15f * size), ResourceType::rtQuickSand);
		addResource({ 0.5f * width(), 0.5f * height() }, (int)(0.15f * size), ResourceType::rtQuickSand);
		addResource({ 0.5f * width(), 0.7f * height() }, (int)(0.15f * size), ResourceType::rtQuickSand);
#endif

		// Add some oil drops
		for (int counter = 0; counter < MAX_CELLS; counter++) {
			FloatPair pos;
			getRandomPosition(pos);			
			addResource(pos, (int)(0.012f * size), ResourceType::rtCell);
		}

		// Finally create some lifeforms
		for (int counter = 0; counter < POPULATION_SIZE; counter++) {
			LifeformData data;
			std::vector<size_t> networkLayers;
#if EXPERIMENT_MODE == MODE1
			networkLayers = { 5, 14, 12, 2 };
#endif
#if EXPERIMENT_MODE == MODE2
			networkLayers = { 8, 14, 12, 2 };
#endif
#if EXPERIMENT_MODE == MODE3
			networkLayers = { 8, 14, 12, 2 };
#endif
#if EXPERIMENT_MODE == MODE4
			networkLayers = { 7, 16, 14, 10, 4 };
#endif
#ifdef LARGER_BRAIN
			// More hidden layers *can* increase intellegence
			networkLayers.insert(networkLayers.begin() + 3, 8);
#endif
			data.brain = new NeuralNetwork(networkLayers);
			data.lifeForm = new LifeForm(data.brain, this, counter);
			m_lifeForms.push_back(data);
		}

#ifdef THREADDED
		m_thread1Start = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_thread2Start = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_thread1Finished = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_thread2Finished = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_terminate = CreateEvent(NULL, TRUE, FALSE, NULL);
		size_t midPoint = POPULATION_SIZE / 2;
		
		m_thread1 = new std::thread([this, midPoint]() {
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			HANDLE events[2] = { m_thread1Start, m_terminate };
			for (;;) {
				DWORD result = WaitForMultipleObjects(2, events, FALSE, INFINITE);
				if (result == WAIT_OBJECT_0 + 1) return;
				ResetEvent(m_thread1Start);

				for (size_t index = 0; index < midPoint; index++)
					m_lifeForms[index].lifeForm->step();

				SetEvent(m_thread1Finished);
			}
		});
		m_thread2 = new std::thread([this, midPoint]() {
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			HANDLE events[2] = { m_thread2Start, m_terminate };
			for (;;) {
				DWORD result = WaitForMultipleObjects(2, events, FALSE, INFINITE);
				if (result == WAIT_OBJECT_0 + 1) return;
				ResetEvent(m_thread2Start);

				for (size_t index = midPoint; index < m_lifeForms.size(); index++)
					m_lifeForms[index].lifeForm->step();

				SetEvent(m_thread2Finished);
			}
		});
#endif
	}

	// Free
	~Simulation() {
#ifdef THREADDED
		SetEvent(m_terminate);
		if (m_thread1) {
			if (m_thread1->joinable()) m_thread1->join();
			delete m_thread1;
		}
		if (m_thread2) {
			if (m_thread2->joinable()) m_thread2->join();
			delete m_thread2;
		}
		CloseHandle(m_thread1Finished);
		CloseHandle(m_thread1Start);
		CloseHandle(m_thread2Finished);
		CloseHandle(m_thread2Start);
#endif
		for (LifeformData& data : m_lifeForms) {
			delete data.brain;
			delete data.lifeForm;
		}
	}

	// Advance the simulation one place
	bool step() {
		int lifeforms = 0;
#ifdef THREADDED
		// Trigger start
		ResetEvent(m_thread1Finished);
		ResetEvent(m_thread2Finished);
		SetEvent(m_thread1Start);
		SetEvent(m_thread2Start);

		HANDLE threadWait[2] = {m_thread1Finished, m_thread2Finished};

		// Wait for completion
		WaitForMultipleObjects(2, threadWait, TRUE, INFINITE);

		// Count survivers
		for (LifeformData& data : m_lifeForms)
			if (data.lifeForm->isAlive()) lifeforms++;
#else

		for (LifeformData& data : m_lifeForms) {
			if (data.lifeForm->step()) lifeforms++;
		}

#endif
		m_ageCounter++;
		return (lifeforms > 0) && (m_ageCounter< MAX_LIFESPAN);
	}

#ifdef TRACK_OTHERS
	// Finds another competitor after the same resource you are and sets up the direction to them
	bool isResourceTargettedByAnother(LifeForm* requester, int resourceIndex) {
		for (const LifeformData& lifeform : m_lifeForms) 
			if ((lifeform.lifeForm != requester) && (lifeform.lifeForm->isTargetingResource(resourceIndex))) {				
				return true;
			}
		return false;
	}

	// Attempt to put a shield around a resource
	bool shieldResource(int resourceIndex, int lifeformIndex) {
		bool found = false;
#ifdef THREADDED
		m_resourceLock.lock();
#endif
		for (size_t index=0; index< m_resources.size(); index++)
			if ((m_resources[index].shieldedBy == lifeformIndex) || ((m_resources[index].shieldedBy == -1) && (index == resourceIndex))) {
				if (resourceIndex == index) {
					m_resources[index].shieldedBy = lifeformIndex;
					found = true;
				}
				else m_resources[index].shieldedBy = -1;
			}
#ifdef THREADDED
		m_resourceLock.unlock();
#endif
		return found;
	}

	// Release shield
	void releaseShield(int lifeformIndex) {
		for (size_t index = 0; index < m_resources.size(); index++)
			if (m_resources[index].shieldedBy == lifeformIndex) {
				m_resources[index].shieldedBy = -1;
			}
	}
#endif

	// Get the pixel width of the simulation
	int width() const {
		return SIMULATION_WIDTH;
	}

	// Get the pixel height of the simulation
	int height() const {
		return SIMULATION_HEIGHT;
	}

	// Get the pixel width of the simulation
	int halfWidth() const {
		return SIMULATION_HALFWIDTH;
	}

	// Get the pixel height of the simulation
	int halfHeight() const {
		return SIMULATION_HALFHEIGHT;
	}

	// Instructs the next generation to be made.  Returns the number of survivors from the current gneration
	// stats is information about the current generation
	void produceNextGeneration(GenStatistics& stats) {
		stats.numSurvivors = 0;
		stats.numIterations = m_ageCounter;
		stats.totalFitness = 0;
		
		// Step 1: Extract the brains from our lifeforms
		std::vector< NetworkWeightFitness > brains;

		for (LifeformData& data : m_lifeForms) {
			data.lifeForm->calculateFitness();
			if (data.lifeForm->isAlive()) stats.numSurvivors++;

			NetworkWeightFitness brain;
			brain.network = data.brain;
			brain.fitness = data.lifeForm->getFitness();
			stats.totalFitness += brain.fitness;
			brains.push_back(brain);
		}

		// Step 2: Pass into the Genetic Algorithm
		m_geneticAlgorithm.produceNextGeneration(brains);

		// Step 3: Re-program the brains and reset them
		for (LifeformData& data : m_lifeForms)
			data.lifeForm->resetAge();

		// Step 4: Reset
		m_ageCounter = 0;

		// Step 5: Reset shields
#ifdef TRACK_OTHERS
		for (Resource& r : m_resources) r.shieldedBy = -1;
#endif
	}

	// Get the current age of the simulation
	int currentAge() const {
		return m_ageCounter;
	}

	// Return what resource was found at a specific coordinate
	ResourceType resourceTypeAtPosition(const FloatPair& position, bool consumeResource, int indexToIgnore = -1, int mustBelongTo = -1) {
		// Iterate
#ifdef THREADDED
		m_resourceLock.lock();
#endif
		for (size_t index = 0; index < m_resources.size(); index++) {
			if (index == indexToIgnore) continue;
			// Skip a resource if its shielded by another lifeform
#ifdef TRACK_OTHERS
			if ((m_resources[index].shieldedBy >= 0) && (m_resources[index].shieldedBy != mustBelongTo) && (mustBelongTo >= 0)) continue;
#endif
			// Calculate the distance away
			int distanceX = (int)(position.x - m_resources[index].position.x);
			int distanceY = (int)(position.y - m_resources[index].position.y);

			// Use everything squared rather than calling sqrt which isnt the fastest thing in the world
			if (sqrt((distanceX * distanceX) + (distanceY * distanceY)) <= m_resources[index].radius) {
				if (consumeResource && (m_resources[index].resourceType != NOT_CONSUMABLE)) {
					// Re-spawn (well, just move it to a new position. But its the same idea)
					getRandomPosition(m_resources[index].position, (int)index);
#ifdef TRACK_OTHERS
					m_resources[index].shieldedBy = -1;
#endif
				}
#ifdef THREADDED
				m_resourceLock.unlock();
#endif
				return m_resources[index].resourceType;
			}
		}

#ifdef THREADDED
		m_resourceLock.unlock();
#endif
		return ResourceType::rtNone;
	}

	// Updates position with a random position where there are no resources
	void getRandomPosition(FloatPair& position, int indexToIgnore = -1) {
		do {
			position.x = (width() * 0.1f) + ((width() * rand() * 0.8f) / RAND_MAX);
			position.y = (height() * 0.1f) + ((height() * rand() * 0.8f) / RAND_MAX);
		} while (resourceTypeAtPosition(position, false, indexToIgnore) != ResourceType::rtNone);
	}
	// rtSunlight, rtOil, rtQuickSand
	// Calculates the distances etc to the nearest of each type of resource
	void findDirectionToResources(const FloatPair& position, ResourceTarget& targetCell
#ifdef USE_SOLAR				
		, ResourceTarget& targetSunlight
#endif
#ifdef HAS_QUICKSAND
		, ResourceTarget& targetQuickSand
#endif
#ifdef TRACK_OTHERS
		, int& resourceIndex
		, int callerIndex
#endif
	) {
		// Reset 
#ifdef USE_SOLAR	
		int nearestSunIndex = -1;
		int nearestSunValue = 0;
#endif
#ifdef HAS_QUICKSAND
		int nearestSandIndex = -1;
		int nearestSandValue = 0;
#endif
		int nearestCellIndex = -1;
		int nearestCellValue = 0;

		// Iterate
		for (size_t index = 0; index < m_resources.size(); index++) {
#ifdef TRACK_OTHERS
			// iF the resource is shielded by another robot, then it's invisible to us
			if ((m_resources[index].shieldedBy >= 0) && (m_resources[index].shieldedBy != callerIndex)) continue;
#endif
			// Calculate the distance away
			int distanceX = (int)abs(position.x - (int)m_resources[index].position.x);
			int distanceY = (int)abs(position.y - (int)m_resources[index].position.y);
			if (distanceX > halfWidth()) distanceX = width() - distanceX;
			if (distanceY > halfHeight()) distanceY = height() - distanceY;

			int distance = ((distanceX * distanceX) + (distanceY * distanceY)) - m_resources[index].radiusSquared;
			if (distance < 0) distance = 0;

			// Check the resource type and keep the nearest
			switch (m_resources[index].resourceType) {
#ifdef USE_SOLAR	
			case ResourceType::rtSunlight:
				if ((distance < nearestSunValue) || (nearestSunIndex == -1)) {
					nearestSunIndex = (int)index;
					nearestSunValue = distance;
				}
				break;
#endif
			case ResourceType::rtCell:
				if ((distance < nearestCellValue) || (nearestCellIndex == -1)) {
					nearestCellIndex = (int)index;
					nearestCellValue = distance;
				}
				break;

#ifdef HAS_QUICKSAND
			case ResourceType::rtQuickSand:
				if ((distance < nearestSandValue) || (nearestSandIndex == -1)) {
					nearestSandIndex = (int)index;
					nearestSandValue = distance;
				}
				break;
#endif
			}
		}

#ifdef USE_SOLAR	
		// Calculate the target and direction for the items
		targetSunlight.available = nearestSunIndex >= 0;
		if (targetSunlight.available) {
			targetSunlight.target = m_resources[nearestSunIndex].position;
			calculateBrainDestination(position, targetSunlight);
		}
#endif

#ifdef TRACK_OTHERS
		resourceIndex = nearestCellIndex;
#endif
		targetCell.available = nearestCellIndex >= 0;
		if (targetCell.available) {
			targetCell.target = m_resources[nearestCellIndex].position;
			calculateBrainDestination(position, targetCell);
		}

#ifdef HAS_QUICKSAND
		targetQuickSand.available = nearestSandIndex >= 0;
		if (targetQuickSand.available) {
			targetQuickSand.target = m_resources[nearestSandIndex].position;
			calculateBrainDestination(position, targetQuickSand);
		}
#endif
	}

	// Wraps the coordinates so if they go off one edge of the simulation they appear on the other side
	void wrapCoordinates(FloatPair& position) {
		if (position.x < 0) position.x += width(); else
			if (position.x >= width()) position.x -= width();
		if (position.y < 0) position.y += height(); else
			if (position.y >= height()) position.y -= height();
	}

	// Draw the resources.  It iterates resources and calls the callback
	void drawResources(std::function<void(const Resource& resource)> onDraw) {
		for (const Resource& r : m_resources) onDraw(r);
	}

	// Draw the lifeforms.  It iterates resources and calls the callback
	void drawLifeforms(std::function<void(const LifeformData& lifeform)> onDraw) {
		for (const LifeformData& r : m_lifeForms) onDraw(r);
	}

	// Snapshot to disk
	bool saveSnapshot(const std::wstring& filename, unsigned int generation, const GenStatistics& lastGeneration) {
		std::fstream file = std::fstream(filename, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);		
		if (!file.is_open()) return false;

		// Save generation number
		if (!file.write((const char*)&generation, sizeof(generation))) return false;
		if (!file.write((const char*)&lastGeneration, sizeof(lastGeneration))) return false;
		unsigned int total = 0;
		

		// Weights
		std::vector<float> weights;
		for (LifeformData& data : m_lifeForms) {
			data.brain->getWeights(weights);
			if (total == 0) total = (unsigned int)weights.size();
		}

		// Save number of weights per brain
		if (!file.write((const char*)&total, sizeof(total))) return false;

		// Save
		file.write((const char*)weights.data(), sizeof(float) * weights.size());
		file.close();
		return true;
	}

	// Load from disk
	bool loadSnapshot(const std::wstring& filename, unsigned int& generationLoaded, GenStatistics& lastGeneration) {
		std::fstream file = std::fstream(filename, std::ofstream::in | std::ofstream::binary);
		if (!file.is_open()) return false;

		// Load generation number
		if (!file.read((char*)&generationLoaded, sizeof(generationLoaded))) return false;
		if (!file.read((char*)&lastGeneration, sizeof(lastGeneration))) return false;
		unsigned int total = 0;

		if (!file.read((char*)&total, sizeof(total))) return false;

		std::vector<float> weights;
		weights.resize(total * POPULATION_SIZE);
		if (!file.read((char*)weights.data(), sizeof(float) * weights.size())) return false;

		// Load
		for (LifeformData& data : m_lifeForms) {
			weights.erase(weights.begin(), weights.begin() + data.brain->setWeights(weights));
			data.lifeForm->resetAge();
		}

		file.close();
		return true;
	}

};