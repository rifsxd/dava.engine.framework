#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>
#include <TArc/Qt/QtByteArray.h>

class LogWidget;
class LoggerOutputObject;

class LogWidgetModule : public DAVA::ClientModule
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::WindowKey& key) override;

    void OnLogOutput(DAVA::Logger::eLogLevel ll, const QByteArray& output);

    DAVA::QtConnections connections;

    LoggerOutputObject* loggerOutput = nullptr; //will be deleted by logger. Isn't it fun?
    LogWidget* logWidget = nullptr;

    DAVA_VIRTUAL_REFLECTION(LogWidgetModule, DAVA::ClientModule);
};
