#include <UI/Spine/UISpineAttachControlsToBonesComponent.h>
#include <UI/Spine/UISpineComponent.h>
#include <UI/Spine/UISpineSingleComponent.h>
#include <UI/Spine/UISpineSystem.h>

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <Base/RefPtrUtils.h>
#include <UI/DefaultUIPackageBuilder.h>
#include <UI/UIControl.h>
#include <UI/UIControlSystem.h>
#include <UI/UIPackageLoader.h>
#include <UI/UIScreen.h>
#include <UnitTests/UnitTests.h>

using namespace DAVA;

namespace UISpineTestConsts
{
static const String ATTACHED_CONTROL_NAME = "AttachedControl";
static const String ATTACHED_BONE_NAME = "root";
static const Vector2 ATTACHED_CONTROL_START_POSITION = Vector2(-10.f, -10.f);
static const Vector2 ATTACHED_CONTROL_START_SCALE = Vector2(.1f, .1f);
static const float32 ATTACHED_CONTROL_START_ANGLE = -90.f;
static const Vector<UISpineAttachControlsToBonesComponent::AttachInfo> ATTACHED_BINDS{ { ATTACHED_BONE_NAME, ATTACHED_CONTROL_NAME } };
static const Vector<UISpineAttachControlsToBonesComponent::AttachInfo> EMPTY_BINDS;
}

