#include "Classes/Modules/PackageModule/Private/PackageModel.h"

#include "Classes/Modules/DocumentsModule/DocumentData.h"

#include "Classes/Modules/PackageModule/PackageMimeData.h"
#include "Classes/Model/ControlProperties/ClassProperty.h"
#include "Classes/Model/ControlProperties/CustomClassProperty.h"
#include "Classes/Model/ControlProperties/NameProperty.h"
#include "Classes/Model/ControlProperties/PrototypeNameProperty.h"
#include "Classes/Model/ControlProperties/RootProperty.h"
#include "Classes/Model/ControlProperties/VisibleValueProperty.h"
#include "Classes/Model/PackageHierarchy/ControlNode.h"
#include "Classes/Model/PackageHierarchy/ControlsContainerNode.h"
#include "Classes/Model/PackageHierarchy/ImportedPackagesNode.h"
#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/PackageNode.h"
#include "Classes/Model/PackageHierarchy/StyleSheetNode.h"
#include "Classes/Model/PackageHierarchy/StyleSheetsNode.h"
#include "Classes/Model/YamlPackageSerializer.h"
#include "Classes/QECommands/ChangePropertyValueCommand.h"
#include "Classes/UI/CommandExecutor.h"
#include "Classes/UI/IconHelper.h"
#include "Classes/Utils/ControlPlacementUtils.h"
#include "Classes/Utils/DragNDropHelper.h"
#include "Classes/Utils/QtDavaConvertion.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Qt/QtIcon.h>
#include <TArc/SharedModules/ThemesModule/ThemesModule.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Base/ObjectFactory.h>
#include <UI/UIControl.h>

#include <QAction>
#include <QUrl>

using namespace DAVA;

PackageModel::PackageModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

PackageModel::~PackageModel() = default;

void PackageModel::SetAccessor(DAVA::ContextAccessor* accessor_)
{
    accessor = accessor_;
}

void PackageModel::SetUI(DAVA::UI* ui_)
{
    ui = ui_;
}

void PackageModel::Reset(PackageNode* package_)
{
    beginResetModel();
    if (nullptr != package)
    {
        package->RemoveListener(this);
    }
    package = package_;
    if (nullptr != package)
    {
        package->AddListener(this);
    }
    endResetModel();
}

QModelIndex PackageModel::indexByNode(PackageBaseNode* node) const
{
    PackageBaseNode* parent = node->GetParent();
    if (parent == nullptr)
    {
        return QModelIndex();
    }
    return createIndex(parent->GetIndex(node), 0, node);
}

QModelIndex PackageModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }
    if (!parent.isValid())
    {
        DVASSERT(nullptr != package);
        if (nullptr != package)
        {
            return createIndex(row, column, package->Get(row));
        }
        return QModelIndex();
    }
    PackageBaseNode* node = static_cast<PackageBaseNode*>(parent.internalPointer());
    return createIndex(row, column, node->Get(row));
}

QModelIndex PackageModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
    {
        DVASSERT(false && "invalid child passed to parent function");
        return QModelIndex();
    }

    PackageBaseNode* node = static_cast<PackageBaseNode*>(child.internalPointer());
    PackageBaseNode* parent = node->GetParent();
    DVASSERT(package != nullptr);
    if (package != nullptr)
    {
        DVASSERT(nullptr != parent);
        if (nullptr == parent || parent == package.Get())
        {
            return QModelIndex();
        }
        DVASSERT(nullptr != parent->GetParent());
        return createIndex(parent->GetParent()->GetIndex(parent), 0, parent);
    }
    return QModelIndex();
}

int PackageModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        if (package != nullptr)
        {
            return package->GetCount();
        }
        return 0;
    }

    return static_cast<PackageBaseNode*>(parent.internalPointer())->GetCount();
}

int PackageModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant PackageModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        DVASSERT(false && "invalid index passed to data function");
        return QVariant();
    }
    PackageBaseNode* node = static_cast<PackageBaseNode*>(index.internalPointer());
    ControlNode* controlNode = dynamic_cast<ControlNode*>(node);

    if (controlNode)
    {
        switch (role)
        {
        case Qt::EditRole:
        case Qt::DisplayRole:
            return StringToQString(node->GetName());

        case Qt::DecorationRole:
            if (controlNode->GetRootProperty()->GetCustomClassProperty()->IsOverridden())
            {
                return QIcon(IconHelper::GetCustomIconPath());
            }
            else
            {
                const String& className = controlNode->GetRootProperty()->GetClassProperty()->GetClassName();
                return QIcon(IconHelper::GetIconPathForClassName(QString::fromStdString(className)));
            }

        case PackageCheckStateRole:
        {
            auto prop = controlNode->GetRootProperty()->GetVisibleProperty();
            return prop->GetVisibleInEditor() ? Qt::Checked : Qt::Unchecked;
        }

        case Qt::ToolTipRole:
        {
            const String& prototype = controlNode->GetRootProperty()->GetPrototypeProperty()->GetPrototypeName();
            const String& className = controlNode->GetRootProperty()->GetClassProperty()->GetClassName();
            const String& customClassName = controlNode->GetRootProperty()->GetCustomClassProperty()->GetCustomClassName();

            QString toolTip;

            if (controlNode->HasErrors())
            {
                toolTip = QString::fromStdString(controlNode->GetResults().GetResultMessages());
            }
            else
            {
                toolTip = QString("class: ") + className.c_str();
                if (!customClassName.empty())
                {
                    toolTip += QString("\ncustom class: ") + customClassName.c_str();
                }

                if (controlNode->GetPrototype())
                {
                    toolTip += QString("\nprototype: ") + prototype.c_str();
                }
            }
            return toolTip;
        }

        case Qt::TextColorRole:
        {
            DAVA::ThemesSettings* settings = accessor->GetGlobalContext()->GetData<DAVA::ThemesSettings>();
            if (controlNode->HasErrors())
            {
                return settings->GetErrorColor();
            }
            else if (controlNode->GetPrototype() != nullptr)
            {
                return settings->GetPrototypeColor();
            }
            return QVariant();
        }

        case Qt::FontRole:
        {
            QFont myFont;
            if (controlNode->GetCreationType() == ControlNode::CREATED_FROM_PROTOTYPE)
                myFont.setBold(true);

            if (node->IsReadOnly())
                myFont.setItalic(true);

            return myFont;
        }
        }
    }
    else
    {
        StyleSheetNode* styleSheet = dynamic_cast<StyleSheetNode*>(node);
        if (styleSheet)
        {
            switch (role)
            {
            case Qt::DisplayRole:
                return StringToQString(node->GetName());

            case Qt::TextColorRole:
            {
                return accessor->GetGlobalContext()->GetData<DAVA::ThemesSettings>()->GetStyleSheetNodeColor();
            }

            case Qt::FontRole:
            {
                QFont myFont;
                if (node->IsReadOnly())
                    myFont.setItalic(true);

                return myFont;
            }
            }
        }
        else
        {
            switch (role)
            {
            case Qt::DisplayRole:
                return StringToQString(node->GetName());

            case Qt::BackgroundRole:
                return accessor->GetGlobalContext()->GetData<DAVA::ThemesSettings>()->GetViewLineAlternateColor();

            case Qt::TextColorRole:
                if (node->HasErrors())
                {
                    return accessor->GetGlobalContext()->GetData<DAVA::ThemesSettings>()->GetErrorColor();
                }
                return QVariant();

            case Qt::ToolTipRole:
            {
                if (node->HasErrors())
                {
                    return QString::fromStdString(node->GetResults().GetResultMessages());
                }
                return QVariant();
            }
            case Qt::FontRole:
            {
                QFont myFont;
                myFont.setBold(true);

                if (node->IsReadOnly())
                    myFont.setItalic(true);

                return myFont;
            }
            }
        }
    }

    return QVariant();
}

