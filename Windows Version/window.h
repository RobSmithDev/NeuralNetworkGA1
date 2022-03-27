/*********************************************************************
 * Neural Network with Genetic Algorithms Demonstration              *
 * Copyright (C) 2002 RobSmithDev                                    *
 * https://robsmithdev.co.uk                                         *
 *                                                                   *
 * For more information about this project please see the video at:  *
 * https://www.youtube.com/watch?v=bq3FdlUeOTU                       *
 *********************************************************************/
#pragma once

class Simulation;

#include "Simulation.h"

#include <thread>
#include <vector>
#include <chrono>

// Main window
class CMainWindow {
private:
	HINSTANCE		m_hInstance;
	HWND			m_hWnd = 0;
	Simulation*		m_simulation = nullptr;
	HBITMAP			m_canvas = 0;
	HDC				m_canvasDC = 0;
	HGDIOBJ			m_oldBitmap = 0;
	SIZE			m_lastBitmapSize = { 0,0 };
	HBITMAP			m_background = 0;
	HDC				m_backgroundDC = 0;
	HGDIOBJ			m_backgroundDCOld = 0;

	HPEN			m_cellPen, m_sunPen, m_sandPen;
	HPEN			m_cellPenTarget, m_sunPenTarget, m_sandPenTarget, m_lifeformTarget;
	HBRUSH			m_cellBrush, m_sunBrush, m_sandBrush;

	HPEN			m_normalRobotPen, m_alphaRobotPen, m_deadRobotPen, m_shieldPen;
	HBRUSH			m_normalRobotBrush, m_alphaRobotBrush, m_deadRobotBrush, m_shieldBrush;

	HPEN			m_graphLifeform, m_graphAvFitness, m_graphIterations;

	HPEN			m_thickWhitePen, m_blackPen;
	HBRUSH			m_darkGray, m_buttonGray, m_deadBrush, m_darkerGray;

	HFONT			m_font;
	HFONT			m_fontVert;
	HGDIOBJ			m_oldFont	 =0;
	HGDIOBJ			m_oldPen	 =0;
	HGDIOBJ			m_oldBrush   =0;
	int				m_iterationSkipSpeed = 1;		// How many iterations to run in one go
	int				m_speed = 0;					// How many steps to run in one go
	int				m_generation = 1;
	int				m_ticksPerSecond = 0;
	
	int				m_tickCounter = 0;
	std::chrono::time_point<std::chrono::steady_clock> m_tickCounterStart;
	bool			m_appTerminating = false;

	GenStatistics		m_lastStatistics;
	std::vector<GenStatistics> m_statistics;


	// Button positions
	RECT			m_speedPlus = { 0 }, m_speedMinus = { 0 }, m_genSpeedPlus = { 0 }, m_genSpeedMinus = { 0 };

	std::thread*	m_simulationThread = nullptr;

	// Render the line with screen-wrap
	void renderLine(int positionX, int positionY, int targetX, int targetY, HPEN pen);

	// Prepare the window class
	bool registerWindowClass();

	// Create a window at a given size
	bool createWindow(int clientWidth, int clientHeight);

	// Free the window created above
	void freeWindowAndClass();

	// Redraew the simulation to the off-screen bitmap and render it to the window
	void redrawSimulation(HWND hWnd);

	// Free the offscreen bitmap
	void freeOffscreenBitmap();

	// Create the offscreen bitmap if it doesnt exist or has changed size
	bool checkCreateBitmap(HDC dc, int width, int height);

	// Request drawing to the internal bitmap
	void internalDraw(RECT* clientRect);

	// Run the simulation
	void runSimulation();

	// Save the geenration file to disk
	void saveGeneration(unsigned int generation, const GenStatistics& lastGeneration);

	// Load the geenration file from disk
	void loadGeneration(unsigned int generation, GenStatistics& lastGeneration);

public:
	CMainWindow(HINSTANCE hInstance);
	~CMainWindow();

	// Returns the application exit code
	int run();

	// Callback for the window proc - a little hacky but will do for now
	LRESULT handleWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};