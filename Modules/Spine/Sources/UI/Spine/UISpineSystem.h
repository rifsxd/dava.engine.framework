#pragma once

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <UI/UISystem.h>
#include <Functional/Signal.h>

namespace DAVA
{
class UIControl;
class UIComponent;
class UISpineAttachControlsToBonesComponent;
class UISpineComponent;
class UIControlBackground;
class SpineBone;
class SpineSkeleton;

class UISpineSystem final : public UISystem
{
public:
    UISpineSystem();
    ~UISpineSystem() override;

    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

    void OnControlVisible(UIControl* control) override;
    void OnControlInvisible(UIControl* control) override;

    void Process(float32 elapsedTime) override;

    /** Set pause to all Spine animations. */
    void SetPause(bool pause);
    /** Return true is Spine system in pause. */
    bool IsPause() const;

    /** Rebuild links between all bones and controls for all loaded spine animations. */
    void RebuildAllBoneLinks();

    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationStart;
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationFinish;
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationComplete;
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/, const String& /*event*/> onAnimationEvent;

private:
    struct BoneLink
    {
        std::shared_ptr<SpineBone> bone;
        RefPtr<UIControl> control;
    };

    struct SpineNode
    {
        RefPtr<UISpineComponent> spine;
        RefPtr<UISpineAttachControlsToBonesComponent> bones;
        RefPtr<UIControlBackground> bg;
        RefPtr<SpineSkeleton> skeleton;
        Vector<BoneLink> boneLinks;
    };

    void AddNode(UISpineComponent* spine);
    void RemoveNode(UISpineComponent* spine);
    void BindBones(UISpineAttachControlsToBonesComponent* bones);
    void UnbindBones(UISpineAttachControlsToBonesComponent* bones);
    void BindBackground(UIControlBackground* bg);
    void UnbindBackground(UIControlBackground* bg);

    void BuildBoneLinks(SpineNode& node);

    UnorderedMap<UIControl*, SpineNode> nodes;
    bool isPause = false;
};
}