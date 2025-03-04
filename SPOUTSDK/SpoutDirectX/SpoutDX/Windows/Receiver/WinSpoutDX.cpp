/*
=========================================================================

                        WinSpoutDX.cpp

   A Windows Desktop Application project generated by Visual Studio
   and adapted for Spout input (http://spout.zeal.co/).

   This is an example using the SpoutDX support class and ReceiveImage

   bool ReceiveImage(ID3D11Device* pd3dDevice, unsigned char * pixels, bool bRGB = false, bool bInvert = false);

   Receives to a pixel buffer and displays the output using Windows bitmap functions.

   Search on "SPOUT" for additions.
   Compare with the DirectX 11 Tutorial07 example using ReceveTexture.

=========================================================================

                 Copyright(C) 2021 Lynn Jarvis.

This program is free software : you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.If not, see < http://www.gnu.org/licenses/>.
========================================================================

*/


#include "framework.h"
#include "WinSpoutDX.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                      // current instance
WCHAR szTitle[MAX_LOADSTRING];        // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];  // the main window class name

// SPOUT
spoutDX receiver;                      // Receiver object
HWND g_hWnd = NULL;                    // Window handle
unsigned char *pixelBuffer = nullptr;  // Receiving pixel buffer
unsigned char *bgraBuffer = nullptr;   // Conversion buffer if required
unsigned char g_SenderName[256];       // Received sender name
unsigned int g_SenderWidth = 0;        // Received sender width
unsigned int g_SenderHeight = 0;       // Received sender height
DWORD g_SenderFormat = 0;              // Received sender format

// Static for sender selection dialog box
static char sendername[256];

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SenderProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// SPOUT
void Render();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

 	// SPOUT
	// Optionally enable Spout logging
	// OpenSpoutConsole(); // Console only for debugging
	// EnableSpoutLog(); // Log to console
	// EnableSpoutLogFile("Windows receiver.log"); // Log to file
	// SetSpoutLogLevel(SPOUT_LOG_WARNING); // show only warnings and errors

	// Optionally set the name of the sender to receive from
	// The receiver will only connect to that sender.
	// The user can over-ride this by selecting another.
	// receiver.SetReceiverName("Spout DX11 Sender");

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINSPOUTDX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	// SPOUT
	// Initialize DirectX
	// A device is created in the SpoutDX class.
	if (!receiver.OpenDirectX11())
		return FALSE;

    // Main message loop:
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// else
		{
			Render();
		}
	}

	// SPOUT
	if (pixelBuffer) delete pixelBuffer;
	if(bgraBuffer) delete bgraBuffer;

	// Release the receiver
	receiver.ReleaseReceiver();

	// Release DirectX 11 resources
	receiver.CloseDirectX11();

    return (int) msg.wParam;
}

