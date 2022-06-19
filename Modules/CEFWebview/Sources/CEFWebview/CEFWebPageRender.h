#pragma once

#include <cef/include/cef_render_handler.h>

#include "Functional/Signal.h"
#include "Render/2D/Sprite.h"
#include "UI/UIControl.h"

namespace DAVA
{
class Window;
class CEFWebPageRender : public CefRenderHandler
{
    IMPLEMENT_REFCOUNTING(CEFWebPageRender);

public:
    CEFWebPageRender(Window* w, float32 k);
    ~CEFWebPageRender();

    void ClearRenderSurface();
    UIControlBackground* GetContentBackground();

    void SetVisible(bool visibility);
    bool IsVisible() const;
    void SetBackgroundTransparency(bool value);
    void SetViewRect(const Rect& rect);
    void ShutDown();

    void SetScale(float32 k);

private:
    void ConnectToSignals();
    void DisconnectFromSignals();

    // CefRenderHandler interface implementation
    bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override;

    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects,
                 const void* buffer, int width, int height) override;

    void OnCursorChange(CefRefPtr<CefBrowser> browser,
                        CefCursorHandle cursor,
                        CursorType type,
                        const CefCursorInfo& custom_cursor_info);

    void PostProcessImage();
    void AppyTexture();
    void RestoreTexture();

    void SetCursor(CefCursorHandle cursor);
    void ResetCursor();

    int imageWidth = 0;
    int imageHeight = 0;
    Vector<uint8> imageData;
    Rect logicalViewRect;
    RefPtr<UIControlBackground> contentBackground;
    bool transparency = true;
    bool isActive = true;
    bool isVisible = true;
    CursorType currentCursorType = CursorType::CT_POINTER;
    Window* window = nullptr;
    float32 scale = 1.f;
    unsigned webViewID = 0;
};

} // namespace DAVA
