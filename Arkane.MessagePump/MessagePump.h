// MessagePump.h

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS

#pragma once
#include <Windows.h>
#include <hash_map>
#include <vcclr.h>

using namespace System;
using namespace std;

namespace ArkaneSystems
{
	namespace Arkane
	{
		namespace MessagePump
		{
			// Callback wrapper
			public interface class IMessageCallback
			{
				// get called each time a new message arrives at the message-only window
				IntPtr OnMessageReceivedCallback(IntPtr hWnd, UINT message, WPARAM wParam, LPARAM lParam);
			};

			// Serves as a Windows message loop manager for the message-only window
			class NativeMessageOnlyWindow
			{
			private:
				HWND m_hWnd;
				bool m_bStop;

				// This map helps in correlating the hWnd we get in the WndProc to the
				// NativeMessageOnlyWindow instance of that window.
				static hash_map<HWND, NativeMessageOnlyWindow *> s_windowsMap;

				// Enables calling back to .NET interface method.
				gcroot<IMessageCallback ^> _messageCallback;

				// To stop the message loop, we need to call PostQuitMessage in the context of the
				// message loop thread. To do that we are sending this message.
				static const unsigned int WM_APPSTOP = WM_APP + 242;

				// Create the message-only window.
				void Create();

				// The WndProc for the message-only window.
				static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

			public:
				NativeMessageOnlyWindow(const gcroot<IMessageCallback ^> &&callback) : m_hWnd(0), m_bStop(false), _messageCallback(callback) {}

				// Stop the message loop; the actual stop happens asynchronously, since we use PostQuitMessage.
				void StopMessageLoop();

				// A blocking call; the message loop is in the context of the calling thread.
				void StartMessageLoop();

				// Return the message-only window handle.
				HWND GetWindowHandle() const { return m_hWnd; }

				// Stop the message loop and destroy the instance.
				~NativeMessageOnlyWindow();
			};

			/// <summary>
			///     Represents a Windows window message encapsulated in a .NET class.
			/// </summary>
			public value class WindowMessage
			{
			private:
				IntPtr m_hwnd;
				unsigned int m_uMessage;
				UIntPtr m_wParam;
				IntPtr m_lParam;

			public:
				WindowMessage(IntPtr hWnd, UINT message, WPARAM wParam, LPARAM lParam) :
					m_hwnd(hWnd), m_uMessage(message), m_wParam(UIntPtr(wParam)), m_lParam(IntPtr(lParam)) {}

				property IntPtr HWnd { IntPtr get() { return m_hwnd; }}
				property unsigned int Message { unsigned int get() { return m_uMessage; }}
				property UIntPtr WParam { UIntPtr get() { return m_wParam; }}
				property IntPtr LParam { IntPtr get() { return m_lParam; }}
			};

			/// <summary>
			///     Use this class to create a message-only window.
			///</summary>
			///<remarks>
			///     With this class you can receive window messages without the need to use a System.Windows.Forms.Form instance.
			///     Message-only windows are lightweight mechanisms that serve only for receiving window messages.
			///</remarks>
			public ref class MessageOnlyWindow : public IMessageCallback
			{
			private:
				NativeMessageOnlyWindow * m_pNativeMessageOnlyWindow;

			protected:
				/// <summary>
				///     This callback function receives the raw WndProc arguments for any message that has been 
				///     sent to the message-only window.
				/// </summary>
				/// <param name=”hWnd”>The target window handle</param>
				/// <param name=”message”>The message number</param>
				/// <param name=”wParam”>the wParam</param>
				/// <param name=”lParam”>the lParam</param>
				/// <returns>The method should return a value for the DefWndProc; usually <see cref=”IntPtr::Zero”/> is suitable.</returns>
				/// <remarks>
				///     When overriding this method, call the base class implementation to fire the 
				///     <see cref=”MessageReceived”/> event.
				/// </remarks>
				virtual IntPtr OnMessageReceivedCallback(IntPtr hWnd, UINT message, WPARAM wParam, LPARAM lParam)
					= IMessageCallback::OnMessageReceivedCallback;

			public:
				/// <summary>
				///     Constructor for the message-only window instance.
				/// </summary>
				/// <remarks>
				///     With this instance you can receive window messages without the need to use a System.Windows.Forms.Form instance.
				///     Register a callback on the <see cref=”MessageReceived”/> event and call the 
				///     <see cref=”StartMessageLoop”/> function to start receiving window messages.
				///     Call the <see cref=”StopMessageLoop”/> function to stop receiving windows messages.
				///     Dispose of the instance whenever you do not need it anymore
				///</remarks>
				MessageOnlyWindow();

				/// <summary>
				///     Fires an event that represents a single window message that has been sent to 
				///     the message-only window.
				/// </summary>
				event Func<WindowMessage, IntPtr> ^MessageReceived;

				/// <summary>
				///     Start the message loop in the context of the calling thread.
				/// </summary>
				/// <remarks>
				///     Call this method in the context of the thread that triggered the window messages source.
				///     This is a blocking call; the message loop is in the context of the calling thread.
				///     This method will return only after a call to the <see cref=”StopMessageLoop”/> function.
				/// </remarks>
				void StartMessageLoop()
				{
					m_pNativeMessageOnlyWindow->StartMessageLoop();
				}

		        /// <summary>
				///     Ask the message pump thread to stop. 
				/// </summary>
				/// <remarks>
				///     The message pump thread is the thread that called the <see cref=”StartMessageLoop”/>.
				///     The stop operations is asynchronous. This call triggers a WM_QUIT message that will 
				///     stop the loop when processed.
				/// </remarks>
				void StopMessageLoop()
				{ 
					m_pNativeMessageOnlyWindow->StopMessageLoop();
				}

				/// <summary>
				///     The message-only window window handle.
				/// </summary>
				property IntPtr WindowHandle 
				{ IntPtr get() { return IntPtr(m_pNativeMessageOnlyWindow->GetWindowHandle()); }}
				
				/// <summary>
				///     Dispose of native resources.
				/// </summary>
				~MessageOnlyWindow() { delete m_pNativeMessageOnlyWindow; }
			};
		}
	}
}