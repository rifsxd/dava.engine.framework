#pragma once

namespace DAVA
{
#if defined(__DAVAENGINE_MACOS__)
void MakeAppForeground();
void FixOSXFonts();
void RestoreMenuBar();
#endif
} // namespace DAVA