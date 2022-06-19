#pragma once

#include <FileSystem/FilePath.h>

#include <TArc/Qt/QtIcon.h>
#include <TArc/Qt/QtString.h>

namespace DAVA
{
class REFileOperation
{
public:
    enum class eOperationType
    {
        IMPORT,
        EXPORT
    };
    virtual QIcon GetIcon() const = 0;
    virtual QString GetName() const = 0;
    virtual eOperationType GetType() const = 0;
    virtual QString GetTargetFileFilter() const = 0; // In QFileDialog filters format
    virtual void Apply(const DAVA::FilePath& filePath) const = 0;
};

class REFileOperationsInterface
{
public:
    virtual void RegisterFileOperation(std::shared_ptr<REFileOperation> fileOperation) = 0;
    virtual void UnregisterFileOperation(std::shared_ptr<REFileOperation> fileOperation) = 0;
};

} // namespace DAVA