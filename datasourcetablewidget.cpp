#include "datasourcetablewidget.h"

#include <QtGui/QAction>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QStyledItemDelegate>

#include <cmath>

class DataSourceItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        /* let the real item delegate do the work: */
        QWidget *editor = QStyledItemDelegate::createEditor(parent, option, index);

        /* if this is an QLineEdit (which it should be) than
           we restrict its input to allow only doubles from -9.99 to 9.99 */
        QLineEdit *le = qobject_cast<QLineEdit *>(editor);
        if (le != nullptr) {
            QDoubleValidator *val = new QDoubleValidator(-9.99, // min value
                                                          9.99, // max value
                                                         2,     // decimals
                                                         editor); // editor is parent so it will get deleted
            le->setValidator(val);
        }

        emit editorCreated(editor, index); // emit signal that an editor has been created

        return editor;
    }

signals:
    void editorCreated(QWidget *editor, const QModelIndex &index) const;
};

DataSourceTableWidgetItem::DataSourceTableWidgetItem()
    : QTableWidgetItem(QTableWidgetItem::UserType),
      m_data(NAN)
{
}

DataSourceTableWidgetItem *DataSourceTableWidgetItem::clone() const
{
    return new DataSourceTableWidgetItem(*this);
}

QVariant DataSourceTableWidgetItem::data(int role) const
{
    if ((role == DataRole) || (role == UserDataRole))
        return QVariant::fromValue(m_data);

    return QTableWidgetItem::data(role);
}

void DataSourceTableWidgetItem::setData(int role, const QVariant &value)
{
    if ((role < Qt::UserRole) && (role != Qt::EditRole))
        return QTableWidgetItem::setData(role, value);

    /* QTableWidget and it's editors will call this function with Qt::EditRole: */
    if (role == Qt::EditRole) {
        /* lets try to convert string to number: */
        bool ok;
        double d = value.toString().toDouble(&ok);
        if (!ok || (d == NAN)) // not okay?
            return;            // leave m_data as it was and exit

        role = UserDataRole; // mark item as edited (see below)
        m_data = d;
    } else { // MainWindow called this function with DataRole/UserDataRole
        bool ok;
        double d = value.toDouble(&ok);
        Q_ASSERT(ok); // assert if it failed, should never as values come from Eigen::Matrix

        if (!ok)
            return;

        m_data = d; // save for easy access via data(DataRole)
    }

    /* set displayable string: (width is always 5, precision is always 2) */
    QTableWidgetItem::setData(Qt::DisplayRole,
                              QString("%L1").arg(m_data, 5, 'f', 2));

    /* set background color: (green if user has edited) */
    QTableWidgetItem::setData(Qt::BackgroundRole,
                              (role == UserDataRole) ? Qt::green : QVariant());

    /* set font color: (lightgray if d is equal to zero) */
    QTableWidgetItem::setData(Qt::ForegroundRole,
                              (m_data == 0.) ? Qt::lightGray : QVariant());
}

DataSourceTableWidget::DataSourceTableWidget(QWidget *parent)
    : QTableWidget(parent),
      m_editor(nullptr)
{
    /* prepare actions for right-click context menu: */
    m_setZeroAction = new QAction(this);
    m_setZeroAction->setText(tr("Set to %L1").arg(0., 0, 'f', 2));
    m_setZeroAction->setData(QVariant::fromValue(0.));

    m_setOneAction = new QAction(this);
    m_setOneAction->setText(tr("Set to %L1").arg(1., 0, 'f', 2));
    m_setOneAction->setData(QVariant::fromValue(1.));

    connect(m_setOneAction, SIGNAL(triggered()),
            this, SLOT(setSimpleActionsValue()));
    connect(m_setZeroAction, SIGNAL(triggered()),
            this, SLOT(setSimpleActionsValue()));

    m_selectAllAction = new QAction(this);
    m_selectAllAction->setText(tr("Select all"));

    m_selectNoneAction = new QAction(this);
    m_selectNoneAction->setText(tr("Select none"));

    connect(m_selectAllAction, SIGNAL(triggered()),
            this, SLOT(selectAll()));
    connect(m_selectNoneAction, SIGNAL(triggered()),
            this, SLOT(clearSelection()));

    QAction *sep = new QAction(this);
    sep->setSeparator(true);

    /* add actions to this to show up in right-click context menu: */
    addAction(m_setZeroAction);
    addAction(m_setOneAction);
    addAction(sep);
    addAction(m_selectAllAction);
    addAction(m_selectNoneAction);

    /* change some class properties: */
    setContextMenuPolicy(Qt::ActionsContextMenu);    // activate right-click context menu
    setItemDelegate(new DataSourceItemDelegate);     // set our item delegte class
    setItemPrototype(new DataSourceTableWidgetItem); // set our item type as prototype

    /* connect signals for multiple item editing: */
    connect(itemDelegate(), SIGNAL(editorCreated(QWidget*,QModelIndex)),
            this, SLOT(editorCreated(QWidget*,QModelIndex)));
    connect(itemDelegate(), SIGNAL(commitData(QWidget*)),
            this, SLOT(commitRequestFromEditor(QWidget*)));
    connect(itemDelegate(), SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
            this, SLOT(closeRequestFromEditor(QWidget*)));

    /* set displaying font to system's monospaced style: */
    QFont f = font();
    f.setFamily("Monospace");         // Linux (X11)
    f.setStyleHint(QFont::Monospace); // Windows & Mac
    setFont(f);

    /* determine minimal field size needed...: */
    QFontMetrics fm = fontMetrics();
    QRect rect = fm.boundingRect("-1.00");
    int wd = rect.width() + fm.width(' ') + 1; // added space is needed for neg. numbers
    int hg = rect.height() + 3;                // +3 for a little extra

    /* ... and set as fixed size: */
    horizontalHeader()->setDefaultSectionSize(wd);
    horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(hg);
    verticalHeader()->setResizeMode(QHeaderView::Fixed);
}

