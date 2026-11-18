#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <hidpi.h>
#include <string>
#include <vector>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")
#pragma comment(lib, "hidparse.lib")

namespace {
constexpr UINT WM_TRAY = WM_APP + 1;
constexpr UINT ID_TRAY = 1;
NOTIFYICONDATAW g_nid{};
HWND g_hwnd{};
std::vector<std::wstring> g_devices;

std::wstring ReadHidString(HANDLE h, bool manufacturer) {
    wchar_t buffer[256]{};
    BOOLEAN ok = manufacturer
        ? HidD_GetManufacturerString(h, buffer, sizeof(buffer))
        : HidD_GetProductString(h, buffer, sizeof(buffer));
    return ok ? buffer : L"";
}

void ScanDevices() {
    g_devices.clear();
    GUID guid{};
    HidD_GetHidGuid(&guid);
    HDEVINFO set = SetupDiGetClassDevsW(&guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (set == INVALID_HANDLE_VALUE) return;

    for (DWORD i = 0;; ++i) {
        SP_DEVICE_INTERFACE_DATA iface{sizeof(iface)};
        if (!SetupDiEnumDeviceInterfaces(set, nullptr, &guid, i, &iface)) break;

        DWORD needed = 0;
        SetupDiGetDeviceInterfaceDetailW(set, &iface, nullptr, 0, &needed, nullptr);
        if (!needed) continue;
        std::vector<BYTE> bytes(needed);
        auto* detail = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(bytes.data());
        detail->cbSize = sizeof(*detail);
        if (!SetupDiGetDeviceInterfaceDetailW(set, &iface, detail, needed, nullptr, nullptr)) continue;

        HANDLE h = CreateFileW(detail->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) continue;

        HIDD_ATTRIBUTES attr{sizeof(attr)};
        if (HidD_GetAttributes(h, &attr)) {
            PHIDP_PREPARSED_DATA pp = nullptr;
            if (HidD_GetPreparsedData(h, &pp)) {
                HIDP_CAPS caps{};
                if (HidP_GetCaps(pp, &caps) == HIDP_STATUS_SUCCESS &&
                    caps.UsagePage == 0x01 && caps.Usage == 0x02) {
                    auto maker = ReadHidString(h, true);
                    auto product = ReadHidString(h, false);
                    std::wstring line = product.empty() ? L"HID mouse" : product;
                    if (!maker.empty()) line = maker + L" " + line;
                    line += L" — заряд: недоступен (стандартный отчёт батареи не прочитан)";
                    g_devices.push_back(std::move(line));
                }
                HidD_FreePreparsedData(pp);
            }
        }
        CloseHandle(h);
    }
    SetupDiDestroyDeviceInfoList(set);
}

void ShowStatus() {
    ScanDevices();
    std::wstring text = L"Universal Mouse Battery Monitor\n\n";
    if (g_devices.empty()) {
        text += L"Мышиные HID-устройства не найдены.";
    } else {
        for (const auto& item : g_devices) text += L"• " + item + L"\n";
    }
    text += L"\nПримечание: заряд показывается только если мышь раскрывает его через совместимый HID-протокол.";
    MessageBoxW(g_hwnd, text.c_str(), L"Universal Mouse Battery Monitor", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_TRAY) {
        if (lp == WM_LBUTTONUP || lp == WM_RBUTTONUP) ShowStatus();
        return 0;
    }
    if (msg == WM_DESTROY) {
        Shell_NotifyIconW(NIM_DELETE, &g_nid);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    const wchar_t* cls = L"UniversalMouseBatteryMonitorWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.lpszClassName = cls;
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassW(&wc);

    g_hwnd = CreateWindowExW(0, cls, L"Universal Mouse Battery Monitor",
        WS_OVERLAPPED, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
    if (!g_hwnd) return 1;

    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = g_hwnd;
    g_nid.uID = ID_TRAY;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAY;
    g_nid.hIcon = LoadIconW(nullptr, IDI_INFORMATION);
    lstrcpynW(g_nid.szTip, L"Mouse battery: open for device status", ARRAYSIZE(g_nid.szTip));
    if (!Shell_NotifyIconW(NIM_ADD, &g_nid)) return 2;

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
