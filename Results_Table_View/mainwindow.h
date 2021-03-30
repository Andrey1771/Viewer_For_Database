#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_MainWindow.h"
//#include "qsqlfield.h"

#include <QMainWindow>
#include <QTranslator>
#include <QtSql/QSqlDatabase>
#include "printdialog.h"

class ProtocolPrinterItemModel;
class ProtocolPrinterHeaderView;
class ProtocolPrinterHeaderViewExtended;
class QSortFilterProxyModel;
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
    void onMenuHelpActionTriggered(QAction* action);
    void onOpenDBAction();
    void onExportDBAction();
    void onQuitAction();
    void onDocumentationAction();
    //XML
private:
    Ui::MainWindow *ui {nullptr};
    QTranslator qLangTranslator;

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

    void savePathDb(const QString& databaseName);
    void saveExportPath(QString = "");
    void saveData(const QVariantHash& c);
    const QString loadPathExportDialog();

    const QString loadDb();
    QList<QString> getNamesTables();

    void updateScrollBar(int count);
    void ProxyNewModelLoad();
    void ProxyModelSupLoad();
    void loadSortModelsList();

    QList <QString> Names;
    int numberTable;
    //ProtocolPrinterItemModel *model{nullptr};
    NewModel *newModel{nullptr};
    ProtocolPrinterItemModel *modelSup{nullptr};
    ProtocolPrinterHeaderView *filter{nullptr};
    ProtocolPrinterHeaderView *filter_Sup{nullptr};
    QSortFilterProxyModel *proxyModelNewModel{nullptr};
    QSortFilterProxyModel *proxyModelModelSup{nullptr};
    QList<bool> sortTypeNewModelList;
    QList<bool> sortTypeModelSupList;

    QSqlDatabase db;
    const QStringList hidenColNewModelList {/*"Session ID", "Project Name", "LRU Name", "LRU S/N", "ATE P/N", "ATE S/N", "ATE S/W Ver"*/};
    const QStringList hidenColModelSupList {/*"Session ID", "Project Name", "LRU Name", "LRU S/N", "ATE P/N", "ATE S/N", "ATE S/W Ver"*/};
    const QMap<QString, QString> supportedLanguages { {"EN", ":/translations/english.qm"}, {"RU", ":/translations/russian.qm"}};

    PrintDialog::DataForSaver exportDataForSaver;
    QVariantHash c;

public slots:
    void sliderUpdateModel(int action);
    void updateModel();
private slots:
    void colomnNewModelFilter(int logicalIndex);
    void colomnModelFilter(int logicalIndex);

    // QWidget interface
protected:
    void changeEvent(QEvent *event) override;
};

#endif // MAINWINDOW_H
