#include "SqlQueryModel.h"
#include <QSqlQueryModel>
#include <QSqlQuery>

namespace shiken {
  SqlQueryModel::SqlQueryModel(QObject *parent): QStandardItemModel(parent) {
    mSqlQueryModel = new QSqlQueryModel(this);
  }

  void SqlQueryModel::setQuery(const QSqlQuery &query) {
    mSqlQueryModel->setQuery(query);
    populate();
  }

  void SqlQueryModel::setQuery(const QString &query, const QSqlDatabase &connection) {
    mSqlQueryModel->setQuery(query, connection);
    populate();
  }

  void SqlQueryModel::update() {
    QString queryString = mSqlQueryModel->query().lastQuery();
    mSqlQueryModel->query().clear();
    mSqlQueryModel->query().prepare(queryString);
    mSqlQueryModel->query().exec();
    mSqlQueryModel->setQuery(mSqlQueryModel->query());
    populate();
  }

  void SqlQueryModel::populate() {
    setRowCount(0);
    setRowCount(mSqlQueryModel->rowCount());
    setColumnCount(mSqlQueryModel->columnCount());

    for(int row = 0; row < mSqlQueryModel->rowCount(); row++)
      for(int column = 0; column < mSqlQueryModel->columnCount(); column++)
        setData(index(row, column), mSqlQueryModel->index(row, column).data());

    emit updated();
  }

} // namespace shiken
