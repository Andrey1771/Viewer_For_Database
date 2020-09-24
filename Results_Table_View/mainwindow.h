#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_MainWindow.h"
#include "qsqlfield.h"

#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QTimer>

class ProtocolPrinterItemModel;
class ProtocolPrinterHeaderView;
class ProtocolPrinterHeaderViewExtended;

class NewModel;
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void connectionDone();
    void modelUpFetchMore();

public slots:
    void setTable(int index);
    bool createConnection();
    bool createConnection(const QString& fileName);
    void disconnect();

    //XML
    void onFileMenuActionTriggered(QAction* action);
    void onOpenDBAction();
    void onExportDBAction();
    void onQuitAction();
    //XML
private:
    Ui::MainWindow *ui {nullptr};

    bool dbIsOpen(QSqlDatabase& db);

    void setPerson();
    void setItems();
    void hideShowLog();
    void hideShowSup();
    void launchSetSettings();
    void launchWizard();
    void fillingComboBoxTableName();
    void addDatabase();
    void hideColumnsModels();
    void saveDb(const QString& databaseName);
    const QString loadDb();
    QList<QString> getNamesTables();

    void updateScrollBar(int count);

    QList <QString> Names;
    int numberTable;
    //ProtocolPrinterItemModel *model{nullptr};
    NewModel *newModel{nullptr};
    ProtocolPrinterItemModel *modelSup{nullptr};
    ProtocolPrinterHeaderView *filter{nullptr};
    ProtocolPrinterHeaderView *filter_Sup{nullptr};
    QSqlDatabase db;
    QTimer waitFetchMoreTimer;
    const QStringList hidenColNewModelList {/*"Session ID", "Project Name", "LRU Name", "LRU S/N", "ATE P/N", "ATE S/N", "ATE S/W Ver"*/};
    const QStringList hidenColModelSupList {/*"Session ID", "Project Name", "LRU Name", "LRU S/N", "ATE P/N", "ATE S/N", "ATE S/W Ver"*/};
public slots:
    void sliderUpdateModel(int action);

};

#endif // MAINWINDOW_H
