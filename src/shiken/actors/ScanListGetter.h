#ifndef SHIKEN_SCAN_LIST_GETTER_H
#define SHIKEN_SCAN_LIST_GETTER_H

#include <shiken/config.h>
#include <QUrl>
#include <QList>
#include <shiken/entities/Scan.h>
#include "Worker.h"

namespace shiken {
    class Shiken;
    class ScanListGetterPrivate;
    class Quiz;

    /**
     * This worker gets a list of uploaded scans for the given quiz from the
     * server.
     */
    class ScanListGetter: public Worker<QList<ExtendedScan> > {
        Q_OBJECT;
    public:
        ScanListGetter(Shiken *ctx, const QUrl &url, const Quiz &quiz);

        virtual ~ScanListGetter();

        virtual QList<ExtendedScan> operator()() OVERRIDE;

    private:
        QScopedPointer<ScanListGetterPrivate> d;
    };

} // namespace shiken

#endif // SHIKEN_SCAN_LIST_GETTER_H