// SPOUT
void Render()
{
	// Get pixels from the sender shared texture.
	// ReceiveImage handles sender detection, creation and update.
	// Because Windows bitmaps are bottom-up, the rgba pixel buffer is flipped by the 
	// ReceiveImage function ready for WM_PAINT but it could also be drawn upside down
	if (receiver.ReceiveImage(pixelBuffer, g_SenderWidth, g_SenderHeight, false, true)) { // RGB = false, invert = true
		
		// IsUpdated() returns true if the sender has changed
		if (receiver.IsUpdated()) {

			// Update the sender name - it could be different
			strcpy_s((char *)g_SenderName, 256, receiver.GetSenderName());

			// Update globals
			g_SenderWidth = receiver.GetSenderWidth();
			g_SenderHeight = receiver.GetSenderHeight();
			g_SenderFormat = receiver.GetSenderFormat();

			// Update the receiving buffer
			if(pixelBuffer)	delete pixelBuffer;
			pixelBuffer = new unsigned char[g_SenderWidth * g_SenderHeight * 4];

			// Update the rgba > bgra conversion buffer
			if (bgraBuffer) delete bgraBuffer;
			bgraBuffer = new unsigned char[g_SenderWidth * g_SenderHeight * 4];

			// Do anything else necessary for the application here

		}
	}

	// Trigger a re-paint to draw the pixel buffer - see WM_PAINT
	InvalidateRect(g_hWnd, NULL, FALSE);
	UpdateWindow(g_hWnd); // Update immediately

	// Optionally hold a target frame rate - e.g. 60 or 30fps.
	// This is not necessary if the application already has
	// fps control. But in this example rendering is done
	// during idle time and render rate can be extremely high.
	receiver.HoldFps(60);

}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINSPOUTDX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = CreateHatchBrush(HS_DIAGCROSS, RGB(192, 192, 192));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINSPOUTDX);
    wcex.lpszClassName  = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WINSPOUTDX));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // Create window
   RECT rc = { 0, 0, 640, 360 }; // Desired client size
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE); // Allow for menu
   HWND hWnd = CreateWindowW(szWindowClass,
	   szTitle,
	   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME,
	   CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
	   nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // SPOUT
   // Centre the window on the desktop work area
   GetWindowRect(hWnd, &rc);
   RECT WorkArea;
   int WindowPosLeft = 0;
   int WindowPosTop = 0;
   SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&WorkArea, 0);
   WindowPosLeft += ((WorkArea.right  - WorkArea.left) - (rc.right - rc.left)) / 2;
   WindowPosTop  += ((WorkArea.bottom - WorkArea.top)  - (rc.bottom - rc.top)) / 2;
   MoveWindow(hWnd, WindowPosLeft, WindowPosTop, (rc.right - rc.left), (rc.bottom - rc.top), false);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hWnd = hWnd;
   
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
			case IDM_OPEN:
				// Sender selection dialog box 
				sendername[0] = 0; // Clear static name for dialog
				DialogBox(hInst, MAKEINTRESOURCE(IDD_SENDERBOX), hWnd, (DLGPROC)SenderProc);
				// Open select a sender
				// receiver.SelectSender();
				break;

			case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_PAINT:
        {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			// SPOUT
			if (pixelBuffer) {

				//
				// Draw the received image
				//

				RECT dr = { 0 };
				GetClientRect(hWnd, &dr);

				// No sender - draw default background
				if (!receiver.IsConnected()) {
					HBRUSH backbrush = CreateHatchBrush(HS_DIAGCROSS, RGB(192, 192, 192));
					FillRect(hdc, &dr, backbrush);
					DeleteObject(backbrush);
				}
				else {
					BITMAPINFO bmi;
					ZeroMemory(&bmi, sizeof(BITMAPINFO));
					bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
					bmi.bmiHeader.biSizeImage = (LONG)(g_SenderWidth * g_SenderHeight * 4); // Pixel buffer size
					bmi.bmiHeader.biWidth = (LONG)g_SenderWidth;   // Width of buffer
					bmi.bmiHeader.biHeight = (LONG)g_SenderHeight;  // Height of buffer
					bmi.bmiHeader.biPlanes = 1;
					bmi.bmiHeader.biBitCount = 32;
					bmi.bmiHeader.biCompression = BI_RGB;
					// If the received sender format is BGRA or BGRX it's a natural match
					if (g_SenderFormat == 87 || g_SenderFormat == 88) {
						// Very fast (< 1msec at 1280x720)
						// StretchDIBits adapts the pixel buffer received from the sender
						// to the window size. The sender can be resized or changed.
						SetStretchBltMode(hdc, COLORONCOLOR); // Fastest method
						StretchDIBits(hdc,
							0, 0, (dr.right - dr.left), (dr.bottom - dr.top), // destination rectangle 
							0, 0, g_SenderWidth, g_SenderHeight, // source rectangle 
							pixelBuffer,
							&bmi, DIB_RGB_COLORS, SRCCOPY);
					}
					else {
						//
						// Received data is RGBA but windows draw is BGRA and conversion is required
						//
						// For widths divisible by 16, a high speed function is
						// available to convert from rgba to bgra.
						// Timing has shown that this is much faster (1-2 msec)
						// than using BITMAPV4HEADER (18-24 msec) at 1280x720.
						//		
						if ((g_SenderWidth % 16) == 0) {
							// SSE conversion from rgba to bgra
							receiver.spoutcopy.rgba2bgra(pixelBuffer, bgraBuffer, g_SenderWidth, g_SenderHeight);
							SetStretchBltMode(hdc, COLORONCOLOR);
							StretchDIBits(hdc,
								0, 0, (dr.right - dr.left), (dr.bottom - dr.top),
								0, 0, g_SenderWidth, g_SenderHeight, bgraBuffer,
								&bmi, DIB_RGB_COLORS, SRCCOPY);
						}
						else {
							// The extended BITMAPV4HEADER bitmap info header
							// is required instead of BITMAPINFO but can be slow.
							BITMAPV4HEADER info = { sizeof(BITMAPV4HEADER) };
							info.bV4Width = (LONG)g_SenderWidth;
							info.bV4Height = -(LONG)g_SenderHeight;
							info.bV4Planes = 1;
							info.bV4BitCount = 32;
							info.bV4V4Compression = BI_BITFIELDS;
							info.bV4RedMask = 0x000000FF;
							info.bV4GreenMask = 0x0000FF00;
							info.bV4BlueMask = 0x00FF0000;
							info.bV4AlphaMask = 0xFF000000;
							SetStretchBltMode(hdc, COLORONCOLOR);
							StretchDIBits(hdc,
								0, 0, (dr.right - dr.left),	(dr.bottom - dr.top),
								0, 0, g_SenderWidth, g_SenderHeight, pixelBuffer,
								reinterpret_cast<BITMAPINFO*>(&info),
								DIB_RGB_COLORS,	SRCCOPY);
						}
					}
				}
			}
			EndPaint(hWnd, &ps);
        }
        break;

	// SPOUT
	case WM_RBUTTONDOWN:
		// RH click to select a sender
		receiver.SelectSender();
		break;
    
	case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
