#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define Gconst 6.67408e-11

#define PRECISIONMS 0
#define PRECISIONS 1
#define PRECISIONM 2
#define PRECISIONH 3

#define ADDBUTTONID 1
#define DELBUTTONID 2
#define SETBUTTONID 3
#define BUTTONIDOFFSET 3

#define RADIUS 0
#define CENTERX 1
#define CENTERY 2
#define SPEEDX 3
#define SPEEDY 4
#define MASS 5
#define ACCELERATIONX 6
#define ACCELERATIONY 7

#define CNUMBER 6

#define PLANETOFFSET (4*cyChar + 2*cyChar*CNUMBER)

char * szAppName = "Planets window";
const char PROPRIETIES[CNUMBER][9] = {"Radius\0", "Center_x\0", "Center_y\0", "Speed_x\0", "Speed_y\0", "Mass\0"};
const char PRECISION[4][15] = {"Milliseconds\n", "Seconds\n", "Minutes\n", "Hours\n"};

typedef struct Control{
    HWND control;
    HWND label;
    UINT_PTR control_number;
}Control;

typedef struct Planet{
    double proprieties[8];
    Control controls[CNUMBER];
    int wasSet;
}Planet;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void drawPlanets(HDC hdc, Planet* planets, RECT graph, int planetNumber, double meterToPixel);
void updatePositions(Planet* planets, int planetNumber, int precision);
void drawGraph(HDC hdc, RECT graph);
void drawPlanetInfo(HDC hdc,HWND hwnd, Planet* planets, RECT planetsInfo, HINSTANCE hInstance, int planetNumber, int cyChar);
void drawBasicButtons(HWND hwnd, RECT planetsInfo, HINSTANCE hInstance, int planetNumber, int cyChar);
void updateTextBoxes(Planet* planets, int planetNumber);
void drawSimulationInfo(HDC hdc, int precision, double meterToPixel);
double convertWithPrecision(double toConvert, int precision);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow){
    MSG msg;
    HWND hwnd;
    WNDCLASS wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hCursor = NULL;
    wndclass.hIcon = NULL;
    wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if(!RegisterClass(&wndclass)){
        MessageBox(NULL, TEXT("Window class registration failed!"), szAppName, MB_ICONERROR);
    }

    hwnd = CreateWindow(szAppName,
                        TEXT("Planets"),
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&msg, NULL,0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){

    HDC hdc;
    PAINTSTRUCT ps;
    static int planetNumber = 0;
    static int iframe;
    static RECT view;
    static RECT graph;
    static RECT planetsInfo;
    static Planet *planets = NULL;
    static double meterToPixel = 0.0000001;
    static int precision = PRECISIONS;
    static TEXTMETRIC tm;
    static int cyChar;
    static int cxChar;
    static int totalOffset = 0;
    static HINSTANCE hInstance;

    switch(message){
        case WM_CREATE:
            hdc = GetDC(hwnd);
            iframe = 0;

            GetClientRect(hwnd, &view);
            GetTextMetrics(hdc, &tm);
            cyChar = tm.tmHeight+tm.tmExternalLeading;
            cxChar = tm.tmAveCharWidth;
            hInstance = (((LPCREATESTRUCT)lParam)->hInstance);
            InvalidateRect(hwnd, &view,1);
            ReleaseDC(hwnd, hdc);
            return 0;
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);

            drawGraph(hdc, graph);
            drawPlanets(hdc, planets, graph, planetNumber, meterToPixel);
            drawPlanetInfo(hdc, hwnd, planets, planetsInfo, hInstance, planetNumber, cyChar);
            drawBasicButtons(hwnd, planetsInfo, hInstance, planetNumber, cyChar);
            drawSimulationInfo(hdc, precision, meterToPixel);
            EndPaint(hwnd, &ps);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            GetClientRect(hwnd, &view);
            graph = view;
            graph.right = view.right - (view.right * 0.3);
            planetsInfo = view;
            planetsInfo.left = graph.right;
            return 0;
        case WM_KEYDOWN:

            switch (wParam)
            {
                case 74:
                    meterToPixel *= 10;
                    InvalidateRect(hwnd, &graph, 1);
                    break;
                case 75:
                    meterToPixel /= 10;
                    InvalidateRect(hwnd, &graph, 1);
                    break;
            case VK_RIGHT:
                //forward
                //calculate forces
                updateTextBoxes(planets, planetNumber);
                updatePositions(planets, planetNumber, precision);
                InvalidateRect(hwnd, &view, 1);
                break;
            default: break;
            }
        case WM_LBUTTONDOWN: {
            SetFocus(hwnd);
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) <=BUTTONIDOFFSET) {
                switch(LOWORD(wParam))
                {
                    case ADDBUTTONID:
                        planetNumber++;

                        if(planets == NULL)
                            planets = (Planet*) malloc(sizeof(Planet)*planetNumber);
                        else
                            planets = (Planet*) realloc(planets, sizeof(Planet)*planetNumber);

                        planets[planetNumber-1].wasSet = 0;
                        for(int i = 0; i < 8; i++)
                        {
                            planets[planetNumber-1].proprieties[i] = 0;
                        }

                        drawBasicButtons(hwnd, planetsInfo, hInstance, planetNumber, cyChar);
                        return 0;
                    case SETBUTTONID:
                        precision++;
                        if(precision > PRECISIONH)
                        {
                            precision = 0;
                        }
                        InvalidateRect(hwnd, &graph, 1);
                        return 0;
                    case DELBUTTONID:
                        if(planetNumber == 0)
                            return 0;

                        planetNumber--;

                        for(int i = 0; i<CNUMBER; i++)
                        {
                            DestroyWindow(planets[planetNumber].controls[i].label);
                            DestroyWindow(planets[planetNumber].controls[i].control);
                        }
                        if(planetNumber == 0)
                        {
                            free(planets);
                            planets = NULL;
                            return 0;
                        }
                        else
                        {
                            planets = (Planet*) realloc(planets, sizeof(Planet)*planetNumber);
                        }
                        drawBasicButtons(hwnd, planetsInfo, hInstance, planetNumber, cyChar);
                        return 0;
                }
            }
            else
            {
                //else : message came from a planet input field, check which one was it
                for(int i = 0; i<planetNumber; i++)
                {
                    for(int j = 0; j < CNUMBER; j++)
                    {
                        if(planets[i].controls[j].control_number == LOWORD(wParam))
                        {
                            char buf[256];
                            memset(buf, 0, sizeof(buf));
                            GetWindowText(planets[i].controls[j].control, buf, sizeof(buf));
                            planets[i].proprieties[j] = atof(buf);
                            InvalidateRect(hwnd, &graph, 1);
                        }
                    }
                }
            }
            break;
    }

    return DefWindowProc(hwnd,message,wParam,lParam);
}

