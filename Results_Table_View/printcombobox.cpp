#include "printcombobox.h"
#include <QComboBox>
#include <QApplication>

PrintComboBox::PrintComboBox(QObject *parent) : QAbstractItemDelegate (parent)
{
}


void PrintComboBox::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    QStyleOptionComboBox comboboxstyle;

    comboboxstyle.rect = option.rect;
    comboboxstyle.rect.setLeft(option.rect.x() + option.rect.width()/8);
    comboboxstyle.rect.setTop(option.rect.y() + option.rect.height()/8);
    comboboxstyle.rect.setWidth(6*option.rect.width()/8);
    comboboxstyle.rect.setHeight(6*option.rect.height()/8);
    comboboxstyle.state = QStyle::State_Active | QStyle::State_Enabled;

    auto list = index.model()->data(index.model()->index(index.row(), index.column()), Qt::UserRole).toString().split("/");
    unsigned int currentIndex = list.last().toInt();
    list.pop_back();
    comboboxstyle.currentText = list.at(currentIndex);

    QApplication::style()->drawComplexControl(QStyle::CC_ComboBox, &comboboxstyle, painter);
    QApplication::style()->drawControl(QStyle::CE_ComboBoxLabel, &comboboxstyle, painter);
}

/**
 * @brief Возникает погрешность при делении, значение не точное
 */
QSize PrintComboBox::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.isValid())
    {
        return 6*option.rect.size()/8;
    }
    return QSize();
}

void PrintComboBox::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = dynamic_cast<QComboBox *>(editor);

    model->setData(model->index(index.row(), index.column()), comboBox->currentIndex(), Qt::UserRole);
}

void PrintComboBox::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = dynamic_cast<QComboBox*>(editor);
    QStringList list = index.model()->data(index.model()->index(index.row(), index.column()), Qt::UserRole).toString().split("/");
    unsigned int currentIndex = list.last().toInt();
    list.pop_back();
    comboBox->addItems(list);
    comboBox->setCurrentIndex(currentIndex);
}

QWidget *PrintComboBox::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox * editor = new QComboBox(parent);

    return editor;
}

void PrintComboBox::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionComboBox comboboxstyle;

    comboboxstyle.rect = option.rect;
    comboboxstyle.rect.setLeft(option.rect.x() + option.rect.width()/8);
    comboboxstyle.rect.setTop(option.rect.y() + option.rect.height()/8);
    comboboxstyle.rect.setWidth(6*option.rect.width()/8);
    comboboxstyle.rect.setHeight(6*option.rect.height()/8);

    editor->setGeometry(comboboxstyle.rect);
}


