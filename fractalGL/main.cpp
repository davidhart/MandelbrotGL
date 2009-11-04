#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <iostream>
#include <sstream>

#define WNDCLASSNAME	"OpenGLWndClass"
#define WINDOW_TITLE	"Can haz fractal?"
#define WINDOW_HEIGHT	768
#define WINDOW_WIDTH	1024
#define WINDOW_STYLE	WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS
#define WINDOW_EX_STYLE	WS_EX_APPWINDOW | WS_EX_WINDOWEDGE

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool InitWindow(HINSTANCE hInstance);
void DeInitWindow();
void Draw();
void GenMandelBrot();
void GenWindowTitle();

HWND appWindow;
HDC hdc;
HGLRC hrc;

float zoom;
float positionX;
float positionY;
int iterations;

void GenWindowTitle()
{
	std::stringstream str;
	str << "Fractal, Zoom: " << zoom << ", Position: (" << positionX << "," << positionY << "), Iterations: " << iterations;
	SetWindowText(appWindow, str.str().c_str());

}

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

struct rgbcolour
{
	float r;
	float g;
	float b;
};

rgbcolour points[WINDOW_WIDTH][WINDOW_HEIGHT];

void GenMandelBrot()
{
	for (int xint = 0; xint < WINDOW_WIDTH; xint++)
	{
		float x0 = (float)(xint-WINDOW_WIDTH/2) *2.0f/WINDOW_HEIGHT * (float)WINDOW_HEIGHT/WINDOW_WIDTH * zoom + positionX;

		for (int yint = 0; yint < WINDOW_HEIGHT; yint++)
		{
			float y0 = (float)(yint-WINDOW_HEIGHT/2.0f) * 2.0f/WINDOW_HEIGHT * zoom + positionY;
			float x = 0;
			float y = 0;

			int iteration = 0;
 
			while ( x*x + y*y <= (2*2)  &&  iteration < iterations ) 
			{
				float xtemp = x*x - y*y + x0;
				y = 2*x*y + y0;

				x = xtemp;

				iteration++;
			}

			points[xint][yint].g = 0.0f;
			points[xint][yint].b = 0.0f;

			if ( iteration == iterations )
			{
				points[xint][yint].r = 1.0f;

			}
			else
			{
				points[xint][yint].r = ((float)iteration/(float)iterations);
			}
		}
	}
	GenWindowTitle();
}

void DrawPointsSet()
{
	for (int x = 0; x < WINDOW_WIDTH; x++)
	{
		for (int y = 0; y < WINDOW_HEIGHT; y++)
		{
			glColor3f(points[x][y].r,points[x][y].g, points[x][y].b);
			glVertex2i(x, y);
		}
	}
}

void Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	glBegin(GL_POINTS);

	DrawPointsSet();

	glEnd();

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
	//gluOrtho2D((double)WINDOW_WIDTH/WINDOW_HEIGHT, (double)WINDOW_WIDTH/WINDOW_HEIGHT, -1.0, 1.0);
	gluOrtho2D(0.0, (double)WINDOW_WIDTH, (double)WINDOW_HEIGHT, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	zoom = 1.0f;
	positionX = 0.0f;
	positionX = 0.0f;
	iterations = 100;

	GenMandelBrot();
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

	case WM_KEYDOWN:
		if (lParam & 0x00000001)
		{
			if (wParam == VK_ADD)
			{
				zoom -= 0.25f*zoom;
				GenMandelBrot();
			}
			else if (wParam == VK_SUBTRACT)
			{
				zoom += 0.33333333333f*zoom;
				GenMandelBrot();
			}
			else if (wParam == VK_UP)
			{
				positionY -= 0.25f * zoom;
				GenMandelBrot();
			}
			else if (wParam == VK_DOWN)
			{
				positionY += 0.25f * zoom;
				GenMandelBrot();
			}
			else if (wParam == VK_LEFT)
			{
				positionX -= 0.25f * zoom;
				GenMandelBrot();
			}
			else if (wParam == VK_RIGHT)
			{
				positionX += 0.25f * zoom;
				GenMandelBrot();
			}
		}
		break;

	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}