bool PackageModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
    {
        DVASSERT(false && "invalid index passed to setData");
        return false;
    }
    PackageBaseNode* node = static_cast<PackageBaseNode*>(index.internalPointer());
    auto control = node->GetControl();
    if (nullptr == control)
    {
        return false;
    }
    ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
    DVASSERT(controlNode);

    if (role == PackageCheckStateRole)
    {
        auto prop = controlNode->GetRootProperty()->GetVisibleProperty();
        prop->SetVisibleInEditor(value.toBool());
        package->RefreshProperty(controlNode, prop);
        return true;
    }
    if (role == Qt::EditRole)
    {
        auto prop = controlNode->GetRootProperty()->GetNameProperty();
        const auto& newName = value.toString().toStdString();
        if (newName != node->GetName())
        {
            DAVA::DataContext* activeContext = accessor->GetActiveContext();
            DVASSERT(activeContext != nullptr);
            DocumentData* documentData = activeContext->GetData<DocumentData>();
            documentData->ExecCommand<ChangePropertyValueCommand>(controlNode, prop, Any(newName));
            return true;
        }
    }
    return false;
}

Qt::ItemFlags PackageModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;

    const PackageBaseNode* node = static_cast<PackageBaseNode*>(index.internalPointer());
    if (node->CanCopy())
    {
        flags |= Qt::ItemIsDragEnabled;
    }
    if (node->IsInsertingControlsSupported() || node->IsInsertingPackagesSupported() || node->IsInsertingStylesSupported())
    {
        flags |= Qt::ItemIsDropEnabled;
    }
    if (node->IsEditingSupported())
    {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

Qt::DropActions PackageModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

QStringList PackageModel::mimeTypes() const
{
    QStringList types;
    types << PackageMimeData::MIME_TYPE;
    types << "text/plain";
    types << "text/uri-list";
    return types;
}

QMimeData* PackageModel::mimeData(const QModelIndexList& indices) const
{
    DVASSERT(nullptr != package);
    if (nullptr == package)
    {
        return nullptr;
    }

    PackageMimeData* mimeData = new PackageMimeData();

    SortedPackageBaseNodeSet controlNodesForCopy(CompareByLCA);
    SortedPackageBaseNodeSet styleSheetNodesForCopy(CompareByLCA);

    for (const QModelIndex& index : indices)
    {
        if (index.isValid())
        {
            PackageBaseNode* node = static_cast<PackageBaseNode*>(index.internalPointer());
            if (node->CanCopy())
            {
                if (dynamic_cast<ControlNode*>(node) != nullptr)
                {
                    controlNodesForCopy.insert(node);
                }
                else if (dynamic_cast<StyleSheetNode*>(node) != nullptr)
                {
                    styleSheetNodesForCopy.insert(node);
                }
            }
        }
    }

    for (PackageBaseNode* controlNode : controlNodesForCopy)
    {
        PackageBaseNode* parent = controlNode->GetParent();
        while (parent != nullptr && controlNodesForCopy.find(parent) == controlNodesForCopy.end())
        {
            parent = parent->GetParent();
        }
        if (parent == nullptr)
        {
            mimeData->AddControl(static_cast<ControlNode*>(controlNode));
        }
    }

    for (PackageBaseNode* styleSheetNode : styleSheetNodesForCopy)
    {
        mimeData->AddStyle(static_cast<StyleSheetNode*>(styleSheetNode));
    }

    YamlPackageSerializer serializer;
    serializer.SerializePackageNodes(package.Get(), mimeData->GetControls(), mimeData->GetStyles());
    String str = serializer.WriteToString();
    mimeData->setText(QString::fromStdString(str));

    return mimeData;
}

int PackageModel::GetRowIndex(int row, const QModelIndex& parent) const
{
    if (row != -1)
    {
        return row;
    }
    return rowCount(parent);
}

bool PackageModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int /*column*/, const QModelIndex& parent)
{
    int rowIndex = GetRowIndex(row, parent);
    DVASSERT(rowIndex >= 0);
    PackageBaseNode* destNode = static_cast<PackageBaseNode*>(parent.internalPointer());
    OnDropMimeData(data, action, destNode, rowIndex, nullptr);
    return true; //if we can drop - we must drop. Otherwise CanDropMimeData must return false;
}

