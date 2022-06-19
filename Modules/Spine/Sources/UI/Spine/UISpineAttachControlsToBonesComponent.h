#pragma once

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>
#include <UI/Components/UIComponent.h>

namespace DAVA
{
/** Component for attach Spine's bones to UIControls.
UIControls each frame gets position, scale and angle from attached bones.
*/
class UISpineAttachControlsToBonesComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UISpineAttachControlsToBonesComponent, UIComponent);
    DECLARE_UI_COMPONENT(UISpineAttachControlsToBonesComponent);

public:
    /** Describe attach information as pair of bone's name and path to nested control.*/
    struct AttachInfo
    {
        String boneName; //<! Name of bone.
        String controlPath; //<! Path to nested control.

        bool operator==(const AttachInfo& other) const;
        bool operator!=(const AttachInfo& other) const;
    };

    /** Default constructor. */
    UISpineAttachControlsToBonesComponent();
    /** Copy constructor. */
    UISpineAttachControlsToBonesComponent(const UISpineAttachControlsToBonesComponent& src);
    /** Removed assignment  operator. */
    UISpineAttachControlsToBonesComponent& operator=(const UISpineAttachControlsToBonesComponent&) = delete;

    UISpineAttachControlsToBonesComponent* Clone() const override;

    /** Return current attachent informations as vector of AttachInfo. */
    const Vector<AttachInfo>& GetBinds() const;
    /** Set current attachent informations as vector of AttachInfo. */
    void SetBinds(const Vector<AttachInfo>& binds);

protected:
    ~UISpineAttachControlsToBonesComponent() override;

    /** Return attach information as string. */
    const String& GetBindsAsString() const;
    /** Set attach information from string. */
    void SetBindsFromString(const String& bindsStr);

private:
    Vector<AttachInfo> bonesBinds;
    String cachedBindsString;

    /** Convert current attach information to string. */
    void MakeBindsString();
    /** Signal Spine system about component modification. */
    void Modify();
};

inline const Vector<UISpineAttachControlsToBonesComponent::AttachInfo>& UISpineAttachControlsToBonesComponent::GetBinds() const
{
    return bonesBinds;
}

inline const String& UISpineAttachControlsToBonesComponent::GetBindsAsString() const
{
    return cachedBindsString;
}
}