#include "lineeditextended.h"

#include <QDebug>
#include <QToolButton>
#include <QStyle>
LineEditExtended::LineEditExtended(const int number, QWidget *parent) : QLineEdit(parent)
{
    this->number = number;
    filterMemory = "";
    installEventFilter(this);
    this->setPlaceholderText("Filter");

    clearButton = new QToolButton(this);
    QPixmap map("://new//prefix1//clearButton.png");// temp
    clearButton->setIcon(QIcon(map));
    clearButton->setIconSize(map.size());
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");

    connect(clearButton, &QToolButton::clicked, this, &LineEditExtended::clear);
    connect(this, &LineEditExtended::textChanged, this, &LineEditExtended::updateClearButton);

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(clearButton->sizeHint().width() + frameWidth));// устанавливаем размер QLineEdit от правого края

    clearButton->hide();
}

QString& LineEditExtended::getFilterMemory()
{
    return filterMemory;
}

void LineEditExtended::setFilterMemory(const QString &value)
{
    filterMemory = value;
}

bool LineEditExtended::eventFilter(QObject *watched, QEvent *event)
{
    return QLineEdit::eventFilter(watched, event);
}

int LineEditExtended::getNumber() const
{
    return number;
}

void LineEditExtended::updateClearButton(const QString& text)
{
    clearButton->setVisible(!text.isEmpty());
}

void LineEditExtended::resizeEvent(QResizeEvent *event)
{
    QSize sz = clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    clearButton->move(rect().right() - frameWidth - sz.width(), (rect().bottom() + 1 - sz.height())/2);
    return QLineEdit::resizeEvent(event);
}
