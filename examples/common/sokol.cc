/* sokol implementations need to live in it's own source file, because
on MacOS and iOS the implementation must be compiled as Objective-C, so there
must be a *.m file on MacOS/iOS, and *.c file everywhere else
*/
#include <time.h>
#if defined(__circle__)
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
#include <circle/usb/usbkeyboard.h>
#include <circle/input/mouse.h>
#include <circle/sched/scheduler.h>
#include <vc4/vchiq/vchiqdevice.h>
#include <circle/types.h>
#if !defined(NULL)
#define NULL 0
#endif

class CKernel
{
public:
	CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_USBHCI (&m_Interrupt, &m_Timer),
	m_VCHIQ (&m_Memory, &m_Interrupt)
    {
        m_ActLED.Blink (5);	// show we are alive
    }
	~CKernel (void)
    {
        
    }
    static CKernel *s_pThis;
	boolean Initialize (void)
    {
        boolean bOK = TRUE;
        if (bOK) bOK = m_Screen.Initialize ();
        if (bOK) bOK = m_Serial.Initialize (115200);
        if (bOK)
        {
            CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
            if (pTarget == 0)
            {
                pTarget = &m_Screen;
            }
            bOK = m_Logger.Initialize (pTarget);
        }
        if (bOK) bOK = m_Interrupt.Initialize ();
        if (bOK) bOK = m_Timer.Initialize ();
        if (bOK) bOK = m_USBHCI.Initialize ();
        if (bOK) bOK = m_VCHIQ.Initialize ();
        return bOK;
    }
    
    long GetClockTicks()
    {
        return m_Timer.GetClockTicks();
    }

	void Run (void)
    {
        CMouseDevice *pMouse = (CMouseDevice *) m_DeviceNameService.GetDevice ("mouse1", FALSE);
        CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *) m_DeviceNameService.GetDevice ("ukbd1", FALSE);    
        if (pKeyboard)
            pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw);
    }
    static void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
    {
        assert (s_pThis != 0);

        CString Message;
        Message.Format ("Key status (modifiers %02X)", (unsigned) ucModifiers);

        for (unsigned i = 0; i < 6; i++)
        {
            if (RawKeys[i] != 0)
            {
                CString KeyCode;
                KeyCode.Format (" %02X", (unsigned) RawKeys[i]);

                Message.Append (KeyCode);
            }
        }

        //s_pThis->m_Logger.Write (FromKernel, LogNotice, Message);
    }
private:
	// do not change this order
	CMemorySystem		m_Memory;
	CActLED			m_ActLED;
	CKernelOptions		m_Options;
	CDeviceNameService	m_DeviceNameService;
	CScreenDevice		m_Screen;
	CSerialDevice		m_Serial;
	CExceptionHandler	m_ExceptionHandler;
	CInterruptSystem	m_Interrupt;
	CTimer			m_Timer;
	CLogger			m_Logger;
	CUSBHCIDevice		m_USBHCI;
	CScheduler		m_Scheduler;
	CVCHIQDevice		m_VCHIQ;
}; 

CKernel m_Kernel;

extern "C" void circle_start_timer()
{
}

extern "C" long unsigned int circle_get_time()
{
    return m_Kernel.GetClockTicks();
}
extern "C" void circle_initialize()
{
    m_Kernel.Initialize();
}
extern "C" void circle_run()
{
    m_Kernel.Run();
}
#endif