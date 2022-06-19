#pragma once

namespace DAVA
{
enum class eApplicationMode
{
    GUI_MODE,
    CONSOLE_MODE,
    TEST_MODE
};

void SetupToolsAssertHandlers(eApplicationMode mode);
bool IsInsideAssertHandler();
} // namespace DAVA
