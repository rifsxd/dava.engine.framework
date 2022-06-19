#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Controls/PopupLineEdit.h"

#include <QtTest>

namespace PopupLineEditTestDetail
{
using namespace DAVA;

DAVA::WindowKey wndKey("LineEditTestWnd");

struct PopupLineEditDataSource
{
    String text = "";

    DAVA_REFLECTION(PopupLineEditDataSource)
    {
        ReflectionRegistrator<PopupLineEditDataSource>::Begin()
        .Field("text", &PopupLineEditDataSource::text)
        .End();
    }
};

} // namespace PopupLineEditTestDetila

DAVA_TARC_TESTCLASS(PopupLineEditTest)
{
    PopupLineEditTestDetail::PopupLineEditDataSource dataSource;

    DAVA_TEST (ShowHideTest)
    {
        using namespace DAVA;
        using namespace DAVA;
        Reflection model = Reflection::Create(&dataSource);

        LineEdit::Params p(GetAccessor(), GetUI(), PopupLineEditTestDetail::wndKey);
        p.fields[LineEdit::Fields::Text] = "text";
        PopupLineEdit* edit = new PopupLineEdit(p, GetAccessor(), model);

        QObject::connect(edit, &QObject::destroyed, [&]()
                         {
                             ControlDestroyed();
                         });

        edit->Show(QPoint(100, 100));
        TEST_VERIFY(edit->isVisible() == true);

        QTestEventList eventList;
        eventList.addKeyClicks("new text");
        eventList.addKeyClick(Qt::Key_Escape);
        eventList.simulate(nullptr);

        TEST_VERIFY(edit->isVisible() == false);
        TEST_VERIFY(dataSource.text.empty() == true);
        EXPECT_CALL(*this, ControlDestroyed())
        .WillOnce(::testing::Return());
    }

    DAVA_TEST (EditTestTest)
    {
        using namespace DAVA;
        using namespace DAVA;
        Reflection model = Reflection::Create(&dataSource);

        LineEdit::Params p(GetAccessor(), GetUI(), PopupLineEditTestDetail::wndKey);
        p.fields[LineEdit::Fields::Text] = "text";
        PopupLineEdit* edit = new PopupLineEdit(p, GetAccessor(), model);

        QObject::connect(edit, &QObject::destroyed, [&]()
                         {
                             ControlDestroyed();
                         });

        edit->Show(QPoint(100, 100));
        TEST_VERIFY(edit->isVisible() == true);

        QTestEventList eventList;
        eventList.addKeyClicks("new text");
        eventList.addKeyClick(Qt::Key_Return);
        eventList.simulate(nullptr);

        TEST_VERIFY(edit->isVisible() == false);
        TEST_VERIFY(dataSource.text == DAVA::String("new text"));
        EXPECT_CALL(*this, ControlDestroyed())
        .WillOnce(::testing::Return());
    }

    MOCK_METHOD0_VIRTUAL(ControlDestroyed, void());
};
