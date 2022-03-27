/*********************************************************************
 * Neural Network with Genetic Algorithms Demonstration              *
 * Copyright (C) 2002 RobSmithDev                                    *
 * https://robsmithdev.co.uk                                         *
 *                                                                   *
 * For more information about this project please see the video at:  *
 * https://www.youtube.com/watch?v=bq3FdlUeOTU                       *
 *********************************************************************/



#include <Windows.h>
#include "framework.h"
#include "window.h"
#include <mmsystem.h>
#include <string>
#include <windowsx.h> 
#include <time.h>

#pragma comment(lib, "Winmm.lib")


#define WINDOW_TITLE   L"Genetic Algorithm Experiments"
#define WINDOW_CLASS   L"GACLASS"

#ifdef FULLSCREEN
#define WINDOW_STYLE   WS_POPUP 
#else
#define WINDOW_STYLE   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
#endif

#define DARK_GRAY      RGB(64, 64, 64)
#define BUTTON_GRAY    RGB(128, 128, 128)
#define ALPHA_COLOR    RGB(0, 255, 0)


// Simple WndProc forwarder
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CREATE) SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);

    CMainWindow* callback = (CMainWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (callback) {
        return callback->handleWindowProc(hWnd, message, wParam, lParam);
    }
    else {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
    
// Create me
CMainWindow::CMainWindow(HINSTANCE hInstance) : m_hInstance(hInstance) {
    srand((unsigned int)time(NULL));
       
    m_simulation = new Simulation();
    
    m_cellPen = CreatePen(PS_SOLID, 1, RGB(255/2, 10/2, 10/2));
    m_sunPen = CreatePen(PS_SOLID, 1, RGB(255/2, 255/2, 128/2));
    m_sandPen = CreatePen(PS_SOLID, 1, RGB(255/2, 128/2, 0/2));

    m_cellPenTarget = CreatePen(PS_DOT, 1, RGB(255 / 2, 10 / 2, 10 / 2));
    m_sunPenTarget = CreatePen(PS_DOT, 1, RGB(255 / 2, 255 / 2, 128 / 2));
    m_sandPenTarget = CreatePen(PS_DOT, 1, RGB(255 / 2, 128 / 2, 0 / 2));
    m_lifeformTarget = CreatePen(PS_DOT, 1, RGB(128, 128, 0));

    m_cellBrush = CreateSolidBrush(RGB(255, 10, 10));
    m_sunBrush = CreateSolidBrush(RGB(255, 255, 128));
    m_sandBrush = CreateHatchBrush(HS_DIAGCROSS, RGB(255, 128, 0));

    m_shieldBrush = CreateSolidBrush(RGB(0, 255, 255));
    m_shieldPen = CreatePen(PS_SOLID, 1, RGB(0, 128, 128));

    m_normalRobotPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255/2));
    m_alphaRobotPen = CreatePen(PS_SOLID, 1, RGB(0, 255/2, 0));
    m_deadRobotPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 64/2));

    m_normalRobotBrush = CreateSolidBrush(RGB(0, 0, 255));
    m_alphaRobotBrush = CreateSolidBrush(ALPHA_COLOR);
    m_deadRobotBrush = CreateSolidBrush(RGB(0, 0, 64));

    m_thickWhitePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    m_buttonGray = CreateSolidBrush(BUTTON_GRAY);
    m_blackPen = CreatePen(PS_SOLID, 0, RGB(0, 0, 0));
    m_darkGray = CreateSolidBrush(DARK_GRAY);
    m_deadBrush = CreateSolidBrush(RGB(32, 32, 32));
    m_darkerGray = CreateSolidBrush(RGB(16, 16, 16));

    m_graphLifeform = CreatePen(PS_SOLID, 1, RGB(64, 64, 255));
    m_graphAvFitness = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
    m_graphIterations = CreatePen(PS_SOLID, 1, RGB(255, 0, 255));

    HDC dc = GetDC(GetDesktopWindow());
    m_font = CreateFont(-MulDiv(10, GetDeviceCaps(dc, LOGPIXELSY), 72), 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Franklin Gothic Medium");
    m_fontVert = CreateFont(-MulDiv(8, GetDeviceCaps(dc, LOGPIXELSY), 72), 0, -900, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Franklin Gothic Medium");

#ifdef FULLSCREEN
    char buffer[20];
    sprintf_s(buffer, "bg%i.bmp", EXPERIMENT_MODE);
    m_background = (HBITMAP)LoadImageA(NULL, buffer, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    if (m_background != 0) {
        m_backgroundDC = CreateCompatibleDC(dc);
        m_backgroundDCOld = SelectObject(m_backgroundDC, m_background);
    }
    ReleaseDC(GetDesktopWindow(), dc);
#endif

    // Jump to generation LOAD_GENERATION
#ifdef SAVE_DATA
    if (LOAD_GENERATION > 0)
        for (size_t generation = 1; generation <= LOAD_GENERATION; generation++) {
            GenStatistics stats;
            loadGeneration(generation, stats);
            m_statistics.push_back(stats);
        }
#endif
}

// Free me
CMainWindow::~CMainWindow() {
    if (m_backgroundDCOld) SelectObject(m_backgroundDC, m_backgroundDCOld);
    if (m_backgroundDC) DeleteDC(m_backgroundDC);
    if (m_background) DeleteObject(m_background);

    DeleteObject(m_graphLifeform);
    DeleteObject(m_graphAvFitness);
    DeleteObject(m_graphIterations);
    DeleteObject(m_fontVert);
    DeleteObject(m_lifeformTarget);
    DeleteObject(m_darkerGray);    
    DeleteObject(m_simulation);
    DeleteObject(m_cellPen);
    DeleteObject(m_sunPen);
    DeleteObject(m_sandPen);
    DeleteObject(m_cellPenTarget);
    DeleteObject(m_sunPenTarget);
    DeleteObject(m_sandPenTarget);
    DeleteObject(m_cellBrush);
    DeleteObject(m_sunBrush);
    DeleteObject(m_shieldBrush);
    DeleteObject(m_shieldPen);
    DeleteObject(m_sandBrush);
    DeleteObject(m_normalRobotPen);
    DeleteObject(m_alphaRobotPen);
    DeleteObject(m_deadRobotPen);
    DeleteObject(m_normalRobotBrush);
    DeleteObject(m_alphaRobotBrush);
    DeleteObject(m_deadRobotBrush);
    DeleteObject(m_thickWhitePen);
    DeleteObject(m_buttonGray);
    DeleteObject(m_blackPen);
    DeleteObject(m_darkGray);
    DeleteObject(m_deadBrush);
    DeleteObject(m_font);

    delete m_simulation;
}

// Register the class for the window
bool CMainWindow::registerWindowClass() {
    WNDCLASSEXW wcex;

    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = m_hInstance;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = WINDOW_CLASS;

    return RegisterClassExW(&wcex) != 0;
}

// Create the window
bool CMainWindow::createWindow(int clientWidth, int clientHeight) {
    // Create the window centered on screen with the required client size
    RECT rect = { 0, 0, clientWidth, clientHeight };
    AdjustWindowRect(&rect, WINDOW_STYLE, false);
    const int offsetX = (GetSystemMetrics(SM_CXSCREEN) - (rect.right - rect.left)) / 2;
    const int offsetY = (GetSystemMetrics(SM_CYSCREEN) - (rect.bottom - rect.top)) / 2;
    OffsetRect(&rect, offsetX, offsetY);

    m_hWnd = CreateWindowExW(0, WINDOW_CLASS, WINDOW_TITLE, WINDOW_STYLE, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, m_hInstance, (LPVOID)this);
    if (!m_hWnd) return false;

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    return true;
}

// Callback for the window proc - a little hacky but will do for now
LRESULT CMainWindow::handleWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {    
    switch (message) {
        case WM_ERASEBKGND:
            return 1;

        case WM_LBUTTONDOWN: {
                POINT pt = { GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
                if (PtInRect(&m_speedMinus, pt)) m_speed = max(0, m_speed / 2);
                if (PtInRect(&m_speedPlus, pt)) if (m_speed == 0) m_speed = 1; else m_speed = min(MAX_LIFESPAN / 2, m_speed * 2);
                if (PtInRect(&m_genSpeedMinus, pt)) m_iterationSkipSpeed = max(1, m_iterationSkipSpeed-1);
                if (PtInRect(&m_genSpeedPlus, pt)) m_iterationSkipSpeed++;
            }
            break;

        case WM_RBUTTONDOWN: {
                POINT pt = { GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) };
                if (PtInRect(&m_speedMinus, pt)) m_speed = max(0, m_speed / 5);
                if (PtInRect(&m_speedPlus, pt)) if (m_speed == 0) m_speed = 5; else m_speed = min(MAX_LIFESPAN / 2, m_speed * 5);
                if (PtInRect(&m_genSpeedMinus, pt)) m_iterationSkipSpeed = max(1, m_iterationSkipSpeed - 5);
                if (PtInRect(&m_genSpeedPlus, pt)) m_iterationSkipSpeed+=5;
            }
            break;

        case WM_PAINT: {   
                PAINTSTRUCT p1;
                BeginPaint(hWnd, &p1);
                // Do nothing.  The thread will do it soon enough anyway
                EndPaint(hWnd, &p1);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;        
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

// Save the geenration file to disk
void CMainWindow::saveGeneration(unsigned int generation, const GenStatistics& lastGeneration) {
#ifdef SAVE_DATA
    //std::string s = 
    std::wstring filename = L"output\\weights_" + std::to_wstring(EXPERIMENT_MODE) + L"_Generation_" + std::to_wstring(generation) + L".dat";
    m_simulation->saveSnapshot(filename, generation, lastGeneration);
#endif
}

// Load the geenration file from disk
void CMainWindow::loadGeneration(unsigned int generation, GenStatistics& lastGeneration) {
#ifdef SAVE_DATA
    std::wstring filename = L"output\\weights_" + std::to_wstring(EXPERIMENT_MODE) + L"_Generation_" + std::to_wstring(generation) + L".dat";
    if (m_simulation->loadSnapshot(filename, generation, lastGeneration)) {
        m_generation = (int)generation;
    }
#endif
}


// Run the simulation
void CMainWindow::runSimulation() {
    if (m_iterationSkipSpeed > 1) {
        for (int generation = 1; generation <= m_iterationSkipSpeed; generation++) {
            // Run fully
            while (m_simulation->step()) { m_tickCounter++; };

            // Last one? Redraw
            if (generation == m_iterationSkipSpeed) redrawSimulation(m_hWnd);

            // Prepare the next one
            m_simulation->produceNextGeneration(m_lastStatistics);
            m_statistics.push_back(m_lastStatistics);

            saveGeneration(m_generation, m_lastStatistics);

            // Keep track
            m_generation++;
        }
    } else {
        // Trigger contineous updates
        bool stillAlive = true;
        if (m_speed == 0) {
            m_tickCounter++;
            stillAlive = m_simulation->step(); 
            // Slow it down!
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else {
            for (int counter = 1; counter <= m_speed; counter++) {
                m_tickCounter++;
                if (!m_simulation->step()) {
                    stillAlive = false;
                    break;
                }
            }
        }

        redrawSimulation(m_hWnd);

        // If they're all dead then make the next generation
        if (!stillAlive) {
            m_simulation->produceNextGeneration(m_lastStatistics);
            m_statistics.push_back(m_lastStatistics);
            saveGeneration(m_generation, m_lastStatistics);
            m_generation++;
        }
    }

    const std::chrono::time_point<std::chrono::steady_clock> nowTime = std::chrono::steady_clock::now();
    const long long timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - m_tickCounterStart).count();
    if (timeTaken > 1) {
        char buffer[200];
        m_ticksPerSecond = MulDiv(m_tickCounter, 1000, (int)timeTaken);
        sprintf_s(buffer, "Genetic Algorithm Experiments (%i steps per second)", m_ticksPerSecond);
        SetWindowTextA(m_hWnd, buffer);
    }
    else {
        m_ticksPerSecond = 1;
        SetWindowText(m_hWnd, L"Genetic Algorithm Experiments");
    }
    m_tickCounterStart = nowTime;
    m_tickCounter = 0;
}

// Free the window class
void CMainWindow::freeWindowAndClass() {
    if (m_hWnd) DestroyWindow(m_hWnd);
    UnregisterClassW(WINDOW_CLASS, m_hInstance);
}

// Render the line with screen-wrap
void CMainWindow::renderLine(int positionX, int positionY, int targetX, int targetY, HPEN pen) {
    SetBkMode(m_canvasDC, TRANSPARENT);
    SetBkColor(m_canvasDC, RGB(0, 0, 0));
    SelectObject(m_canvasDC, pen);
    MoveToEx(m_canvasDC, positionX, positionY, NULL);
    LineTo(m_canvasDC, targetX, targetY);

    // Hacky but quick
    if (targetX < 0) {
        MoveToEx(m_canvasDC, positionX+m_simulation->width(), positionY, NULL);
        LineTo(m_canvasDC, targetX+m_simulation->width(), targetY);
    } else
        if (targetX >= m_simulation->width()) {
            MoveToEx(m_canvasDC, positionX - m_simulation->width(), positionY, NULL);
            LineTo(m_canvasDC, targetX - m_simulation->width(), targetY);
        }
    // Hacky but quick
    if (targetY < 0) {
        MoveToEx(m_canvasDC, positionX, positionY + m_simulation->height(), NULL);
        LineTo(m_canvasDC, targetX, targetY + m_simulation->height());
    }
    else
        if (targetY >= m_simulation->height()) {
            MoveToEx(m_canvasDC, positionX, positionY - m_simulation->height(), NULL);
            LineTo(m_canvasDC, targetX, targetY - m_simulation->height());
        }
    SetBkMode(m_canvasDC, OPAQUE);
}

// Draw the simulation onto the offscreen canvas - this is a bit messy
void CMainWindow::internalDraw(RECT* clientRect) {
    // First clear the screen    
    if (!m_simulation) return;
    char buffer[100];

#ifdef FULLSCREEN
    const int yOffset = 250;
#else
    const int yOffset = 0;
#endif

    RECT r = { 0,yOffset,m_simulation->width(), yOffset + m_simulation->height() };
    FillRect(m_canvasDC, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
    r = { 5, yOffset + m_simulation->width() - 20, m_simulation->width(), yOffset + m_simulation->width() - 5 };
    SetTextColor(m_canvasDC, RGB(96, 96, 96));
    SetBkColor(m_canvasDC, RGB(0, 0, 0));
    DrawTextA(m_canvasDC, "https://youtube.com/c/RobSmithDev", -1, &r, DT_LEFT | DT_BOTTOM | DT_SINGLELINE);

    // Draw all resources, this runs callback for each one
    m_simulation->drawResources([this, &buffer, yOffset](const Resource& resource) {
        RECT r = { (LONG)(resource.position.x - resource.radius), (LONG)(resource.position.y - resource.radius) + yOffset ,
                   (LONG)(resource.position.x + resource.radius), (LONG)(resource.position.y + resource.radius) + yOffset };

#ifdef TRACK_OTHERS
        if (resource.shieldedBy>=0) {
            SelectObject(m_canvasDC, m_shieldPen);
            SelectObject(m_canvasDC, m_shieldBrush);
            Ellipse(m_canvasDC, r.left-3, r.top-3, r.right+3, r.bottom+3);           
        }
#endif

        switch (resource.resourceType) {
        case ResourceType::rtSunlight:
            SelectObject(m_canvasDC, m_sunPen);
            SelectObject(m_canvasDC, m_sunBrush);
            Ellipse(m_canvasDC, r.left, r.top, r.right, r.bottom);
            break;
        case ResourceType::rtCell:
            SelectObject(m_canvasDC, m_cellPen);
            SelectObject(m_canvasDC, m_cellBrush);
            Ellipse(m_canvasDC, r.left, r.top, r.right, r.bottom);
            break;
        case ResourceType::rtQuickSand:
            SelectObject(m_canvasDC, m_sandPen);
            SelectObject(m_canvasDC, m_sandBrush);
            SetBkMode(m_canvasDC, TRANSPARENT);
            Ellipse(m_canvasDC, r.left, r.top, r.right, r.bottom);
            SetBkMode(m_canvasDC, OPAQUE);
            break;
        }
    });

    const float lifeformSize = ( m_simulation->width() > m_simulation->height() ? m_simulation->width() : m_simulation->height() ) * 0.01f;

    const int rowHeight = 16;
    const int leftEdge = m_simulation->width();
    const int startY = 40 + yOffset;
    int yPosition = startY;


    int index = 1;
    int numAlive = 0;   


    // Draw all lifeforms, this runs callback for each one
    m_simulation->drawLifeforms([this, lifeformSize, &index, &numAlive, yOffset](const LifeformData& lifeform) {
        LifeformStatus status;
        lifeform.lifeForm->getDrawDetails(status);

        RECT r = { (LONG)(status.positionX - lifeformSize), (LONG)(status.positionY - lifeformSize) + yOffset ,
                   (LONG)(status.positionX + lifeformSize), (LONG)(status.positionY + lifeformSize) + yOffset };

        if (status.alive) {
            numAlive++;
            if ((m_generation > 1) && (index <= NUM_ALPHAS)) {
                SelectObject(m_canvasDC, m_alphaRobotPen);
                SelectObject(m_canvasDC, m_alphaRobotBrush);
            }
            else {
                SelectObject(m_canvasDC, m_normalRobotBrush);
                SelectObject(m_canvasDC, m_normalRobotPen);
            }
        }
        else {
            SelectObject(m_canvasDC, m_deadBrush);
            SelectObject(m_canvasDC, m_deadRobotPen);
        }
        Ellipse(m_canvasDC, r.left, r.top, r.right, r.bottom);

        if (status.alive) {
            // Draw direction
            SelectObject(m_canvasDC, m_thickWhitePen);
            const float radius = lifeformSize * 1.2f;
            MoveToEx(m_canvasDC, (LONG)status.positionX, (LONG)status.positionY + yOffset, NULL);
            LineTo(m_canvasDC, (LONG)(status.positionX + (cos(status.angleFacing) * radius)), (LONG)(status.positionY + yOffset + (sin(status.angleFacing) * radius)));
            MoveToEx(m_canvasDC, (LONG)status.positionX - 1, (LONG)status.positionY - 1 + yOffset, NULL);
            LineTo(m_canvasDC, (LONG)(status.positionX + (cos(status.angleFacing) * radius)), (LONG)(status.positionY + yOffset + (sin(status.angleFacing) * radius)));

            // Draw lines to the nearest resource
            if (status.cellTargetAvailable) renderLine((LONG)status.positionX, (LONG)status.positionY + yOffset, (int)status.cellTarget.x, (int)status.cellTarget.y + yOffset, m_cellPenTarget);
#ifdef USE_SOLAR
            if (status.sunTargetAvailable) renderLine((LONG)status.positionX, (LONG)status.positionY + yOffset, (int)status.sunTarget.x, (int)status.sunTarget.y + yOffset, m_sunPenTarget);
#endif
#ifdef HAS_QUICKSAND
            if (status.sandTargetAvailable) renderLine((LONG)status.positionX, (LONG)status.positionY + yOffset, (int)status.sandTarget.x, (int)status.sandTarget.y + yOffset, m_sandPenTarget);
#endif
#ifdef TRACK_OTHERS
            if (status.otherCompetitorFound)
                renderLine((LONG)status.positionX, (LONG)status.positionY + yOffset, (int)status.otherCompetitor.x, (int)status.otherCompetitor.y + yOffset, m_lifeformTarget);
#endif
        }

        index++;
    });

    // Now draw the stats area
    RECT statsArea = *clientRect;
    statsArea.left = m_simulation->width();
    FillRect(m_canvasDC, &statsArea, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
    SetBkColor(m_canvasDC, DARK_GRAY);
    SelectObject(m_canvasDC, m_blackPen);

    index = 1;
    
    // Horizontal line to start with
    MoveToEx(m_canvasDC, leftEdge, yPosition, NULL);
    LineTo(m_canvasDC, leftEdge + leftEdge, yPosition);

    // Draw the lines between lifeforms and resources
    m_simulation->drawLifeforms([this, &index, &buffer, lifeformSize, leftEdge, &yPosition, rowHeight](const LifeformData& lifeform) {
        LifeformStatus status;
        lifeform.lifeForm->getDrawDetails(status);        

        if ((m_generation > 1) && (index <= NUM_ALPHAS)) {
            RECT row = { leftEdge , yPosition+1, leftEdge + 33 , yPosition + rowHeight + 1 };
            FillRect(m_canvasDC, &row, m_alphaRobotBrush);
            SetBkColor(m_canvasDC, ALPHA_COLOR);
        }
        else {
            SetBkColor(m_canvasDC, DARK_GRAY);
        }

        // Number
        if (status.alive) {
            if ((m_generation > 1) && (index <= NUM_ALPHAS)) {
                SetTextColor(m_canvasDC, RGB(0, 0, 0));
            }
            else {
                SetTextColor(m_canvasDC, RGB(255, 255, 255));
            }
        } 
        else {
            if ((m_generation > 1) && (index <= NUM_ALPHAS)) {
                SetTextColor(m_canvasDC, RGB(128, 128, 128));
            }
            else {
                SetTextColor(m_canvasDC, RGB(128, 128, 128));
            }
        }
        RECT row = { leftEdge + 3 , yPosition+1, leftEdge + 30 , yPosition + rowHeight };
        sprintf_s(buffer, "%i", index++);
        DrawTextA(m_canvasDC, buffer, -1, &row, DT_LEFT | DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // Horizontal line
        MoveToEx(m_canvasDC, leftEdge, yPosition + rowHeight + 1, NULL);
        LineTo(m_canvasDC, leftEdge + leftEdge, yPosition + rowHeight + 1);

        row.left = leftEdge + 34;

        if (!status.alive) {
            row.right = leftEdge + leftEdge;
            row.bottom++;
            FillRect(m_canvasDC, &row, m_deadBrush);
        }

#ifdef USE_SOLAR
        row.bottom = row.top + (rowHeight / 2);
        row.right = row.left + MulDiv(status.resources.sun, leftEdge - 34, MAX_SUN);
        FillRect(m_canvasDC, &row, m_sunBrush);
        row.top = row.bottom;
        row.bottom = row.top + (rowHeight / 2);
        row.right = row.left + MulDiv(status.resources.cell, leftEdge - 34, MAX_CELL);
        FillRect(m_canvasDC, &row, m_cellBrush);
#else
        row.right = row.left + MulDiv(status.resources.cell, leftEdge - 34, MAX_CELL);
        row.bottom++;
        FillRect(m_canvasDC, &row, m_cellBrush);
#endif

        yPosition += rowHeight + 1;
    });

   
    // Vertical line
    MoveToEx(m_canvasDC, leftEdge + 33, startY, NULL);
    LineTo(m_canvasDC, leftEdge + 33, yPosition);
    MoveToEx(m_canvasDC, leftEdge + leftEdge, startY, NULL);
    LineTo(m_canvasDC, leftEdge + leftEdge, yPosition+1);

    // Clear above and below
#ifdef FULLSCREEN
    RECT boxRect = { 0, 0, clientRect->right - clientRect->left, startY };
#else
    RECT boxRect = { 0, 0, clientRect->right - clientRect->left, yOffset };
#endif
    if (m_backgroundDC) BitBlt(m_canvasDC, boxRect.left, boxRect.top, boxRect.right, boxRect.bottom, m_backgroundDC, 0, 0, SRCCOPY);
        else FillRect(m_canvasDC, &boxRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
    boxRect = { 0, yOffset + m_simulation->height(), clientRect->right - clientRect->left, clientRect->bottom};
    if (m_backgroundDC) BitBlt(m_canvasDC, boxRect.left, boxRect.top, boxRect.right, boxRect.bottom-boxRect.top, m_backgroundDC, 0, boxRect.top, SRCCOPY);
    else FillRect(m_canvasDC, &boxRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

    // Stats
    SetBkMode(m_canvasDC, TRANSPARENT);
    sprintf_s(buffer, "Generation: %04i  Iteration: %04i (%03i%%)  %05i/sec", m_generation, m_simulation->currentAge(), MulDiv(m_simulation->currentAge(), 100, MAX_LIFESPAN), m_ticksPerSecond);
    SetTextColor(m_canvasDC, RGB(255, 255, 255));
    r = { leftEdge + 5, 1 + yOffset, leftEdge * 2, 20 + yOffset };
    DrawTextA(m_canvasDC, buffer, -1, &r, DT_CENTER | DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    sprintf_s(buffer, "Alive: %02i  Survived Previous Generation: %02i", numAlive, m_lastStatistics.numSurvivors);
    SetTextColor(m_canvasDC, RGB(255, 255, 255));
    r = { leftEdge + 5, 19 + yOffset, leftEdge * 2, 30 + yOffset };
    DrawTextA(m_canvasDC, buffer, -1, &r, DT_CENTER | DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SetBkMode(m_canvasDC, OPAQUE);


    // Controls
    r = { leftEdge + 20, yOffset + leftEdge - 35, leftEdge*2,yOffset + leftEdge-5};
    SetTextColor(m_canvasDC, RGB(255, 255, 255));
    if (m_speed == 0) strcpy_s(buffer, "Speed: slow"); else sprintf_s(buffer, "Speed: %i (iterations/frame)", m_speed);
    DrawTextA(m_canvasDC, buffer, -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
   
    m_speedMinus = { leftEdge + 240,yOffset + leftEdge - 35, leftEdge + 270,yOffset + leftEdge-5 };
    m_speedPlus = { leftEdge + 280,yOffset + leftEdge - 35, leftEdge + 310,yOffset + leftEdge-5 };
    FillRect(m_canvasDC, &m_speedMinus, m_buttonGray);
    FillRect(m_canvasDC, &m_speedPlus, m_buttonGray);
    

    r.left += 300;
    sprintf_s(buffer, "Turbo: %i (generations/frame)", m_iterationSkipSpeed);
    DrawTextA(m_canvasDC, buffer, -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    m_genSpeedMinus = { leftEdge + 520,yOffset + leftEdge - 35, leftEdge + 550,yOffset + leftEdge-5 };
    m_genSpeedPlus = { leftEdge + 560,yOffset + leftEdge - 35, leftEdge + 590,yOffset + leftEdge-5 };
    FillRect(m_canvasDC, &m_genSpeedMinus, m_buttonGray);
    FillRect(m_canvasDC, &m_genSpeedPlus, m_buttonGray);

    SetBkColor(m_canvasDC, RGB(128,128,128));
    DrawTextA(m_canvasDC, "-", -1, &m_speedMinus, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawTextA(m_canvasDC, "+", -1, &m_speedPlus, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawTextA(m_canvasDC, "-", -1, &m_genSpeedMinus, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawTextA(m_canvasDC, "+", -1, &m_genSpeedPlus, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

#ifdef FULLSCREEN
    // Create graph area
    SelectObject(m_canvasDC, m_blackPen);
    RECT graphArea = { leftEdge * 2 , startY, clientRect->right, yOffset + leftEdge };
    RECT graphOutput = graphArea; graphOutput.left += 2; graphOutput.bottom -= 50;
    FillRect(m_canvasDC, &graphOutput, m_darkerGray);
    MoveToEx(m_canvasDC, graphOutput.left-1, graphOutput.top, NULL);
    LineTo(m_canvasDC, graphOutput.left - 1, graphOutput.bottom);
    LineTo(m_canvasDC, graphOutput.right, graphOutput.bottom);
    // Horizontal line to start with
    const int graphWidth = graphOutput.right - graphOutput.left;
    
    graphOutput.bottom -= 2;

    const int survivorScale = (graphOutput.bottom - graphOutput.top) / POPULATION_SIZE;
    const float fitnessScale = (graphOutput.bottom - graphOutput.top) / (MAX_FITNESS * POPULATION_SIZE);

    int spacing = 4;
    // Shrink graph down as we get more data
    while ((spacing>1) && (m_statistics.size() * spacing > graphWidth)) spacing--;

    int offset = (int)m_statistics.size() - (graphWidth / spacing);
    // Clip or round up to the nearest 20
    if (offset < 0) offset = 0; else offset = ((offset + 19) / 20) * 20;
    int xSpacing = 20;

    for (int col = spacing; col < graphWidth; col+= spacing) {
        offset++;

        if (offset < m_statistics.size()) {
            const GenStatistics& p1 = m_statistics[offset-1];
            const GenStatistics& p2 = m_statistics[offset];

            // Lifeform line
            SelectObject(m_canvasDC, m_graphLifeform);
            MoveToEx(m_canvasDC, graphOutput.left + col - spacing, graphOutput.bottom - (p1.numSurvivors * survivorScale), NULL);
            LineTo(m_canvasDC, graphOutput.left + col, graphOutput.bottom - (p2.numSurvivors * survivorScale));

            // Iterations
            SelectObject(m_canvasDC, m_graphIterations);
            MoveToEx(m_canvasDC, graphOutput.left + col - spacing, graphOutput.bottom - (MulDiv(p1.numIterations - (INITIAL_STEPS/2), graphOutput.bottom - graphOutput.top, MAX_LIFESPAN - (INITIAL_STEPS/2))), NULL);
            LineTo(m_canvasDC, graphOutput.left + col, graphOutput.bottom - (MulDiv(p2.numIterations - (INITIAL_STEPS/2), graphOutput.bottom - graphOutput.top, MAX_LIFESPAN - (INITIAL_STEPS/2))));

            // Total fitness
            SelectObject(m_canvasDC, m_graphAvFitness);
            MoveToEx(m_canvasDC, graphOutput.left + col - spacing, graphOutput.bottom - (int)(fitnessScale * p1.totalFitness), NULL);
            LineTo(m_canvasDC, graphOutput.left + col, graphOutput.bottom - (int)(fitnessScale * p2.totalFitness));                      
        }
        
        //  Horizontal Scale
        if ((xSpacing >= 20) && ((offset-1) % 10) ==0){
            SelectObject(m_canvasDC, m_fontVert);
            sprintf_s(buffer, "%i", offset-1);
            SetBkColor(m_canvasDC, RGB(64, 64, 64));
            SetTextColor(m_canvasDC, RGB(255, 255, 255));
            RECT r = { graphOutput.left + col + 4, graphOutput.bottom+5, graphOutput.left + col, graphOutput.bottom+10 };
            DrawTextA(m_canvasDC, buffer, -1, &r, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOCLIP);
            SelectObject(m_canvasDC, m_font);
            xSpacing -= 20;
        }
        xSpacing += spacing;

    }
#endif
}

// Update and redraw
void CMainWindow::redrawSimulation(HWND hWnd) {
    HDC dc = GetDC(hWnd);
    RECT rec;
    GetClientRect(hWnd, &rec);

    // ensure we have a rendering surface
    if (checkCreateBitmap(dc, rec.right - rec.left, rec.bottom - rec.top)) {
        // internal draw of the simulation to the offscreen bitmap
        internalDraw(&rec);
        BitBlt(dc, 0, 0, m_lastBitmapSize.cx, m_lastBitmapSize.cy, m_canvasDC, 0, 0, SRCCOPY);
    }

    ReleaseDC(hWnd, dc);
}

// Free resources used by the offscreen bitmap
void CMainWindow::freeOffscreenBitmap() {
    // Free up memory
    if ((m_oldBitmap != 0) && (m_canvasDC!=0)) SelectObject(m_canvasDC, m_oldBitmap);
    if (m_canvas != 0) DeleteObject(m_canvas);
    if (m_canvasDC != 0) DeleteDC(m_canvasDC);

    m_lastBitmapSize = { 0,0 };
    m_canvasDC = 0;
    m_canvas = 0;
    m_oldBitmap = 0;
}

// Check and create an offscreen bitmap if required
bool CMainWindow::checkCreateBitmap(HDC dc, int width, int height) {
    if ((m_canvas == 0) || (width != m_lastBitmapSize.cx) || (height != m_lastBitmapSize.cy)) {
        freeOffscreenBitmap();

         // Create canvas
        m_canvasDC = CreateCompatibleDC(dc);
        if (!m_canvasDC) return false;

        void* bits;
        BITMAPINFO bi;
        memset(&bi, 0, sizeof(bi));
        
        bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
        bi.bmiHeader.biWidth = width;
        bi.bmiHeader.biHeight = height;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;
        
        // Create bitmap
        m_canvas = CreateDIBSection(m_canvasDC, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
        if (!m_canvas) return false;

        // Put it into play
        m_oldBitmap = SelectObject(m_canvasDC, m_canvas);

        // Store the new size
        m_lastBitmapSize = { width, height };
    }

    return true;
}

// Run the window
int CMainWindow::run() {
    // And register the class
    if (!registerWindowClass()) return 1;
#ifdef FULLSCREEN
    if (!createWindow(1920, 1080)) return 2;
#else
    if (!createWindow(m_simulation->width() * 2, m_simulation->height())) return 2;
#endif
        
    MSG msg;

    m_oldBrush = SelectObject(m_canvasDC, m_darkGray);
    m_oldPen = SelectObject(m_canvasDC, m_blackPen);
    m_oldFont = SelectObject(m_canvasDC, m_font);    

    m_simulationThread = new std::thread([this]() {
        while (!m_appTerminating) {
            runSimulation();
        }
    });

    // Main message loop:
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            msg.wParam = 3;
            break;
        }
        else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }    

    m_appTerminating = true;
    if (m_simulationThread->joinable()) m_simulationThread->join();
    delete m_simulationThread;

    SelectObject(m_canvasDC, m_oldBrush);
    SelectObject(m_canvasDC, m_oldPen);
    SelectObject(m_canvasDC, m_oldFont);

    // Free bitmap
    freeOffscreenBitmap();

    // Free up
    freeWindowAndClass();

    return (int)msg.wParam;
}