void drawPlanets(HDC hdc, Planet* planets, RECT graph, int planetNumber, double meterToPixel)
{
    for(int i = 0; i< planetNumber; i++)
    {
        //calculate planets boxes dimensions
        double max_posx, max_posy;
        double min_posx, min_posy;

        min_posx = planets[i].proprieties[CENTERX] - planets[i].proprieties[RADIUS];
        max_posx = min_posx + 2*planets[i].proprieties[RADIUS];

        min_posy = planets[i].proprieties[CENTERY] - planets[i].proprieties[RADIUS];
        max_posy = min_posy + 2*planets[i].proprieties[RADIUS];

        //transform meters to pixels

        max_posx *= meterToPixel;
        min_posx *= meterToPixel;

        max_posy *= meterToPixel;
        min_posy *= meterToPixel;

        //transform into world coordinates

        max_posx += (graph.right/2.);
        min_posx += (graph.right/2.);

        max_posy += (graph.bottom/2.);
        min_posy += (graph.bottom/2.);

        //flip y coordinate

        max_posy = graph.bottom - max_posy;
        min_posy = graph.bottom - min_posy;

        //if smaller than minimum
        if(max_posx - min_posx < 10)
        {
            max_posx += 5;
            min_posx -= 5;
            max_posy += 5;
            min_posy -= 5;
        }

        Ellipse(hdc, max_posx, max_posy, min_posx, min_posy);
    }
}

