#pragma once

#include <QThread>

namespace DAVA
{
// MemoryLeak-Safe Qt thread that call base ::exec method, but creates Autorelease Pool on Mac
class QtThread : public QThread
{
public:
    QtThread(QObject* parent = nullptr);

protected:
    void run() override;
};
} // namespace DAVA
