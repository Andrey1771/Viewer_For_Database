#ifndef LINEEDITEXTENDED_H
#define LINEEDITEXTENDED_H

#include <QLineEdit>

class QEvent;
class QObject;
class QToolButton;

class LineEditExtended : public QLineEdit
{
    Q_OBJECT
    int number;
    QString filterMemory;
    QToolButton *clearButton{nullptr};

public:
    LineEditExtended(const int number = 0, QWidget *parent = nullptr);
    int getNumber() const;
    QString &getFilterMemory();
    void setFilterMemory(const QString &value);

public slots:
    void updateClearButton(const QString& text);

signals:
    void clearButtonPressed ();

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event) override;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // LINEEDITEXTENDED_H
