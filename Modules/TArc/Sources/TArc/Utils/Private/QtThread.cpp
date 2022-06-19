#include "TArc/Utils/QtThread.h"

namespace DAVA
{
QtThread::QtThread(QObject* parent /*= nullptr*/)
    : QThread(parent)
{
}

#if defined(__DAVAENGINE_WINDOWS__)
void QtThread::run()
{
    QThread::exec();
}
#endif
} // namespace DAVA