void drawGraph(HDC hdc, RECT graph)
{
    HPEN greyPen = CreatePen(PS_SOLID, 1, RGB(192,192,192));
    SelectObject(hdc, greyPen);

    MoveToEx(hdc, graph.left, graph.bottom/2, NULL);
    LineTo(hdc, graph.right, graph.bottom/2);

    MoveToEx(hdc, graph.right/2, graph.top, NULL);
    LineTo(hdc, graph.right/2, graph.bottom);

    MoveToEx(hdc, graph.right , graph.top, NULL);
    LineTo(hdc, graph.right, graph.bottom);
    DeleteObject(greyPen);
    SelectObject(hdc, GetStockObject(BLACK_PEN));
}

void updatePositions(Planet* planets, int planetNumber, int precision)
{
    double force_x;
    double force_y;
    double force;
    double distance;
    double angle;

    for(int i = 0; i< planetNumber; i++){
        for(int j = 0; j<planetNumber; j++){
            if(j == i)
                continue;

            distance = pow(planets[i].proprieties[CENTERX]-planets[j].proprieties[CENTERX],2) + pow(planets[i].proprieties[CENTERY]-planets[j].proprieties[CENTERY],2);

            if(distance == 0)
                continue;
            
            force = (Gconst * planets[i].proprieties[MASS] * planets[j].proprieties[MASS]) / distance;

            angle = atan2(planets[i].proprieties[CENTERY] - planets[j].proprieties[CENTERY], planets[i].proprieties[CENTERX] - planets[j].proprieties[CENTERX]);

            force_x = cos(angle) * -force;
            force_y = sin(angle) * -force;

            planets[i].proprieties[ACCELERATIONX] += (force_x / planets[i].proprieties[MASS]);
            planets[i].proprieties[ACCELERATIONY] += (force_y / planets[i].proprieties[MASS]);

        }
    }

    //Update speeds
    for(int i = 0; i<planetNumber; i++)
    {
        planets[i].proprieties[SPEEDX] += planets[i].proprieties[ACCELERATIONX];
        planets[i].proprieties[SPEEDY] += planets[i].proprieties[ACCELERATIONY];
    }

    //update positions, reset accelerations
    for(int i = 0; i<planetNumber; i++)
    {
        planets[i].proprieties[CENTERX] += convertWithPrecision(planets[i].proprieties[SPEEDX],precision);
        planets[i].proprieties[CENTERY] += convertWithPrecision(planets[i].proprieties[SPEEDY],precision);
        planets[i].proprieties[ACCELERATIONX] = 0;
        planets[i].proprieties[ACCELERATIONY] = 0;
    }
}

void drawBasicButtons(HWND hwnd, RECT planetsInfo, HINSTANCE hInstance, int planetNumber, int cyChar)
{
    static HWND addButton = NULL;
    static HWND setButton = NULL;
    static HWND delButton = NULL;

    float width =  (planetsInfo.right-planetsInfo.left)*(1./3);
    if(addButton == NULL)
    {
        addButton = CreateWindow(
                TEXT("BUTTON"),
                TEXT("Add"),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                planetsInfo.left, planetNumber*PLANETOFFSET,
                width, 2*cyChar,
                hwnd,
                (HMENU) ADDBUTTONID,
                hInstance,
                NULL
                );
    }
    else
    {
        MoveWindow(
                addButton,
                planetsInfo.left, planetNumber*PLANETOFFSET,
                width, 2*cyChar,
                TRUE
        );
    }

    if(setButton == NULL)
    {
        setButton = CreateWindow(
                TEXT("BUTTON"),
                TEXT("Set"),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                planetsInfo.left + (planetsInfo.right-planetsInfo.left)*(1./3), planetNumber*PLANETOFFSET,
                width, 2*cyChar,
                hwnd,
                (HMENU) SETBUTTONID,
                hInstance,
                NULL
        );
    }
    else
    {
        MoveWindow(
                setButton,
                planetsInfo.left + (planetsInfo.right-planetsInfo.left)*(1./3), planetNumber*PLANETOFFSET,
                width, 2*cyChar,
                TRUE
        );
    }

    if(delButton == NULL)
    {
        delButton = CreateWindow(
                TEXT("BUTTON"),
                TEXT("Del"),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                planetsInfo.left + (planetsInfo.right-planetsInfo.left)*(2./3), planetNumber*PLANETOFFSET,
                width, 2*cyChar,
                hwnd,
                (HMENU) DELBUTTONID,
                hInstance,
                NULL
        );
    }
    else
    {
        MoveWindow(
                delButton,
                planetsInfo.left + (planetsInfo.right-planetsInfo.left)*(2./3), planetNumber*PLANETOFFSET,
                width, 2*cyChar,
                TRUE
        );
    }
}