void PackageModel::OnDropMimeData(const QMimeData* data, Qt::DropAction action, PackageBaseNode* destNode, uint32 destIndex, const DAVA::Vector2* pos)
{
    DVASSERT(nullptr != package);

    ControlsContainerNode* destControlContainer = dynamic_cast<ControlsContainerNode*>(destNode);
    StyleSheetsNode* destStylesContainer = dynamic_cast<StyleSheetsNode*>(destNode);

    CommandExecutor executor(accessor, ui);

    if (destControlContainer && data->hasFormat(PackageMimeData::MIME_TYPE))
    {
        const PackageMimeData* controlMimeData = DynamicTypeCheck<const PackageMimeData*>(data);

        const Vector<ControlNode*>& srcControls = controlMimeData->GetControls();
        if (!srcControls.empty())
        {
            Vector<ControlNode*> nodes;
            emit BeforeProcessNodes(SelectedNodes(srcControls.begin(), srcControls.end()));
            switch (action)
            {
            case Qt::CopyAction:
                nodes = executor.CopyControls(srcControls, destControlContainer, destIndex);
                break;
            case Qt::MoveAction:
                nodes = executor.MoveControls(srcControls, destControlContainer, destIndex);
                break;
            default:
                // just do nothing
                break;
            }
            if (pos != nullptr && destNode != package->GetPackageControlsNode())
            {
                auto destControl = dynamic_cast<ControlNode*>(destNode);
                if (destControl != nullptr)
                {
                    for (const auto& node : nodes)
                    {
                        ControlPlacementUtils::SetAbsoulutePosToControlNode(package.Get(), node, destControl, *pos);
                    }
                }
            }
            emit AfterProcessNodes(SelectedNodes(nodes.begin(), nodes.end()));
        }
    }
    else if (destStylesContainer && data->hasFormat(PackageMimeData::MIME_TYPE))
    {
        const PackageMimeData* mimeData = DynamicTypeCheck<const PackageMimeData*>(data);

        const Vector<StyleSheetNode*>& srcStyles = mimeData->GetStyles();
        if (!srcStyles.empty())
        {
            switch (action)
            {
            case Qt::CopyAction:
                executor.CopyStyles(srcStyles, destStylesContainer, destIndex);
                break;
            case Qt::MoveAction:
                executor.MoveStyles(srcStyles, destStylesContainer, destIndex);
                break;
            default:
                // just do nothing
                break;
            }
        }
    }
    else if (data->hasFormat("text/uri-list") && data->hasText())
    {
        QStringList list = data->text().split("\n");
        Vector<FilePath> packages;
        ResultList wrongExtensionResults;
        ResultList wrongSourceResults;

        for (const QString& str : list)
        {
            QUrl url(str);
            if (url.isLocalFile())
            {
                QString path = url.toLocalFile();

                if (DragNDropHelper::IsExtensionSupported(path) == false)
                {
                    wrongExtensionResults.AddResult(Result::RESULT_WARNING, Format("%s", path.toStdString().c_str()));
                }
                else if (DragNDropHelper::IsFileFromProject(accessor, path) == false)
                {
                    wrongSourceResults.AddResult(Result::RESULT_WARNING, Format("%s", path.toStdString().c_str()));
                }
                else
                {
                    packages.push_back(FilePath(path.toStdString()));
                }
            }
        }

        if (wrongExtensionResults.HasWarnings())
        {
            DAVA::NotificationParams notificationParams;
            notificationParams.title = "can not drop";
            notificationParams.message = Result(Result::RESULT_WARNING, Format("next files have unsupported extension:\n%s", wrongExtensionResults.GetResultMessages().c_str()));
            ui->ShowNotification(DAVA::mainWindowKey, notificationParams);
        }

        if (wrongSourceResults.HasWarnings())
        {
            DAVA::NotificationParams notificationParams;
            notificationParams.title = "can not drop";
            notificationParams.message = Result(Result::RESULT_WARNING, Format("next files are not from project:\n%s", wrongSourceResults.GetResultMessages().c_str()));
            ui->ShowNotification(DAVA::mainWindowKey, notificationParams);
        }

        if (!packages.empty())
        {
            executor.AddImportedPackagesIntoPackage(packages, package.Get());
        }
    }
    else if (destNode && data->hasFormat("text/plain") && data->hasText())
    {
        String string = data->text().toStdString();
        auto nodes = executor.Paste(package.Get(), destNode, destIndex, string);
        if (pos != nullptr && destNode != package->GetPackageControlsNode())
        {
            auto destControl = dynamic_cast<ControlNode*>(destNode);
            if (destControl != nullptr)
            {
                for (const auto& node : nodes)
                {
                    auto control = dynamic_cast<ControlNode*>(node);
                    if (control != nullptr)
                    {
                        ControlPlacementUtils::SetAbsoulutePosToControlNode(package.Get(), control, destControl, *pos);
                    }
                }
            }
        }
        emit AfterProcessNodes(SelectedNodes(nodes.begin(), nodes.end()));
    }
}

