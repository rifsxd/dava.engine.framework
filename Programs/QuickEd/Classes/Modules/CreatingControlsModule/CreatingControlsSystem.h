#pragma once

#include "Classes/EditorSystems/BaseEditorSystem.h"
#include "Classes/EditorSystems/EditorSystemsManager.h"

#include <TArc/Core/FieldBinder.h>

#include <Math/Vector.h>

namespace DAVA
{
class UI;
}

class CreatingControlsSystem final : public BaseEditorSystem
{
public:
    CreatingControlsSystem(DAVA::ContextAccessor* accessor, DAVA::UI* ui);

    // BaseEditorSystem
    eSystems GetOrder() const override;

    void SetCreateByClick(const DAVA::String& controlYamlString);
    void CancelCreateByClick();

private:
    // BaseEditorSystem
    eDragState RequireNewState(DAVA::UIEvent* currentInput, eInputSource inputSource) override;
    bool CanProcessInput(DAVA::UIEvent* currentInput, eInputSource inputSource) const override;
    void ProcessInput(DAVA::UIEvent* currentInput, eInputSource inputSource) override;

    void BindFields();

    void OnPackageChanged(const DAVA::Any& package);
    void OnProjectPathChanged(const DAVA::Any& projectPath);

    void AddControlAtPoint(const DAVA::Vector2& point);

private:
    DAVA::UI* ui = nullptr;
    std::unique_ptr<DAVA::FieldBinder> fieldBinder;
    DAVA::DataWrapper documentDataWrapper;
    DAVA::String controlYamlString;
    bool isEscPressed = false;
    bool isLMBPressed = false;
};
