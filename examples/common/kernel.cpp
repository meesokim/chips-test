//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "kernel.h"
#include <stdio.h>
#include <circle/string.h>
#include <circle/debug.h>
#include <SDCard/emmc.h>
#include <assert.h>
#include <SDL.h>
#define NDEBUG
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_circle.h"
static const char FromKernel[] = "kernel";
	
extern "C" int _main(void);
extern "C" int __main(void);
extern "C" {
#include "GLES2/gl2.h"
}
#define CSTDLIBAPP_DEFAULT_PARTITION "emmc1-1"
#if 1	
CKernel *CKernel::s_pThis = 0;
CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED), 
	m_USBHCI (&m_Interrupt, &m_Timer),
	m_VCHIQ (&m_Memory, &m_Interrupt),
	m_Console (&m_Screen)
{
//	m_ActLED.Blink (5);	// show we are alive
	s_pThis = this;  
}


CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}
	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		}

		bOK = m_Logger.Initialize (pTarget);
	}
	m_EMMC.Initialize ();
	CDevice * const pPartition =
			m_DeviceNameService.GetDevice (CSTDLIBAPP_DEFAULT_PARTITION, true);
	if (pPartition == 0)
	{
			m_Logger.Write (FromKernel, LogError,
						   "Partition not found: %s", CSTDLIBAPP_DEFAULT_PARTITION);
			return false;
	}

	if (!m_FileSystem.Mount (pPartition))
	{
			m_Logger.Write (FromKernel, LogError,
							 "Cannot mount partition: %s", CSTDLIBAPP_DEFAULT_PARTITION);

			return false;
	}

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}

	if (bOK)
	{
		bOK = m_USBHCI.Initialize ();
	}

	if (bOK)
	{
		bOK = m_VCHIQ.Initialize ();
	}
	m_Console.Initialize ();
	CGlueStdioInit (m_FileSystem, m_Console);
	w = m_Screen.GetWidth();
	h = m_Screen.GetHeight();
	return bOK;
}
#else
CKernel::CKernel (void)
:	CStdlibAppStdio ("03-stdio-fatfs"),
	m_VCHIQ (&m_Memory, &mInterrupt)
{
//	mActLED.Blink (5);	// show we are alive
}	

#endif
extern "C" int __main(void);
extern "C" int _init_gl();
extern "C" void _glSwapWindow();
TShutdownMode CKernel::Run (void)
{
	CMouseDevice *pMouse = (CMouseDevice *) m_DeviceNameService.GetDevice ("mouse1", FALSE);
	if (pMouse == 0)
	{
		m_Logger.Write (FromKernel, LogPanic, "Mouse not found");
	}
	if (!pMouse->Setup (m_Screen.GetWidth (), m_Screen.GetHeight ()))
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot setup mouse");
	}
	m_nPosX = m_Screen.GetWidth () / 2;
	m_nPosY = m_Screen.GetHeight () / 2;

	pMouse->SetCursor (m_nPosX, m_nPosY);
	_init_gl();
	ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.MouseDrawCursor = true;
	ImGui::StyleColorsDark();
	//const char* glsl_version = "#version 120";
	ImGui_ImplCircle_InitForGLES();
	ImGui_ImplOpenGL3_Init(0);
	pMouse->RegisterEventHandler (ImGui_ImplCircle_ProcessEvent);	
    // Our state
    bool show_demo_window = true;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
   // Main loop
    bool done = false;
	int counter = 0;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // SDL_Event event;
        // while (SDL_PollEvent(&event))
        // {
            // ImGui_ImplSDL2_ProcessEvent(&event);
            // if (event.type == SDL_QUIT)
                // done = true;
            // if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                // done = true;
        // }
		printf("\rTest:%d", counter++);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplCircle_NewFrame(w, h);
		ImGui::NewFrame();
        // Start the Dear ImGui frame

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
	
        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
			static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
        //Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		//glFlush();
        _glSwapWindow();		
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplCircle_Shutdown();
    ImGui::DestroyContext();	
	m_Screen.Write ("Done\n", 5);
//	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
#ifndef NDEBUG
	// some debugging features
	m_Logger.Write (FromKernel, LogDebug, "Dumping the start of the ATAGS");
	debug_hexdump ((void *) 0x100, 128, FromKernel);

	m_Logger.Write (FromKernel, LogNotice, "The following assertion will fail");
	assert (1 == 2);
#endif

	return ShutdownHalt;
}

void CKernel::MouseEventHandler (TMouseEvent Event, unsigned nButtons, unsigned nPosX, unsigned nPosY)
{
	switch (Event)
	{
	case MouseEventMouseMove:
		if (nButtons & (MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT))
		{
			
		}
		
		m_nPosX = nPosX;
		m_nPosY = nPosY;
		break;

	case MouseEventMouseDown:
		if (nButtons & MOUSE_BUTTON_MIDDLE)
		{
			// m_Screen.Write (ClearScreen, sizeof ClearScreen-1);
		}
		break;

	default:
		break;
	}
} 
