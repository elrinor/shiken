#ifndef SHIKEN_VERSION_GETTER_H
#define SHIKEN_VERSION_GETTER_H

#include <shiken/config.h>
#include <QUrl>
#include "Worker.h"

namespace shiken {
  class Shiken;

// -------------------------------------------------------------------------- //
// VersionGetter
// -------------------------------------------------------------------------- //
  /**
   * This worker gets current version number from the server.
   */
  class VersionGetter: public Worker<int> {
    Q_OBJECT;
  public:
    VersionGetter(Shiken *ctx, const QUrl &url):
      Worker<int>(ctx, 1), mUrl(url) {}

    virtual int operator()() OVERRIDE;

  private:
    QUrl mUrl;
  };

} // namespace shiken

#endif // SHIKEN_VERSION_GETTER_H