/**
 * Close all Editor-Widgets which have been added to m_persistentEditor beforehand.
 * @note All items in m_persistentEditor will be deleted and list will be empty
 *       afterwards.
 * @param commitDataToModel @c true if editor's content should be committed to model,
 *                          @c false if not.
 */
void DataSourceTableWidget::closeMultipleEditors(bool commitDataToModel)
{
    QList<QPointer<QWidget> >::const_iterator it = m_persistentEditor.constBegin();
    while (it != m_persistentEditor.constEnd()) {
        if (commitDataToModel)
            commitData(*it);  // commit data of persistent editor to model
        //delete *it;         // this will automatically close editor
        (*it)->deleteLater(); // !!! use deleteLater instead of delete !!!
        it++;                 //   program crashes when user clicks on one of these pers. editors
    }

    m_persistentEditor.clear(); // empty list (items will be deleted on entering event-loop)
}

/**
 * This slot is called when @a editor is about to close.
 * If editing is aborted by user, we close all openend
 * persistent editors by calling closeMultipleEditors with
 * @c false as param.
 * If editing was ended succesfully, then the other slot
 * commitRequestFromEditor is called earlier.
 * @param editor Editor-widget which is about to be closed.
 * @see commitRequestFromEditor()
 */
void DataSourceTableWidget::closeRequestFromEditor(QWidget *editor)//, QAbstractItemDelegate::EndEditHint hint)
{
    /* case: persistent editors for multi-selection of items: */
    if (editor != m_editor)
        return; // we do nothing in this case but leave

    /* case: first editor openend on multi-selection of items: */
    closeMultipleEditors(false); // close all persistent editors, but revert to old data
}

/**
 * This slot is called when editing was succesfully ended and @a editor
 * is about to commit its data to our model.
 * If @a editor is one of m_persistentEditor, we do nothing.
 * If @a editor was the original one (@see editorCreated()), we then commit
 * and close all open persistent editors from m_persistentEditor.
 * @note This slot will not be called if user aborts editing.
 *       Closing and reverting (in this case "not committing") will
 *       be handled by closeRequestFromEditor().
 * @param editor Editor-widget which is about to commit its data to model.
 * @see closeRequestFromEditor()
 */
void DataSourceTableWidget::commitRequestFromEditor(QWidget *editor)
{
    /* case: persistent editors for multi-selection of items: */
    if (editor != m_editor)
        return; // we do nothing in this case but leave

    /* case: first editor openend on multi-selection of items: */
    closeMultipleEditors(true); // close all persistent editors and save new data
}

void DataSourceTableWidget::editorCreated(QWidget *editor, const QModelIndex &index)
{
    //qDebug() << "Editor opened for index" << index;

    /* case: persistent editors for multi-selection of items: */
    if (m_editor != nullptr) {
        editor->setCursor(Qt::ForbiddenCursor);        // show user that he cannot click in here
        m_persistentEditor << editor;                  // add this one to persistent list
        connect(m_editor, SIGNAL(textEdited(QString)), // changes on "normal" editor will
                editor, SLOT(setText(QString)));       // propagate to all others
        return;
    }

    /* case: first editor openend on multi-selection of items: */
    QList<QTableWidgetItem *> items = selectedItems();
    if (items.size() > 1) { // we do this only if more than 1 item is selected

        QTableWidgetItem *current = item(index.row(), index.column()); // get item for this editor
        Q_ASSERT(current != nullptr);

        if (items.indexOf(current) == -1) // item is not in selection?
            return;                       // so we do just want to edit this one

        m_editor = editor;  // save it so that we know later which editor was the original one

        /* open "persistent editors" for all other items: */
        QList<QTableWidgetItem *>::const_iterator it = items.constBegin();
        while (it != items.constEnd()) {
            if (*it != current)
                openPersistentEditor(*it); // this will call this function again and we end up above!
            it++;
        }
    }
}

void DataSourceTableWidget::setSimpleActionsValue()
{
    QAction *act = qobject_cast<QAction *>(sender());
    Q_ASSERT(act != nullptr);

    QList<QTableWidgetItem *> items = selectedItems();
    Q_ASSERT(!items.isEmpty());

    auto it = items.begin();
    while (it != items.end()) {
        double value = act->data().toDouble();
        (*it++)->setData(DataSourceTableWidgetItem::UserDataRole, // will mark item as modified
                         value);
    }
}

#include "moc_datasourcetablewidget.cpp"
#include "datasourcetablewidget.moc" // for DataSourceItemDelegate class
