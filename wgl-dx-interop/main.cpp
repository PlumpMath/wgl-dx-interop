#include <Windows.h>
#include <cstdlib>
#include "WTFEngine.hpp"
#include "WTFWindow.hpp"

using namespace WTFDanmaku;

LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static const GUID sIID_IDXGISwapChain1 = { 0x790a45f7, 0x0d42, 0x4876, { 0x98, 0x3a, 0x0a, 0x55, 0xcf, 0xe6, 0xf4, 0xaa } };

bool sWtfInited = false;
WTFEngine* engine = nullptr;
WTFWindow* window = nullptr;

int main(int argc, char** argv) {
    window = new WTFWindow(GetModuleHandle(NULL), SW_SHOWNORMAL);
    window->SetCustomWindowProc(CustomWndProc);
    window->Initialize(WS_EX_NOREDIRECTIONBITMAP, 1280, 720, L"WGL DX Interop");
    window->SetHitTestOverEnabled(false);
    int ret = window->Run();
    delete window;
    return ret;
}

void InitializeWTF(HWND hwnd) {
    if (!sWtfInited) {
        engine = new WTFEngine;
        engine->Initialize((void*)hwnd);
        //engine->InitializeOffscreen(1280, 720);

        void* pSwapChain = nullptr;
        int ret = engine->QuerySwapChain(&sIID_IDXGISwapChain1, &pSwapChain);
        if (ret) DebugBreak();

        engine->SetFontName(L"SimHei");
        engine->SetFontWeight(700);
        engine->SetFontScaleFactor(1.0f);
        engine->SetCompositionOpacity(0.9f);

        //engine->LoadBilibiliFile("F:\\Downloads\\469970.xml");

        sWtfInited = true;
    }
    engine->Start();
}

void ResizeWTF(uint32_t width, uint32_t height) {
    if (engine) {
        engine->Resize(width, height);
    }
}

void RButtonWTF() {
    if (engine) {
        engine->AddLiveDanmaku(Type::Scrolling, 0, L"Fuck you", 25, 0xFFFFFFFF, 0, 0);
        //engine->Pause();
        //engine->SetFontScaleFactor(engine->GetFontScaleFactor() + 0.05f);
        //engine->SeekTo(engine->GetCurrentPosition() + 1000);
    }
}

void ReleaseWTF() {
    if (engine) {
        if (engine->IsRunning()) {
            engine->Stop();
        }
        engine->Terminate();
        delete engine;
        engine = nullptr;
        sWtfInited = false;
    }
}

LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_SIZE:
            if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
                ResizeWTF(LOWORD(lParam), HIWORD(lParam));
            }
            break;
        case WM_LBUTTONDOWN:
            InitializeWTF(hwnd);
            break;
        case WM_RBUTTONDOWN:
            RButtonWTF();
            break;
        case WM_DESTROY:
            ReleaseWTF();
    }
    return window->DefaultWindowProc(hwnd, message, wParam, lParam);
}