void PackageModel::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    if (property->GetName() == "Name")
    {
        QModelIndex index = indexByNode(node);
        emit dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
    }

    else if (property->GetName() == "Custom Class")
    {
        QModelIndex index = indexByNode(node);
        emit dataChanged(index, index, QVector<int>() << Qt::DecorationRole);
    }
}

void PackageModel::StylePropertyWasChanged(StyleSheetNode* node, AbstractProperty* property)
{
    const auto& name = property->GetName();
    if (name == "Name" || name == "Selector")
    {
        QModelIndex index = indexByNode(node);
        emit dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
    }
}

void PackageModel::ControlWillBeAdded(ControlNode* /*node*/, ControlsContainerNode* destination, int row)
{
    QModelIndex destIndex = indexByNode(destination);
    beginInsertRows(destIndex, row, row);
}

void PackageModel::ControlWasAdded(ControlNode* /*node*/, ControlsContainerNode* /*destination*/, int /*row*/)
{
    endInsertRows();
}

void PackageModel::ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::ControlWasRemoved(ControlNode* /*node*/, ControlsContainerNode* /*from*/)
{
    endRemoveRows();
}

void PackageModel::StyleWillBeAdded(StyleSheetNode* /*node*/, StyleSheetsNode* destination, int index)
{
    QModelIndex destIndex = indexByNode(destination);
    beginInsertRows(destIndex, index, index);
}

void PackageModel::StyleWasAdded(StyleSheetNode* /*node*/, StyleSheetsNode* /*destination*/, int /*index*/)
{
    endInsertRows();
}

void PackageModel::StyleWillBeRemoved(StyleSheetNode* node, StyleSheetsNode* from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::StyleWasRemoved(StyleSheetNode* /*node*/, StyleSheetsNode* /*from*/)
{
    endRemoveRows();
}

void PackageModel::ImportedPackageWillBeAdded(PackageNode* /*node*/, ImportedPackagesNode* to, int index)
{
    QModelIndex destIndex = indexByNode(to);
    beginInsertRows(destIndex, index, index);
}

void PackageModel::ImportedPackageWasAdded(PackageNode* /*node*/, ImportedPackagesNode* /*to*/, int /*index*/)
{
    endInsertRows();
}

void PackageModel::ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from)
{
    QModelIndex parentIndex = indexByNode(from);
    int index = from->GetIndex(node);
    beginRemoveRows(parentIndex, index, index);
}

void PackageModel::ImportedPackageWasRemoved(PackageNode* /*node*/, ImportedPackagesNode* /*from*/)
{
    endRemoveRows();
}
