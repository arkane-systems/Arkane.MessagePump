// This is the main DLL file.

#include "stdafx.h"

#include "MessagePump.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace ArkaneSystems
{
	namespace Arkane
	{
		namespace MessagePump
		{
			MessageOnlyWindow::MessageOnlyWindow()
			{
				m_pNativeMessageOnlyWindow = new NativeMessageOnlyWindow(gcroot<IMessageCallback ^>(this));
			}

			IntPtr MessageOnlyWindow::OnMessageReceivedCallback(IntPtr hWnd, UINT message, WPARAM wParam, LPARAM lParam)
			{
				return MessageReceived(WindowMessage(hWnd, message, wParam, lParam));
			}

			unordered_map<HWND, NativeMessageOnlyWindow *> NativeMessageOnlyWindow::s_windowsMap;

			void NativeMessageOnlyWindow::Create()
			{
				// Create a window that can only receive messages.
				m_hWnd = CreateWindow(L"MessageOnly", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, GetModuleHandle(0), nullptr);

				if (m_hWnd == nullptr)
					Marshal::ThrowExceptionForHR(GetLastError());

				// Set the window's WndProc to our window procedure.
				auto result = SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
				if (result == 0)
					Marshal::ThrowExceptionForHR(GetLastError());

				// Keep the correlation between the hWnd and the MessageOnlyWindow instance.
				s_windowsMap[m_hWnd] = this;
			}

			void NativeMessageOnlyWindow::StartMessageLoop()
			{
				m_bStop = false;

				// Create the window.
				Create();

				MSG msg;

				// The traditional message loop.
				while (GetMessage(&msg, NULL, 0, 0))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

			void NativeMessageOnlyWindow::StopMessageLoop()
			{
				m_bStop = true;
				SendMessage(m_hWnd, WM_APPSTOP, 0, 0);
			}

			NativeMessageOnlyWindow::~NativeMessageOnlyWindow()
			{
				if (m_hWnd == nullptr)
					return;

				StopMessageLoop();
			}

			LRESULT CALLBACK NativeMessageOnlyWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
			{
				// Find the correlated NativeMessageOnlyWindow instance.
				auto iter = s_windowsMap.find(hWnd);
				if (iter == s_windowsMap.end())
				{
					// instance not found
					OutputDebugString(L"NativeMessageOnlyWindow: a message cannot be dispatched; hWnd not found.");
					return DefWindowProc(hWnd, message, wParam, lParam);
				}

				auto This = iter->second;

				if (message == WM_APPSTOP && This->m_bStop)
				{
					// Is stop order.
					// Send WM_QUIT to stop the message loop.
					PostQuitMessage(0);

					// Remove the NativeMessageOnlyWindow instance from the hash map.
					s_windowsMap.erase(iter);

					This->m_hWnd = nullptr;

					return DefWindowProc(hWnd, message, wParam, lParam);
				}

				// Trigger the callback call to the managed wrapper instance.
				return reinterpret_cast<LRESULT>(This->_messageCallback->OnMessageReceivedCallback(IntPtr(hWnd), message, wParam, lParam).ToPointer());
			}
		}
	}
}