#ifndef SHIKEN_SQL_QUERY_MODEL_H
#define SHIKEN_SQL_QUERY_MODEL_H

#include <shiken/config.h>
#include <QStandardItemModel>
#include <QSqlDatabase>

class QSqlQueryModel;
class QSqlQuery;

namespace shiken {
// -------------------------------------------------------------------------- //
// SqlQueryModel
// -------------------------------------------------------------------------- //
  /**
   * SqlQueryModel is a QStandardItemModel that gets its data from 
   * QSqlQueryModel. 
   * 
   * Additional signals and slots allow to update the model when the 
   * underlying data is updated, and to perform any kind of post-processing.
   */
  class SqlQueryModel: public QStandardItemModel {
    Q_OBJECT;
  public:
    SqlQueryModel(QObject *parent = NULL);

    void setQuery(const QSqlQuery &query);

    void setQuery(const QString &query, const QSqlDatabase &connection = QSqlDatabase());

  signals:
    /** 
     * This signal is emitted whenever this model re-synchronizes its 
     * contents with the underlying database. 
     *
     * Post-processing may be performed in a slot connected to this signal.
     */
    void updated();

  public slots:
    /**
     * Re-synchronizes the contents of this model with the underlying database.
     */
    void update();

  protected:
    void populate();

  private:
    /** Underlying SQL query model. */
    QSqlQueryModel *mSqlQueryModel;
  };

} // namespace shiken

#endif // SHIKEN_SQL_QUERY_MODEL_H
