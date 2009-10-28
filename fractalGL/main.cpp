#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

#define WNDCLASSNAME	"OpenGLWndClass"
#define WINDOW_TITLE	"OpenGL Window"
#define WINDOW_HEIGHT	768
#define WINDOW_WIDTH	1024
#define WINDOW_STYLE	WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS
#define WINDOW_EX_STYLE	WS_EX_APPWINDOW | WS_EX_WINDOWEDGE

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool InitWindow(HINSTANCE hInstance);
void DeInitWindow();
void Draw();
void DrawMandelBrot();

HWND appWindow;
HDC hdc;
HGLRC hrc;

int WINAPI WinMain(      
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
	if ( !InitWindow(hInstance) )
		return 0;

	bool running = true;
	MSG msg;

	while ( running )
	{
		while (PeekMessage(&msg, appWindow, 0, 0, PM_REMOVE))
		{
			if ( msg.message == WM_QUIT )
			{
				running = false;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		Draw();
	}

	DeInitWindow();

	return 0;
}

void DrawMandelBrot()
{
	for (float x0 = -(double)WINDOW_WIDTH/WINDOW_HEIGHT; x0 < (double)WINDOW_WIDTH/WINDOW_HEIGHT; x0 += 2.0/WINDOW_HEIGHT*WINDOW_HEIGHT/WINDOW_WIDTH)
	{

		for (float y0 = -1.0; y0 < 1.0; y0+= 2.0/WINDOW_HEIGHT)
		{
			float x = 0;
			float y = 0;

			int iteration = 0;
			int max_iteration = 100;
 
			while ( x*x + y*y <= (2*2)  &&  iteration < max_iteration ) 
			{
				float xtemp = x*x - y*y + x0;
				y = 2*x*y + y0;

				x = xtemp;

				iteration++;
			}
 
			float r = 0;
			if ( iteration != max_iteration )
			{
				r = (float)iteration/(float)max_iteration;
				glColor3f(r, 0.0f, 0.0f);
			}
			else
			{
				glColor3f(0.0f, 0.0f, 0.0f);
			}


			glVertex2f(x0, y0);
		}
	}
}

void Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	

	
	glBegin(GL_POINTS);

	DrawMandelBrot();

	glEnd();

	/*
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	glVertex2f(0, 0);
	glVertex2f(0, 1);
	glVertex2f(1, 0);

	glEnd();
	*/
	

	SwapBuffers(hdc);
}

bool InitWindow(HINSTANCE hInstance)
{
	WNDCLASS	wc;
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= WNDCLASSNAME;

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL,"Error registering window class","",MB_OK);
		return false;
	}

	RECT windowRect;
	windowRect.left = 0;
	windowRect.right = WINDOW_WIDTH;
	windowRect.top = 0;
	windowRect.bottom = WINDOW_HEIGHT;
	AdjustWindowRectEx(&windowRect, WINDOW_STYLE, FALSE, WINDOW_EX_STYLE);

	appWindow = CreateWindowEx(	WINDOW_EX_STYLE,
								WNDCLASSNAME,
								WINDOW_TITLE,
								WINDOW_STYLE,
								0, 0,
								windowRect.right - windowRect.left,
								windowRect.bottom - windowRect.top,
								NULL,
								NULL,
								hInstance,
								NULL );

	if (appWindow == NULL)
	{
		MessageBox(NULL,"Could not create window","",MB_OK);
		return false;
	}

	hdc = GetDC(appWindow);

	if ( hdc == NULL )
	{
		DeInitWindow();
		MessageBox(NULL,"Could not get DC","",MB_OK);
		return false;
	}

	static	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		32,											// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	int pf = ChoosePixelFormat(hdc, &pfd);

	if ( pf == 0 )
	{
		DeInitWindow();
		MessageBox(NULL,"Could not choose PF","",MB_OK);
		return false;
	}

	if (!SetPixelFormat(hdc, pf, &pfd))
	{
		DeInitWindow();
		MessageBox(NULL,"Could not set PF","",MB_OK);
		return false;
	}


	hrc = wglCreateContext(hdc);

	if ( hrc == NULL )
	{
		DeInitWindow();
		MessageBox(NULL,"Could not get RC","",MB_OK);
		return false;
	}

	if ( !wglMakeCurrent(hdc, hrc) )
	{
		DeInitWindow();
		MessageBox(NULL,"Can't activate context","",MB_OK);
		return false;
	}

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D((double)WINDOW_WIDTH/WINDOW_HEIGHT, (double)WINDOW_WIDTH/WINDOW_HEIGHT, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	return true;
}

void DeInitWindow()
{
	if (hrc != NULL)
	{
		wglDeleteContext(hrc);
		hrc = NULL;
	}
	if (hdc != NULL)
	{
		hdc = NULL;
	}
	if (appWindow != NULL)
	{
		DestroyWindow(appWindow);
		appWindow = NULL;
	}
}

LRESULT CALLBACK WndProc(	HWND	hWnd,
							UINT	uMsg,
							WPARAM	wParam,
							LPARAM	lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}