// SPOUT : adapted for this example.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
 	UNREFERENCED_PARAMETER(lParam);
	char tmp[MAX_PATH];
	char about[1024];
	LPDRAWITEMSTRUCT lpdis;
	HWND hwnd = NULL;
	HCURSOR cursorHand = NULL;

    switch (message)
    {
    case WM_INITDIALOG:

		sprintf_s(about, 256, "                WinSpoutDX");
		strcat_s(about, 1024, "\n\n\n");
		strcat_s(about, 1024, "Windows Spout receiver example.\n");
		strcat_s(about, 1024, "Using the SpoutDX class, receive\n");
		strcat_s(about, 1024, "a buffer and display it in WM_PAINT.");
		SetDlgItemTextA(hDlg, IDC_ABOUT_TEXT, (LPCSTR)about);

		// Spout website hyperlink hand cursor
		cursorHand = LoadCursor(NULL, IDC_HAND);
		hwnd = GetDlgItem(hDlg, IDC_SPOUT_URL);
		SetClassLongPtrA(hwnd, GCLP_HCURSOR, (LONG_PTR)cursorHand);
        return (INT_PTR)TRUE;

	case WM_DRAWITEM:
		// The blue hyperlink
		lpdis = (LPDRAWITEMSTRUCT)lParam;
		if (lpdis->itemID == -1) break;
		SetTextColor(lpdis->hDC, RGB(6, 69, 173));
		switch (lpdis->CtlID) {
		case IDC_SPOUT_URL:
			DrawTextA(lpdis->hDC, "http://spout.zeal.co", -1, &lpdis->rcItem, DT_LEFT);
			break;
		default:
			break;
		}
		break;

    case WM_COMMAND:

		if (LOWORD(wParam) == IDC_SPOUT_URL) {
			// Open the website url
			sprintf_s(tmp, MAX_PATH, "http://spout.zeal.co");
			ShellExecuteA(hDlg, "open", tmp, NULL, NULL, SW_SHOWNORMAL);
			EndDialog(hDlg, 0);
			return (INT_PTR)TRUE;
		}

        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// SPOUT : added for this example
// Message handler for selecting sender
INT_PTR  CALLBACK SenderProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam); // suppress warning

	switch (message) {

	case WM_INITDIALOG:
		// Sender combo selection
	{
		// Create an sender name list for the combo box
		HWND hwndList = GetDlgItem(hDlg, IDC_SENDERS);

		// Active sender name for initial item
		char activename[256];
		receiver.GetActiveSender(activename);
		int activeindex = 0;

		// Sender count
		int count = receiver.GetSenderCount();

		// Populate the combo box
		char name[128];
		for (int i = 0; i < count; i++) {
			receiver.GetSender(i, name, 128);
			// Active sender index for the initial combo box item
			if (strcmp(name, activename) == 0)
				activeindex = i;
			SendMessageA(hwndList, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)name);
		}

		// Show the active sender as the initial item
		SendMessageA(hwndList, CB_SETCURSEL, (WPARAM)activeindex, (LPARAM)0);
	}
	return TRUE;

	case WM_COMMAND:

		// Combo box selection
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			// Get the selected sender name
			int index = (int)SendMessageA((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			SendMessageA((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)index, (LPARAM)sendername);
		}
		// Drop through

		switch (LOWORD(wParam)) {

		case IDOK:
			// Selected sender
			if (sendername[0]) {
				// Make the sender active
				receiver.SetActiveSender(sendername);
				// Reset the receiving name
				// A new sender is detected on the first ReceiveTexture call
				receiver.SetReceiverName();
			}
			EndDialog(hDlg, 1);
			break;

		case IDCANCEL:
			// User pressed cancel.
			EndDialog(hDlg, 0);
			return (INT_PTR)TRUE;

		default:
			return (INT_PTR)FALSE;
		}
	}

	return (INT_PTR)FALSE;
}


// That's all..