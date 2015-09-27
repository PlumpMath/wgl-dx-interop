#include <Windows.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include <wrl.h>
#include <cstdlib>
#include "WTFEngine.hpp"
#include "WTFWindow.hpp"

using namespace WTFDanmaku;
using Microsoft::WRL::ComPtr;

LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static const GUID sIID_IDXGISwapChain1 = { 0x790a45f7, 0x0d42, 0x4876, { 0x98, 0x3a, 0x0a, 0x55, 0xcf, 0xe6, 0xf4, 0xaa } };

int sWidth = 1280;
int sHeight = 720;

bool sWtfInited = false;
WTFEngine* engine = nullptr;
WTFWindow* window = nullptr;

ComPtr<IDXGIFactory2> sDxgiFactory;
ComPtr<IDXGISurface> sDxgiSurface;
ComPtr<IDXGISwapChain1> sDxgiSwapChain;

ComPtr<IDCompositionDevice> sDCompDevice;
ComPtr<IDCompositionTarget> sDCompTarget;
ComPtr<IDCompositionVisual> sDCompVisual;

int main(int argc, char** argv) {
    window = new WTFWindow(GetModuleHandle(NULL), SW_SHOWNORMAL);
    window->SetCustomWindowProc(CustomWndProc);
    window->Initialize(WS_EX_NOREDIRECTIONBITMAP, sWidth, sHeight, L"WGL DX Interop");
    window->SetHitTestOverEnabled(false);
    int ret = window->Run();
    delete window;
    return ret;
}

void InitializeVideoSurface() {
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&sDxgiFactory));
    if (FAILED(hr))
        DebugBreak();



    DXGI_SWAP_CHAIN_DESC1 desc = { 0 };
    desc.Width = sWidth;
    desc.Height = sHeight;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Stereo = false;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferCount = 2;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    desc.Flags = 0;

}

void InitializeDComp(HWND hwnd, void* danmakuSwapChain) {
    HRESULT hr = DCompositionCreateDevice2(nullptr, IID_PPV_ARGS(&sDCompDevice));
    if (FAILED(hr))
        DebugBreak();

    hr = sDCompDevice->CreateTargetForHwnd(hwnd, true, &sDCompTarget);
    if (FAILED(hr))
        DebugBreak();

    hr = sDCompDevice->CreateVisual(&sDCompVisual);
    if (FAILED(hr))
        DebugBreak();

    sDCompVisual->SetBitmapInterpolationMode(DCOMPOSITION_BITMAP_INTERPOLATION_MODE_LINEAR);
    sDCompVisual->SetCompositeMode(DCOMPOSITION_COMPOSITE_MODE_SOURCE_OVER);
    sDCompVisual->SetContent((IUnknown*)danmakuSwapChain);

    sDCompTarget->SetRoot(sDCompVisual.Get());
    hr = sDCompDevice->Commit();
    if (FAILED(hr))
        DebugBreak();
}

void InitializeWTF(HWND hwnd) {
    if (!sWtfInited) {
        engine = new WTFEngine;
        engine->InitializeOffscreen(sWidth, sHeight);

        void* pSwapChain = nullptr;
        int ret = engine->QuerySwapChain(&sIID_IDXGISwapChain1, &pSwapChain);
        if (ret) DebugBreak();

        InitializeDComp(hwnd, pSwapChain);

        engine->SetFontName(L"SimHei");
        engine->SetFontWeight(700);
        engine->SetFontScaleFactor(1.0f);
        engine->SetCompositionOpacity(0.9f);

        engine->LoadBilibiliFile("F:\\Downloads\\469970.xml");

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