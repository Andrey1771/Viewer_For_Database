#ifndef PRINTCOMBOBOX_H
#define PRINTCOMBOBOX_H
#include <QAbstractItemDelegate>

class QComboBox;
class PrintComboBox : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    PrintComboBox(QObject *parent = nullptr);//hover event

   void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
   QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
   void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
   void setEditorData(QWidget *editor, const QModelIndex &index) const override;
   QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
   void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;


};

#endif // PRINTCOMBOBOX_H
