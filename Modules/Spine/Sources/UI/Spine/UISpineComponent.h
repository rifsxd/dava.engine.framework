#pragma once

#include <FileSystem/FilePath.h>
#include <Functional/Signal.h>
#include <Reflection/Reflection.h>
#include <UI/Components/UIComponent.h>

namespace DAVA
{
/** Component for attach Spine animation to UIControl.
For display animation also need append UIControlBackground compoent to same
UIControl and set in it `DRAW_BATCH` draw type.
*/
class UISpineComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UISpineComponent, UIComponent);
    DECLARE_UI_COMPONENT(UISpineComponent);

public:
    /** Available animation states. */
    enum AnimationState
    {
        STOPPED = 0, //<! Stop animation
        PLAYED //!< Playback animation
    };

    /** Default constructor. */
    UISpineComponent() = default;
    /** Copy constructor. */
    UISpineComponent(const UISpineComponent& copy);
    /** Removed assignment  operator. */
    UISpineComponent& operator=(const UISpineComponent&) = delete;

    UISpineComponent* Clone() const override;

    /** Return skeleton data file path. */
    const FilePath& GetSkeletonPath() const;
    /** Set skeleton data file path. */
    void SetSkeletonPath(const FilePath& path);

    /** Return atlas texture file path. */
    const FilePath& GetAtlasPath() const;
    /** Set atlas texture file path. */
    void SetAtlasPath(const FilePath& path);

    /** Return animation state. */
    const AnimationState& GetAnimationState() const;
    /** Set animation state. */
    void SetAnimationState(const AnimationState& state);

    /** Return current animation name. */
    const String& GetAnimationName() const;
    /** Set current animation name. */
    void SetAnimationName(const String& name);

    /** Return available animations names. */
    const Vector<String>& GetAnimationsNames() const;
    /** Set available animations names. */
    void SetAnimationsNames(const Vector<String>& names);

    /** Return current skin name. */
    const String& GetSkinName() const;
    /** Set current skin name. */
    void SetSkinName(const String& name);

    /** Return available skins names. */
    const Vector<String>& GetSkinsNames() const;
    /** Set available skins names. */
    void SetSkinsNames(const Vector<String>& names);

    /** Return animation time scale (speed). */
    float32 GetTimeScale() const;
    /** Set animation time scale (speed). */
    void SetTimeScale(float32 scale);

    /** Return looped playback flag. */
    bool IsLoopedPlayback() const;
    /** Set looped playback flag. */
    void SetLoopedPlayback(bool loop);

    /** Emit slots when new animations starts with specified
    component's pointer and track's index. */
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationStart;
    /** Emit slots when animations finished with specified
    component's pointer and track's index. */
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationFinish;
    /** Emit slots when animation complete (every loop) with specified
    component's pointer and track's index. */
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationComplete;
    /** Emit slots when animation fire some user event with specified
    component's pointer, track's index and user's event string. */
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/, const String& /*event*/> onAnimationEvent;

protected:
    ~UISpineComponent() override;

private:
    /* Resources */
    FilePath skeletonPath;
    FilePath atlasPath;

    /* State */
    String animationName;
    Vector<String> animationsNames;
    AnimationState animationState = STOPPED;
    String skinName;
    Vector<String> skinsNames;
    float32 timeScale = 1.f;
    bool animationLooped = false;

    /** Signal Spine system about changes. */
    void Modify(bool needReload = false);
};

inline const FilePath& UISpineComponent::GetSkeletonPath() const
{
    return skeletonPath;
}

inline const FilePath& UISpineComponent::GetAtlasPath() const
{
    return atlasPath;
}

inline const UISpineComponent::AnimationState& UISpineComponent::GetAnimationState() const
{
    return animationState;
}

inline const String& UISpineComponent::GetAnimationName() const
{
    return animationName;
}

inline const Vector<String>& UISpineComponent::GetAnimationsNames() const
{
    return animationsNames;
}

inline const String& UISpineComponent::GetSkinName() const
{
    return skinName;
}

inline const Vector<String>& UISpineComponent::GetSkinsNames() const
{
    return skinsNames;
}

inline bool UISpineComponent::IsLoopedPlayback() const
{
    return animationLooped;
}

inline float32 UISpineComponent::GetTimeScale() const
{
    return timeScale;
}
}