DAVA_TESTCLASS (UISpineTest)
{
    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(DavaFramework)
    DECLARE_COVERED_FILES("SpineModule.cpp")
    DECLARE_COVERED_FILES("UISpineSystem.cpp")
    DECLARE_COVERED_FILES("UISpineComponent.cpp")
    DECLARE_COVERED_FILES("UISpineAttachControlsToBonesComponent.cpp")
    DECLARE_COVERED_FILES("SpineSkeleton.cpp")
    END_FILES_COVERED_BY_TESTS();

    RefPtr<UIControl> spineControl;
    RefPtr<UIControl> attachedControl;
    RefPtr<UISpineComponent> spineComponent;
    RefPtr<UISpineAttachControlsToBonesComponent> spineAttachComponent;
    UISpineSystem* spineSystem = nullptr;
    UISpineSingleComponent* spineSingleComponent = nullptr;

    void SkipTime(float32 timeDelta)
    {
        spineSystem->Process(timeDelta);
        spineSingleComponent->ResetState();
    }

    void SkipFrames(int32 frameDelta)
    {
        static const float32 SPINE_FRAMES_PER_SECOND = 30.f;
        SkipTime(frameDelta / SPINE_FRAMES_PER_SECOND);
    }

    bool CompareVectors(const Vector2& a, const Vector2& b, float32 epsilon = DAVA::EPSILON)
    {
        return FLOAT_EQUAL_EPS(a.x, b.x, epsilon) && FLOAT_EQUAL_EPS(a.y, b.y, epsilon);
    }

    UISpineTest()
    {
        RefPtr<UIScreen> screen(new UIScreen());
        GetEngineContext()->uiControlSystem->SetScreen(screen.Get());
        GetEngineContext()->uiControlSystem->Update();

        spineControl = MakeRef<UIControl>(Rect(0, 0, 10, 10));
        screen->AddControl(spineControl.Get());

        attachedControl = MakeRef<UIControl>(Rect(UISpineTestConsts::ATTACHED_CONTROL_START_POSITION, Vector2(10, 10)));
        attachedControl->SetName(UISpineTestConsts::ATTACHED_CONTROL_NAME);
        spineControl->AddControl(attachedControl.Get());

        spineSystem = spineControl->GetScene()->GetSystem<UISpineSystem>();
        TEST_VERIFY(spineSystem);
        spineSingleComponent = spineControl->GetScene()->GetSingleComponent<UISpineSingleComponent>();
        TEST_VERIFY(spineSingleComponent);
    }

    ~UISpineTest()
    {
        GetEngineContext()->uiControlSystem->Reset();
    }

    void SetUp(const String& testName) override
    {
        // Drop control's properties
        attachedControl->SetPosition(UISpineTestConsts::ATTACHED_CONTROL_START_POSITION);
        attachedControl->SetScale(UISpineTestConsts::ATTACHED_CONTROL_START_SCALE);
        attachedControl->SetAngle(UISpineTestConsts::ATTACHED_CONTROL_START_ANGLE);
        TEST_VERIFY_WITH_MESSAGE(CompareVectors(attachedControl->GetPosition(), UISpineTestConsts::ATTACHED_CONTROL_START_POSITION, 0.1f), testName);
        TEST_VERIFY_WITH_MESSAGE(CompareVectors(attachedControl->GetScale(), UISpineTestConsts::ATTACHED_CONTROL_START_SCALE, 0.1f), testName);
        TEST_VERIFY_WITH_MESSAGE(FLOAT_EQUAL_EPS(attachedControl->GetAngle(), UISpineTestConsts::ATTACHED_CONTROL_START_ANGLE, 0.1f), testName);

        // Setup component
        spineComponent = spineControl->GetOrCreateComponent<UISpineComponent>();
        spineComponent->SetSkeletonPath("~res:/UI/Spine/SpineTest.json");
        spineComponent->SetAtlasPath("~res:/UI/Spine/SpineTest.atlas");

        spineAttachComponent = spineControl->GetOrCreateComponent<UISpineAttachControlsToBonesComponent>();
        spineAttachComponent->SetBinds(UISpineTestConsts::ATTACHED_BINDS);

        // Load skeleton and attach control in system
        SkipTime(0.f);

        // Check component's properies after load
        TEST_VERIFY_WITH_MESSAGE(spineAttachComponent->GetBinds() == UISpineTestConsts::ATTACHED_BINDS, testName);
        TEST_VERIFY_WITH_MESSAGE(spineComponent->GetAnimationsNames().size() == 3, testName);
        TEST_VERIFY_WITH_MESSAGE(spineComponent->GetSkinsNames().size() == 3, testName);
        // Check control's properies after load
        TEST_VERIFY_WITH_MESSAGE(CompareVectors(attachedControl->GetPosition(), Vector2(), 0.1f), testName);
        TEST_VERIFY_WITH_MESSAGE(CompareVectors(attachedControl->GetScale(), Vector2(1.f, 1.f), 0.1f), testName);
        TEST_VERIFY_WITH_MESSAGE(FLOAT_EQUAL_EPS(attachedControl->GetAngle(), 0.f, 0.1f), testName);
    }

    void TearDown(const String& testName) override
    {
        spineComponent = spineControl->GetOrCreateComponent<UISpineComponent>();
        spineComponent->SetSkeletonPath("");
        spineComponent->SetAtlasPath("");
        spineComponent->SetAnimationState(UISpineComponent::STOPPED);
        spineComponent->SetLoopedPlayback(false);
        spineComponent->SetTimeScale(1.f);

        spineAttachComponent = spineControl->GetOrCreateComponent<UISpineAttachControlsToBonesComponent>();
        spineAttachComponent->SetBinds(UISpineTestConsts::EMPTY_BINDS);

        // Reload skeleton and reattach control in system
        SkipTime(0.f);

        // Check component's properies after unload
        TEST_VERIFY_WITH_MESSAGE(spineAttachComponent->GetBinds() == UISpineTestConsts::EMPTY_BINDS, testName);
        TEST_VERIFY_WITH_MESSAGE(spineComponent->GetAnimationsNames().size() == 0, testName);
        TEST_VERIFY_WITH_MESSAGE(spineComponent->GetSkinsNames().size() == 0, testName);
    }

    DAVA_TEST (PositionAnimationTest)
    {
        spineComponent->SetSkinName("gold");
        spineComponent->SetAnimationState(UISpineComponent::PLAYED);

        spineComponent->SetAnimationName("position");
        TEST_VERIFY(CompareVectors(attachedControl->GetPosition(), Vector2(), 0.1f));
        SkipFrames(20);
        TEST_VERIFY(CompareVectors(attachedControl->GetPosition(), Vector2(100.f, 0.f), 0.1f));
        SkipFrames(20);
        TEST_VERIFY(CompareVectors(attachedControl->GetPosition(), Vector2(100.f, -100.f), 0.1f));
        SkipFrames(20);
        TEST_VERIFY(CompareVectors(attachedControl->GetPosition(), Vector2(), 0.1f));
    }

    DAVA_TEST (RotationAnimationTest)
    {
        spineComponent->SetSkinName("gold");
        spineComponent->SetAnimationState(UISpineComponent::PLAYED);

        spineComponent->SetAnimationName("rotate");
        TEST_VERIFY(FLOAT_EQUAL_EPS(attachedControl->GetAngleInDegrees(), 0.f, 0.1f));
        SkipFrames(20);
        TEST_VERIFY(FLOAT_EQUAL_EPS(attachedControl->GetAngleInDegrees(), -90.f, 0.1f));
        SkipFrames(20);
        TEST_VERIFY(FLOAT_EQUAL_EPS(attachedControl->GetAngleInDegrees(), -180.f, 0.1f));
        SkipFrames(20);
        TEST_VERIFY(FLOAT_EQUAL_EPS(attachedControl->GetAngleInDegrees(), 0.f, 0.1f));
    }

    DAVA_TEST (ScaleAnimationTest)
    {
        spineComponent->SetSkinName("gold");
        spineComponent->SetAnimationState(UISpineComponent::PLAYED);

        spineComponent->SetAnimationName("scale");
        TEST_VERIFY(CompareVectors(attachedControl->GetScale(), Vector2(1.f, 1.f), 0.1f));
        SkipFrames(20);
        TEST_VERIFY(CompareVectors(attachedControl->GetScale(), Vector2(2.f, 2.f), 0.1f));
        SkipFrames(20);
        TEST_VERIFY(CompareVectors(attachedControl->GetScale(), Vector2(0.5f, 0.5f), 0.1f));
        SkipFrames(20);
        TEST_VERIFY(CompareVectors(attachedControl->GetScale(), Vector2(1.f, 1.f), 0.1f));
    }

    DAVA_TEST (SkinTest)
    {
        spineComponent->SetAnimationName("position");
        spineComponent->SetAnimationState(UISpineComponent::PLAYED);

        // Default test
        spineComponent->SetSkinName("default");
        SkipFrames(1);
        TEST_VERIFY(spineComponent->GetSkinName() == "default");

        // First skin name
        spineComponent->SetSkinName("gold");
        SkipFrames(1);
        TEST_VERIFY(spineComponent->GetSkinName() == "gold");

        // Second skin name
        spineComponent->SetSkinName("silver");
        SkipFrames(1);
        TEST_VERIFY(spineComponent->GetSkinName() == "silver");

        // Invalid name
        spineComponent->SetSkinName("invalid");
        SkipFrames(1);
        TEST_VERIFY(spineComponent->GetSkinName() == "silver");
    }

    DAVA_TEST (AnimationStateTest)
    {
        spineComponent->SetAnimationName("position");
        spineComponent->SetSkinName("gold");

        spineComponent->SetAnimationState(UISpineComponent::PLAYED);
        SkipFrames(20);
        TEST_VERIFY(CompareVectors(attachedControl->GetPosition(), Vector2(100.f, 0.f), 0.1f));

        spineComponent->SetAnimationState(UISpineComponent::STOPPED);
        SkipFrames(1);
        TEST_VERIFY(CompareVectors(attachedControl->GetPosition(), Vector2(), 0.1f));
    }

    DAVA_TEST (AnimationSpeedTest)
    {
        spineComponent->SetAnimationName("position");
        spineComponent->SetSkinName("gold");
        spineComponent->SetAnimationState(UISpineComponent::PLAYED);

        spineComponent->SetTimeScale(2.f);
        SkipFrames(10);
        TEST_VERIFY(CompareVectors(attachedControl->GetPosition(), Vector2(100.f, 0.f), 0.1f));

        spineComponent->SetTimeScale(0.5f);
        SkipFrames(40);
        TEST_VERIFY(CompareVectors(attachedControl->GetPosition(), Vector2(100.f, 0.f), 0.1f));
    }

    DAVA_TEST (LoopedPlaybackTest)
    {
        spineComponent->SetAnimationName("position");
        spineComponent->SetSkinName("gold");

        spineComponent->SetAnimationState(UISpineComponent::PLAYED);
        spineComponent->SetLoopedPlayback(true);
        SkipFrames(100);
        TEST_VERIFY(spineComponent->GetAnimationState() == UISpineComponent::PLAYED);

        spineComponent->SetAnimationState(UISpineComponent::PLAYED);
        spineComponent->SetLoopedPlayback(false);
        SkipFrames(100); // Spine fire COMPLETE event at non-loop animation end
        SkipFrames(0); // Spine fire FINISH event only at next update after animation complete
        TEST_VERIFY(spineComponent->GetAnimationState() == UISpineComponent::STOPPED);
    }
};
