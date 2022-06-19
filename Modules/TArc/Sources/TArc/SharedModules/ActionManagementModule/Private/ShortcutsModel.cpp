#include "TArc/SharedModules/ActionManagementModule/Private/ShortcutsModel.h"

namespace DAVA
{
namespace ShortcursModelDetails
{
bool IsContextCoflicted(Qt::ShortcutContext left, Qt::ShortcutContext right)
{
    return left == Qt::WindowShortcut || left == Qt::ApplicationShortcut || right == Qt::WindowShortcut || right == Qt::ApplicationShortcut;
}
} // namespace ShortcursModelDetails

void ShortcutsModel::SetData(const Vector<KeyBindableAction>& actionsData)
{
    using namespace ShortcursModelDetails;
    beginResetModel();
    blocks.clear();
    actions.clear();

    Set<QString> sortedBlocks;
    for (size_t i = 0; i < actionsData.size(); ++i)
    {
        const KeyBindableAction& action = actionsData[i];
        if (action.action == nullptr)
        {
            continue;
        }

        sortedBlocks.insert(action.blockName);
        QString conflitsWith;
        foreach (const QKeySequence& seq1, action.sequences)
        {
            for (size_t j = 0; j < actionsData.size(); ++j)
            {
                if (i == j)
                {
                    continue;
                }

                const KeyBindableAction& conflicted = actionsData[j];
                if (conflicted.action == nullptr)
                {
                    continue;
                }

                foreach (const QKeySequence& seq2, conflicted.sequences)
                {
                    if (seq1.matches(seq2) != QKeySequence::NoMatch && IsContextCoflicted(conflicted.context, action.context))
                    {
                        QString conflictedAction = QString("%1.%2").arg(conflicted.blockName).arg(conflicted.actionName);
                        if (conflitsWith.isEmpty())
                        {
                            conflitsWith = conflictedAction;
                        }
                        else
                        {
                            conflitsWith = QString("%1;%2").arg(conflitsWith).arg(conflictedAction);
                        }
                        break;
                    }
                }
            }
        }

        Node node;
        node.action = action;
        node.conflictsWith = conflitsWith;
        actions[action.blockName].push_back(node);
    }

    blocks.assign(sortedBlocks.begin(), sortedBlocks.end());
    for (auto& action : actions)
    {
        std::sort(action.second.begin(), action.second.end(), [](const Node& n1, const Node& n2) {
            return n1.action.actionName < n2.action.actionName;
        });
    }
    endResetModel();
}

const KeyBindableAction* ShortcutsModel::GetKeyBindableAction(const QModelIndex& index)
{
    quintptr id = index.internalId();
    if (id == static_cast<quintptr>(-1))
    {
        return nullptr;
    }

    QString blockName = blocks[id];
    auto iter = actions.find(blockName);
    DVASSERT(iter != actions.end());
    return &(iter->second[index.row()].action);
}

QModelIndex ShortcutsModel::GetIndex(const QString& blockName) const
{
    if (blockName.isEmpty())
    {
        return QModelIndex();
    }

    auto blockNameIter = std::find(blocks.begin(), blocks.end(), blockName);
    DVASSERT(blockNameIter != blocks.end());
    int row = std::distance(blocks.begin(), blockNameIter);
    return createIndex(row, 0, static_cast<quintptr>(-1));
}

QModelIndex ShortcutsModel::GetIndex(const QString& blockName, QPointer<QAction> action) const
{
    if (blockName.isEmpty())
    {
        return QModelIndex();
    }

    auto blockNameIter = std::find(blocks.begin(), blocks.end(), blockName);
    DVASSERT(blockNameIter != blocks.end());
    quintptr id = std::distance(blocks.begin(), blockNameIter);

    auto iter = actions.find(blockName);
    DVASSERT(iter != actions.end());
    auto actionIter = std::find_if(iter->second.begin(), iter->second.end(), [action](const Node& n) {
        return n.action.action == action;
    });
    DVASSERT(actionIter != iter->second.end());

    int row = static_cast<int>(std::distance(iter->second.begin(), actionIter));
    return createIndex(row, 0, id);
}

int ShortcutsModel::columnCount(const QModelIndex& parent) const
{
    return 4;
}

int ShortcutsModel::rowCount(const QModelIndex& parent) const
{
    if (parent == QModelIndex())
    {
        return static_cast<int>(blocks.size());
    }

    quintptr id = parent.internalId();
    if (id == static_cast<quintptr>(-1))
    {
        QString blockName = blocks[parent.row()];
        auto iter = actions.find(blockName);
        DVASSERT(iter != actions.end());
        return static_cast<int>(iter->second.size());
    }

    return 0;
}

QVariant ShortcutsModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::ForegroundRole && index.column() == 3)
    {
        return QColor(Qt::red);
    }

    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    quintptr id = index.internalId();
    if (id == static_cast<quintptr>(-1))
    {
        if (index.column() == 0)
        {
            return blocks[index.row()];
        }
        else
        {
            return QVariant();
        }
    }
    else
    {
        QString blockName = blocks[id];
        auto iter = actions.find(blockName);
        DVASSERT(iter != actions.end());
        const Node& n = iter->second[index.row()];
        switch (index.column())
        {
        case 0:
            return n.action.actionName;
        case 1:
        {
            QString result;
            if (n.action.sequences.size() > 0)
            {
                result = n.action.sequences[0].toString(QKeySequence::NativeText);
            }

            for (int i = 1; i < n.action.sequences.size(); ++i)
            {
                result += "; ";
                result += n.action.sequences[i].toString(QKeySequence::NativeText);
            }

            return result;
        }
        case 2:
        {
            const EnumMap* contextMap = GlobalEnumMap<Qt::ShortcutContext>::Instance();
            return QString(contextMap->ToString(n.action.context));
        }
        case 3:
        {
            return n.conflictsWith;
        }
        default:
            break;
        }
    }

    return QVariant();
}

QModelIndex ShortcutsModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent == QModelIndex())
    {
        if (row >= static_cast<int>(blocks.size()))
        {
            return QModelIndex();
        }

        return createIndex(row, column, static_cast<quintptr>(-1));
    }

    int blockIndex = parent.row();
    DVASSERT(blockIndex < static_cast<int>(blocks.size()));
    QString blockName = blocks[blockIndex];
    auto iter = actions.find(blockName);
    DVASSERT(iter != actions.end());
    DVASSERT(row < static_cast<int>(iter->second.size()));
    return createIndex(row, column, static_cast<quintptr>(blockIndex));
}

QModelIndex ShortcutsModel::parent(const QModelIndex& index) const
{
    quintptr id = index.internalId();
    if (id == static_cast<quintptr>(-1))
    {
        return QModelIndex();
    }

    return createIndex(id, 0, static_cast<quintptr>(-1));
}

QVariant ShortcutsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation == Qt::Vertical)
    {
        return QVariant();
    }

    switch (section)
    {
    case 0:
        return "Command";
    case 1:
        return "Shortcut";
    case 2:
        return "Context";
    case 3:
        return "Conflicts with";
    }

    return QVariant();
}
} // namespace DAVA
