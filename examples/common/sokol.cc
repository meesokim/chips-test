#include <time.h>
#include "kernel.h"
#include <stdio.h>
#include <circle/string.h>
#include <circle/debug.h>
#include <SDCard/emmc.h>
#include <assert.h>
static const char FromKernel[] = "kernel";
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
	printf("Clear!!\n");
	return bOK;
}

typedef enum sapp_event_type {
    SAPP_EVENTTYPE_INVALID,
    SAPP_EVENTTYPE_KEY_DOWN,
    SAPP_EVENTTYPE_KEY_UP,
    SAPP_EVENTTYPE_CHAR,
    SAPP_EVENTTYPE_MOUSE_DOWN,
    SAPP_EVENTTYPE_MOUSE_UP,
    SAPP_EVENTTYPE_MOUSE_SCROLL,
    SAPP_EVENTTYPE_MOUSE_MOVE,
    SAPP_EVENTTYPE_MOUSE_ENTER,
    SAPP_EVENTTYPE_MOUSE_LEAVE,
    SAPP_EVENTTYPE_TOUCHES_BEGAN,
    SAPP_EVENTTYPE_TOUCHES_MOVED,
    SAPP_EVENTTYPE_TOUCHES_ENDED,
    SAPP_EVENTTYPE_TOUCHES_CANCELLED,
    SAPP_EVENTTYPE_RESIZED,
    SAPP_EVENTTYPE_ICONIFIED,
    SAPP_EVENTTYPE_RESTORED,
    SAPP_EVENTTYPE_SUSPENDED,
    SAPP_EVENTTYPE_RESUMED,
    SAPP_EVENTTYPE_UPDATE_CURSOR,
    SAPP_EVENTTYPE_QUIT_REQUESTED,
    _SAPP_EVENTTYPE_NUM,
    _SAPP_EVENTTYPE_FORCE_U32 = 0x7FFFFFFF
} sapp_event_type;

typedef enum sapp_mousebutton {
    SAPP_MOUSEBUTTON_INVALID = -1,
    SAPP_MOUSEBUTTON_LEFT = 0,
    SAPP_MOUSEBUTTON_RIGHT = 1,
    SAPP_MOUSEBUTTON_MIDDLE = 2,
} sapp_mousebutton;

CKernel m_Kernel;
void ImGui_ImplCircle_ProcessEvent(TMouseEvent Event, unsigned nButtons, unsigned nPosX, unsigned nPosY);
TShutdownMode CKernel::Run (void)
{
	CMouseDevice *pMouse = (CMouseDevice *) m_DeviceNameService.GetDevice ("mouse1", FALSE);
	if (!pMouse)
	{
		pMouse->Setup (m_Screen.GetWidth (), m_Screen.GetHeight ());
	}
	m_nPosX = m_Screen.GetWidth () / 2;
	m_nPosY = m_Screen.GetHeight () / 2;
	pMouse->SetCursor (m_nPosX, m_nPosY);
	pMouse->RegisterEventHandler (ImGui_ImplCircle_ProcessEvent);	
	return ShutdownHalt;
}

//static DEFINE_SPINLOCK (mouse_lock);
extern "C" void _sapp_raspi_mouse_event(sapp_event_type type, sapp_mousebutton btn, int x, int y);
void ImGui_ImplCircle_ProcessEvent(TMouseEvent Event, unsigned nButtons, unsigned nPosX, unsigned nPosY)
{
	//spin_lock (&mouse_lock);
	static int mx, my;
	static bool g_MouseButton[3];
	sapp_mousebutton button;
    switch (Event)
    {
	case MouseEventMouseMove:
	{
		mx = nPosX;
		my = nPosY;
		button = g_MouseButton[SAPP_MOUSEBUTTON_LEFT] ? SAPP_MOUSEBUTTON_LEFT : g_MouseButton[SAPP_MOUSEBUTTON_RIGHT] ? SAPP_MOUSEBUTTON_RIGHT : g_MouseButton[2] ? SAPP_MOUSEBUTTON_MIDDLE : SAPP_MOUSEBUTTON_INVALID; 
		_sapp_raspi_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, button, mx, my);
		break;
	}
	case MouseEventMouseDown:
	{
		g_MouseButton[0] = nButtons & MOUSE_BUTTON_LEFT ? true : false;
		g_MouseButton[1] = nButtons & MOUSE_BUTTON_RIGHT ? true : false;
		g_MouseButton[2] = nButtons & MOUSE_BUTTON_MIDDLE ? true : false;
		button = g_MouseButton[SAPP_MOUSEBUTTON_LEFT] ? SAPP_MOUSEBUTTON_LEFT : g_MouseButton[SAPP_MOUSEBUTTON_RIGHT] ? SAPP_MOUSEBUTTON_RIGHT : g_MouseButton[2] ? SAPP_MOUSEBUTTON_MIDDLE : SAPP_MOUSEBUTTON_INVALID; 
		_sapp_raspi_mouse_event(SAPP_EVENTTYPE_MOUSE_DOWN, button, mx, my);
		break;
	}
	case MouseEventMouseUp:
	{
		g_MouseButton[0] = nButtons & MOUSE_BUTTON_LEFT ? false : true;
		g_MouseButton[1] = nButtons & MOUSE_BUTTON_RIGHT ? false : true;
		g_MouseButton[2] = nButtons & MOUSE_BUTTON_MIDDLE ? false : true;
		button = g_MouseButton[SAPP_MOUSEBUTTON_LEFT] ? SAPP_MOUSEBUTTON_LEFT : g_MouseButton[SAPP_MOUSEBUTTON_RIGHT] ? SAPP_MOUSEBUTTON_RIGHT : g_MouseButton[2] ? SAPP_MOUSEBUTTON_MIDDLE : SAPP_MOUSEBUTTON_INVALID; 
		_sapp_raspi_mouse_event(SAPP_EVENTTYPE_MOUSE_UP, button, mx, my);
		break;
	}
	default:
	{
	}
    }
	//spin_unlock (&mouse_lock);
    return;
}

extern "C" void circle_start_timer()
{
}

extern "C" long unsigned int circle_get_time()
{
    if (CKernel::s_pThis)
		return CKernel::s_pThis->GetTicks();
	return 0;
}
extern "C" void circle_initialize()
{
    m_Kernel.Initialize();
}
extern "C" void circle_run()
{
    m_Kernel.Run();
}
