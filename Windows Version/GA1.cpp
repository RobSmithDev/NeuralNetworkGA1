/*********************************************************************
 * Neural Network with Genetic Algorithms Demonstration              *
 * Copyright (C) 2002 RobSmithDev                                    *
 * https://robsmithdev.co.uk                                         *
 *                                                                   *
 * For more information about this project please see the video at:  *
 * https://www.youtube.com/watch?v=bq3FdlUeOTU                       *
 *********************************************************************/


#include "framework.h"
#include "GA1.h"
#include "window.h"

 int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CMainWindow* window = new CMainWindow(hInstance);
    int ret = window->run();
    delete window;

    return ret;
}

