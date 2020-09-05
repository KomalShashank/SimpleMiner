#define WIN_32_LEAN_AND_MEAN
#include <Windows.h>
#include <time.h>
#include <cassert>
#include <crtdbg.h>
#include "Game/GameCommons.hpp"
#include "Game/TheGame.hpp"

#define UNUSED(x) (void)(x);

HWND g_hWnd = nullptr;
HDC g_DisplayDeviceContext = nullptr;
HGLRC g_OpenGLRenderingContext = nullptr;
const char* APP_NAME = "Mining Adventure";



LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	unsigned char asKey = (unsigned char)wParam;
	switch (wmMessageCode)
	{
	case WM_CLOSE:
	case WM_DESTROY:
	case WM_QUIT:
		g_IsQuitting = true;

		return 0;

	case WM_KEYDOWN:
		InputSystem::SingletonInstance()->SetKeyDownState(asKey, true);
		break;

	case WM_KEYUP:
		InputSystem::SingletonInstance()->SetKeyDownState(asKey, false);
		break;

	case WM_LBUTTONDOWN:
		InputSystem::SingletonInstance()->SetMouseClickedState(LEFT_MOUSE_BUTTON, true);
		break;

	case WM_LBUTTONUP:
		InputSystem::SingletonInstance()->SetMouseClickedState(LEFT_MOUSE_BUTTON, false);
		break;

	case WM_MBUTTONDOWN:
		InputSystem::SingletonInstance()->SetMouseClickedState(MIDDLE_MOUSE_BUTTON, true);
		break;

	case WM_MBUTTONUP:
		InputSystem::SingletonInstance()->SetMouseClickedState(MIDDLE_MOUSE_BUTTON, false);
		break;

	case WM_RBUTTONDOWN:
		InputSystem::SingletonInstance()->SetMouseClickedState(RIGHT_MOUSE_BUTTON, true);
		break;

	case WM_RBUTTONUP:
		InputSystem::SingletonInstance()->SetMouseClickedState(RIGHT_MOUSE_BUTTON, false);
		break;

	case WM_MOUSEWHEEL:
	{
		short mouseWheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (mouseWheelDelta > 0)
		{
			InputSystem::SingletonInstance()->SetMouseWheelDirection(1);
		}
		else if (mouseWheelDelta < 0)
		{
			InputSystem::SingletonInstance()->SetMouseWheelDirection(-1);
		}
		else
		{
			InputSystem::SingletonInstance()->SetMouseWheelDirection(0);
		}
		break;
	}

	}

	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}



void CreateOpenGLWindow(HINSTANCE applicationInstanceHandle)
{
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));

	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC;
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure);
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");

	RegisterClassEx(&windowClassDescription);

	const DWORD windowStyleFlags = (g_FullScreen) ? (WS_POPUP) : (WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED);
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT desktopRectangle;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRectangle);

	RECT windowRectangle;
	if (g_FullScreen)
	{
		windowRectangle = { 0, 0, WINDOW_PHYSICAL_WIDTH, WINDOW_PHYSICAL_HEIGHT };
	}
	else
	{
		windowRectangle = { OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_WIDTH, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_HEIGHT };
	}

	AdjustWindowRectEx(&windowRectangle, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, APP_NAME, -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));

	g_hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRectangle.left,
		windowRectangle.top,
		windowRectangle.right - windowRectangle.left,
		windowRectangle.bottom - windowRectangle.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL);

	ShowWindow(g_hWnd, SW_SHOW);
	SetForegroundWindow(g_hWnd);
	SetFocus(g_hWnd);

	g_DisplayDeviceContext = GetDC(g_hWnd);

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset(&pixelFormatDescriptor, 0, sizeof(pixelFormatDescriptor));

	pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	int pixelFormatCode = ChoosePixelFormat(g_DisplayDeviceContext, &pixelFormatDescriptor);
	SetPixelFormat(g_DisplayDeviceContext, pixelFormatCode, &pixelFormatDescriptor);
	g_OpenGLRenderingContext = wglCreateContext(g_DisplayDeviceContext);
	wglMakeCurrent(g_DisplayDeviceContext, g_OpenGLRenderingContext);
}



void RunMessagePump()
{
	MSG queuedMessage;
	for (;;)
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}
		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage);
	}
}



void Update()
{
	const float MINIMUM_SECONDS_PER_FRAME = (1.0f / 60.0f) - 0.0001f;
	static double lastFrameBeginTime = GetCurrentTimeInSeconds();

	float deltaTimeInSeconds = static_cast<float>(GetCurrentTimeInSeconds() - lastFrameBeginTime);
	while (deltaTimeInSeconds < MINIMUM_SECONDS_PER_FRAME)
	{
		deltaTimeInSeconds = static_cast<float>(GetCurrentTimeInSeconds() - lastFrameBeginTime);
	}
	lastFrameBeginTime = GetCurrentTimeInSeconds();

	g_TheGame->Update(deltaTimeInSeconds);
}



void Render()
{
	g_TheGame->Render();
	SwapBuffers(g_DisplayDeviceContext);
}



void RunFrame()
{
	++g_FrameNumber;
	InputSystem::SingletonInstance()->UpdateInputSystem();
	AudioSystem::SingletonInstance()->Update();
	RunMessagePump();
	Update();
	Render();
}



void Initialize(HINSTANCE applicationInstanceHandle)
{
	SetProcessDPIAware();
	CreateOpenGLWindow(applicationInstanceHandle);

	AudioSystem::InitializeAudioSystem();
	InputSystem::InitializeInputSystem(g_hWnd);
	AdvancedRenderer::InitializeRenderer(WINDOW_DIMENSIONS);
	g_FileUtilities = new FileUtilities();
	g_TheGame = new TheGame();
}



void Shutdown()
{
	delete g_TheGame;
	g_TheGame = nullptr;

	delete g_FileUtilities;
	g_FileUtilities = nullptr;

	AdvancedRenderer::UninitializeRenderer();
	InputSystem::UninitializeInputSystem();
	AudioSystem::UninitializeAudioSystem();
}



int WINAPI WinMain(HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int)
{
	UNUSED(commandLineString);
	srand((unsigned int)time(NULL));
	Initialize(applicationInstanceHandle);

	//InitializeCallStackSystem();
	//MemoryAnalyticsStartup();

	while (!g_IsQuitting)
	{
		RunFrame();
	}
	Shutdown();

	//MemoryAnalyticsShutdown();
	//UninitializeCallStackSystem();

	return 0;
}