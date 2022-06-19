#include "LazyUpdater.h"

#include <QTimer>

LazyUpdater::LazyUpdater(Updater updater_, QObject* parent /* = nullptr */)
    : QObject(parent)
    , updater(updater_)
{
}

void LazyUpdater::Update(int msecDelay)
{
    ++counter;
    QTimer::singleShot(msecDelay, this, &LazyUpdater::OnTimer);
}

void LazyUpdater::OnTimer()
{
    if (counter > 1)
    {
        --counter;
        return;
    }

    counter = 0;

    updater();
}