void drawPlanetInfo(HDC hdc, HWND hwnd, Planet* planets, RECT planetsInfo, HINSTANCE hInstance, int planetNumber, int cyChar)
{
    char buf[256];
    for(int i = 0; i<planetNumber; i++)
    {
        //1) Planet number, always
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "Planet n%d",i+1);
        TextOut(hdc, planetsInfo.left+10, i*PLANETOFFSET, TEXT(buf), strlen(buf));
        if(!planets[i].wasSet)
        {
            //2) set various controls
            for(int j = 0; j<CNUMBER; j++)
            {
                memset(buf, 0, sizeof(buf));
                snprintf(buf, sizeof(buf), "%s :",PROPRIETIES[j]);
                planets[i].controls[j].label = CreateWindow(
                        "STATIC",
                        buf,
                        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
                        planetsInfo.left, i*PLANETOFFSET+2*cyChar+2*cyChar*j,
                        (planetsInfo.right - planetsInfo.left) - (planetsInfo.right-planetsInfo.left)*0.7, 2*cyChar,
                        hwnd,
                        NULL,
                        NULL,
                        NULL
                        );
                memset(buf, 0, sizeof(buf));
                snprintf(buf, sizeof(buf), "%lf",planets[i].proprieties[j]);
                planets[i].controls[j].control_number =  BUTTONIDOFFSET + 1 + ((i*CNUMBER) + j);
                planets[i].controls[j].control = CreateWindow(
                        "EDIT", buf,
                        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                        planetsInfo.left + (planetsInfo.right-planetsInfo.left) * 0.3, i*PLANETOFFSET+2*cyChar+2*cyChar*j,
                        (planetsInfo.right-planetsInfo.left)*0.7, 2*cyChar,
                        hwnd,
                        (HMENU) planets[i].controls[j].control_number,
                        hInstance,
                        NULL);
            }
            planets[i].wasSet = 1;
        }
        else
        {
            for(int j = 0; j<CNUMBER; j++)
            {
                MoveWindow(
                        planets[i].controls[j].label,
                        planetsInfo.left, i*PLANETOFFSET+2*cyChar+2*cyChar*j,
                        (planetsInfo.right - planetsInfo.left) - (planetsInfo.right-planetsInfo.left)*0.7, 2*cyChar,
                        TRUE
                );
                MoveWindow(
                        planets[i].controls[j].control,
                        planetsInfo.left + (planetsInfo.right-planetsInfo.left) * 0.3, i*PLANETOFFSET+2*cyChar+2*cyChar*j,
                        (planetsInfo.right-planetsInfo.left)*0.7, 2*cyChar,
                        TRUE
                );
            }
        }
    }
}

void updateTextBoxes(Planet* planets, int planetNumber)
{
    char buf[256];

    for(int i = 0; i<planetNumber; i++)
    {
        for(int j = 0; j<CNUMBER; j++)
        {
            memset(buf, 0, sizeof(buf));
            snprintf(buf, sizeof(buf), "%lf", planets[i].proprieties[j]);
            SetWindowText(planets[i].controls[j].control, buf);
        }
    }
}

double convertWithPrecision(double toConvert, int precision)
{
    switch(precision){
        case PRECISIONMS:
            return toConvert/0.001;
        case PRECISIONS:
            return toConvert*1;
        case PRECISIONM:
            return toConvert*60;
        case PRECISIONH:
            return toConvert*60*60;
    }
}

void drawSimulationInfo(HDC hdc, int precision, double meterToPixel)
{
    char buf[256];
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "Meter to pixel: %.12lf - Precision: %s", meterToPixel, PRECISION[precision]);
    TextOut(hdc, 0, 0, TEXT(buf), strlen(buf));
}
