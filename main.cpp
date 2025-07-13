#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQueryModel>
#include <QtWidgets/QTableView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtSql/QSqlError>

class TableViewer : public QMainWindow
{
public:
    TableViewer(QWidget *parent = 0) : QMainWindow(parent)
    {
        setupUI();
        setupDatabase();
    }

    ~TableViewer()
    {
        if (db.isOpen()) {
            db.close();
        }
    }

private:
    QSqlDatabase db;
    QSqlQueryModel *model;
    QTableView *tableView;
    QLabel *statusLabel;

    void setupUI()
    {
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *btnTable1 = new QPushButton("Рыбы", this);
        QPushButton *btnTable2 = new QPushButton("Наживки", this);
        QPushButton *btnTable3 = new QPushButton("Реки", this);
        QPushButton *btnTable4 = new QPushButton("Рыбы и наживки", this);
        QPushButton *btnTable5 = new QPushButton("Рыбы и реки", this);

        buttonLayout->addWidget(btnTable1);
        buttonLayout->addWidget(btnTable2);
        buttonLayout->addWidget(btnTable3);
        buttonLayout->addWidget(btnTable4);
        buttonLayout->addWidget(btnTable5);

        tableView = new QTableView(this);
        model = new QSqlQueryModel(this);
        tableView->setModel(model);

        mainLayout->addLayout(buttonLayout);
        mainLayout->addWidget(tableView);
        mainLayout->addWidget(statusLabel);

        setCentralWidget(centralWidget);

        connect(btnTable1, &QPushButton::clicked, [this]() { showTable("fishes"); });
        connect(btnTable2, &QPushButton::clicked, [this]() { showTable("baits"); });
        connect(btnTable3, &QPushButton::clicked, [this]() { showTable("rivers"); });
        connect(btnTable4, &QPushButton::clicked, [this]() { showFishesBaits(); });
        connect(btnTable5, &QPushButton::clicked, [this]() { showFishesRivers(); });
    }

    void setupTable(QVBoxLayout *mainLayout) {
        tableView = new QTableView(this);
        model = new QSqlQueryModel(this);
        tableView->setModel(model);
        mainLayout->addWidget(tableView);
    }

    void setupDatabase() {
        db = QSqlDatabase::addDatabase("QPSQL");
        db.setHostName("localhost");
        db.setPort(5432);
        db.setDatabaseName("postgres");
        db.setUserName("postgres");
        db.setPassword("1");

        if (!db.open()) {
            QMessageBox::critical(this, "Ошибка подключения",
                                "Не удалось подключиться к базе данных:\n" + db.lastError().text());
        }
    }

    void showTable(const QString &tableName) {
        if (!db.isOpen()) return;

        model->setQuery("SELECT * FROM " + tableName, db);
        handleQueryError();
        tableView->resizeColumnsToContents();
    }

    void showFishesBaits() {
        if (!db.isOpen()) return;

        model->setQuery(
            "SELECT fishes.name, fishes.weight, fishes.lenght_sm, fishes.predator, "
            "baits.name, baits.cost, baits.weight "
            "FROM fishes "
            "INNER JOIN fish_bait ON fishes.fish_id = fish_bait.fish_id "
            "INNER JOIN baits ON fish_bait.bait_id = baits.bait_id", db);
        handleQueryError();
        tableView->resizeColumnsToContents();
    }

    void showFishesRivers() {
        if (!db.isOpen()) return;

        model->setQuery(
            "SELECT fishes.name, fishes.weight, fishes.lenght_sm, fishes.predator, "
            "rivers.name, rivers.lenght_km, rivers.depth_m, rivers.max_flow_speed "
            "FROM fishes "
            "FULL OUTER JOIN fish_river ON fishes.fish_id = fish_river.fish_id "
            "FULL OUTER JOIN rivers ON fish_river.river_id = rivers.river_id", db);
        handleQueryError();
        tableView->resizeColumnsToContents();
    }

    void handleQueryError() {
        if (model->lastError().isValid()) {
            QMessageBox::warning(this, "Ошибка запроса",
                               "Не удалось выполнить запрос:\n" + model->lastError().text());
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TableViewer viewer;
    viewer.setWindowTitle("PostgreSQL Table Viewer");
    viewer.resize(800, 600);
    viewer.show();

    return app.exec();
}
