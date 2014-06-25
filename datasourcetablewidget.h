#ifndef DATASOURCETABLEWIDGET_H_
#define DATASOURCETABLEWIDGET_H_

#include <QtGui/QTableWidget>

#include <QtCore/QPointer>

class QAction;

class DataSourceTableWidgetItem : public QTableWidgetItem
{
public:
    explicit DataSourceTableWidgetItem();

    DataSourceTableWidgetItem *clone() const override;

    enum {
        DataRole = Qt::UserRole,        /**< float/double number */
        UserDataRole = Qt::UserRole + 1 /**< use this if user has entered the number */
    };

    QVariant data(int role) const override;
    void setData(int role, const QVariant &value) override;

private:
    double m_data;
};

class DataSourceTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit DataSourceTableWidget(QWidget *parent = 0);

private slots:
    void editorCreated(QWidget *editor, const QModelIndex &index);
    void commitPersistent(QWidget *editor);
    void setSimpleActionsValue();

private:
    QAction *m_selectAllAction;
    QAction *m_selectNoneAction;
    QAction *m_setOneAction;
    QAction *m_setZeroAction;
    //QAction *m_setRandomAction; // TODO: implement this
    QList<QPointer<QWidget> > m_persistentEditor;
    QPointer<QWidget> m_editor;
};

#endif // DATASOURCETABLEWIDGET_H_
