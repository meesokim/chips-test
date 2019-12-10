//
// kernel.h
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
#ifndef _kernel_h
#define _kernel_h

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/input/mouse.h>
#include <circle/sched/scheduler.h>
#include <vc4/vchiq/vchiqdevice.h>
#include <circle/types.h>
#include <circle_stdlib_app.h>

#if 1
enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

class CKernel
{
public:
	CKernel (void);
	~CKernel (void);

	boolean Initialize (void);

	TShutdownMode Run (void);
	void msleep(int ms)
	{
		m_Scheduler.MsSleep(ms);
	}
	void usleep(int us)
	{
		m_Scheduler.usSleep(us);
	}
	unsigned GetTicks()
	{
		return m_Timer.GetClockTicks();
	}
	static CKernel *s_pThis;

private:
	// do not change this order
	CMemorySystem		m_Memory;
	CActLED			m_ActLED;
	CKernelOptions		m_Options;
	CDeviceNameService	m_DeviceNameService;
	CScreenDevice		m_Screen;
	CVCHIQDevice		m_VCHIQ;
	CConsole		m_Console;
	CSerialDevice		m_Serial;
	CExceptionHandler	m_ExceptionHandler;
	CInterruptSystem	m_Interrupt;
	CTimer			m_Timer;
	CLogger			m_Logger;
	CEMMCDevice     m_EMMC;
	CUSBHCIDevice	m_USBHCI;
	CFATFileSystem  m_FileSystem;	
	CScheduler		m_Scheduler;
	int w, h;
	unsigned m_nPosX;
	unsigned m_nPosY;
	void MouseEventHandler (TMouseEvent Event, unsigned nButtons, unsigned nPosX, unsigned nPosY);
	static void MouseEventStub (TMouseEvent Event, unsigned nButtons, unsigned nPosX, unsigned nPosY);	

};
#else
class CKernel : public CStdlibAppStdio
{
public:
	CKernel (void);
	TShutdownMode Run (void);
	bool Initialize() {
		m_VCHIQ.Initialize ();
		w = mScreen.GetWidth();
		h = mScreen.GetHeight();
		return CStdlibAppScreen::Initialize ();
	};

private:
	CMemorySystem		m_Memory;
	CScheduler		m_Scheduler;
	CVCHIQDevice		m_VCHIQ;
	int w, h;

};	
#endif
#endif

