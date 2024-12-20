#include <basetyps.h>
#include <stdlib.h>
#include <wtypes.h>
#include <initguid.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <ntddkbd.h>

#pragma warning(disable:4201)

#include <setupapi.h>
#include <winioctl.h>

#pragma warning(default:4201)

#include "..\sys\public.h"
#include "..\wsk\public.h"

//-----------------------------------------------------------------------------
// 4127 -- Conditional Expression is Constant warning
//-----------------------------------------------------------------------------
#define WHILE(constant) \
__pragma(warning(disable: 4127)) while(constant); __pragma(warning(default: 4127))

DEFINE_GUID(GUID_DEVINTERFACE_KBFILTER,
    0x3fb7299d, 0x6847, 0x4490, 0xb0, 0xc9, 0x99, 0xe0, 0x98, 0x6a, 0xb8, 0x86);
// {3FB7299D-6847-4490-B0C9-99E0986AB886}

#define BUFFER_SIZE 128

typedef struct _KEY_BUFFER {
    UCHAR ScanCodes[BUFFER_SIZE];
    ULONG Count;
} KEY_BUFFER;

// 2D array for scan code to key name mapping
const char* KeyMap[8][16] = {
    {"NULL", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Backspace", "Tab"},
    {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "Enter", "LCtrl", "A", "S"},
    {"D", "F", "G", "H", "J", "K", "L", ";", "\"", "`", "LShift", "\\", "Z", "X", "C", "V"},
    {"B", "N", "M", ",", ".", "/", "RShift", "*", "Alt", "Space", "CapsLock", "F1", "F2", "F3", "F4", "F5"},
    {"F6", "F7", "F8", "F9", "F10", "Numlock", "Right", "Numpad7", "Numpad8", "Numpad9", "-", "Numpad4", "Numpad5", "Numpad6", "+", "Numpad1"},
    {"Numpad2", "Numpad3", "Numpad0", "Delete", "F5", "F6", "F7", "F11", "F12", "F10", "F11", "WinKey", "", "", "", ""},
    {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
    {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""}
};

// Helper function to convert scan code to key name
const char* ScanCodeToKeyName(UCHAR scanCode) {
    if (scanCode < 0x80) {
        return KeyMap[scanCode / 16][scanCode % 16]; // Row and column based on scan code
    }
    return "Unknown";
}

// Helper function to convert scan code to special key name
const char* ScanCodeToSpecialKeyName(UCHAR scanCode) {
    switch (scanCode) {
    case 0x52:
        return "Insert";
        break;
    case 0x4D:
        return "Right";
        break;
    case 0x4B:
        return "Left";
        break;
    case 0x50:
        return "Down";
        break;
    case 0x48:
        return "Up";
        break;

    default:
        return "Unknown";
    }
}

int
_cdecl
main(
    _In_ int argc,
    _In_ char* argv[]
)
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0;
    HANDLE                              fileKeyboard, fileWebsocket;
    ULONG                               i = 0;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    //
    // Open a handle to the device interface information set of all
    // present toaster class interfaces.
    //

    hardwareDeviceInfo = SetupDiGetClassDevs(
        (LPGUID)&GUID_DEVINTERFACE_KBFILTER,
        NULL, // Define no enumerator (global)
        NULL, // Define no
        (DIGCF_PRESENT | // Only Devices present
            DIGCF_DEVICEINTERFACE)); // Function class devices.
    if (INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        printf("SetupDiGetClassDevs failed: %x\n", GetLastError());
        return 0;
    }

    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    i = 0;

    //
    // Enumerate devices of toaster class
    //

    do {
        if (SetupDiEnumDeviceInterfaces(hardwareDeviceInfo,
            0, // No care about specific PDOs
            (LPGUID)&GUID_DEVINTERFACE_KBFILTER,
            i, //
            &deviceInterfaceData)) {

            if (deviceInterfaceDetailData) {
                free(deviceInterfaceDetailData);
                deviceInterfaceDetailData = NULL;
            }

            //
            // Allocate a function class device data structure to
            // receive the information about this particular device.
            //

            //
            // First find out required length of the buffer
            //

            if (!SetupDiGetDeviceInterfaceDetail(
                hardwareDeviceInfo,
                &deviceInterfaceData,
                NULL, // probing so no output buffer yet
                0, // probing so output buffer length of zero
                &requiredLength,
                NULL)) { // not interested in the specific dev-node
                if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
                    printf("SetupDiGetDeviceInterfaceDetail failed %d\n", GetLastError());
                    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
                    return FALSE;
                }

            }

            predictedLength = requiredLength;

            deviceInterfaceDetailData = malloc(predictedLength);

            if (deviceInterfaceDetailData) {
                deviceInterfaceDetailData->cbSize =
                    sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            }
            else {
                printf("Couldn't allocate %d bytes for device interface details.\n", predictedLength);
                SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
                return FALSE;
            }


            if (!SetupDiGetDeviceInterfaceDetail(
                hardwareDeviceInfo,
                &deviceInterfaceData,
                deviceInterfaceDetailData,
                predictedLength,
                &requiredLength,
                NULL)) {
                printf("Error in SetupDiGetDeviceInterfaceDetail\n");
                SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
                free(deviceInterfaceDetailData);
                return FALSE;
            }
            printf("%d) %s\n", ++i,
                deviceInterfaceDetailData->DevicePath);
        }
        else if (ERROR_NO_MORE_ITEMS != GetLastError()) {
            free(deviceInterfaceDetailData);
            deviceInterfaceDetailData = NULL;
            continue;
        }
        else
            break;

    } WHILE(TRUE);


    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);

    if (!deviceInterfaceDetailData)
    {
        printf("No device interfaces present\n");
        return 0;
    }

    fileWebsocket = CreateFileW(
        L"\\\\.\\WskSampleDevice", // U¿yj L"..." dla ³añcucha Unicode
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (fileWebsocket == INVALID_HANDLE_VALUE) {
        printf("Failed to open device: %d\n", GetLastError());
        return 1;
    }

    fileKeyboard = CreateFile(deviceInterfaceDetailData->DevicePath,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL, // no SECURITY_ATTRIBUTES structure
        OPEN_EXISTING, // No special create flags
        0, // No special attributes
        NULL);

    if (INVALID_HANDLE_VALUE == fileKeyboard) {
        printf("Error in CreateFile: %x\n", GetLastError());
        free(deviceInterfaceDetailData);
        CloseHandle(fileWebsocket);
        return 0;
    }

    char message[] = "Hello from user mode!";
    DWORD bytesReturned;

    if (!DeviceIoControl(
        fileWebsocket,
        IOCTL_SEND_DATA,
        (PVOID)message, // Input buffer
        sizeof(message), // Input buffer length
        NULL, // No output buffer
        0, // No output buffer length
        &bytesReturned,
        0
    )) {
        printf("Failed to send data: %d\n", GetLastError());
    }
    else {
        printf("Data sent successfully\n");
    }

    //
    // Send an IOCTL to retrieve the keyboard attributes
    // These are cached in the kbfiltr
    //

    //while (TRUE) {
    //    DWORD dwBytesReturned = 0;
    //    KEY_BUFFER keyBuffer;

    //    BOOL result = DeviceIoControl(
    //        fileKeyboard,
    //        IOCTL_DISPATCH_KEYBOARD_IO,
    //        NULL, 0,
    //        &keyBuffer, sizeof(KEY_BUFFER),
    //        &dwBytesReturned,
    //        NULL
    //    );

    //    if (result) {
    //        if (keyBuffer.Count > 0) {
    //            for (i = 0; i < keyBuffer.Count; i++) {
    //                if (keyBuffer.ScanCodes[i] == 0x00) {
    //                    break;
    //                }
    //                else {
    //                    char key;
    //                    if (keyBuffer.ScanCodes[i] == 0x2a) {
    //                        // special signs, another table
    //                        key = keyBuffer.ScanCodes[i];
    //                        UCHAR next = keyBuffer.ScanCodes[i + 1];
    //                        i++;
    //                        printf("ScanCode: 0x%x 0x%x -> %s\n", key, next, ScanCodeToSpecialKeyName(next));
    //                    }
    //                    else {
    //                        printf("ScanCode: 0x%x -> %s\n", keyBuffer.ScanCodes[i], ScanCodeToKeyName(keyBuffer.ScanCodes[i]));
    //                    }
    //                }
    //            }
    //        }
    //    }
    //    else {
    //        printf("Error during reading keys from buffer: %x\n", GetLastError());
    //    }

    //    Sleep(500);
    //}

    free(deviceInterfaceDetailData);
    CloseHandle(fileKeyboard);
    CloseHandle(fileWebsocket);
    return 0;
}
