#include "TArc/Utils/QtThread.h"

#if defined(__DAVAENGINE_MACOS__)

#import "AppKit/NSView.h"
namespace DAVA
{
void QtThread::run()
{
    @autoreleasepool
    {
        QThread::exec();
    }
}
}

#endif