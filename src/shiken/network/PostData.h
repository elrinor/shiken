#ifndef SHIKEN_POST_DATA_H
#define SHIKEN_POST_DATA_H

#include <shiken/config.h>
#include <QString>
#include <QList>
#include <QByteArray>
#include <QFile>
#include <QDir>

namespace shiken {
// -------------------------------------------------------------------------- //
// PostData
// -------------------------------------------------------------------------- //
  /**
   * PostData encapsulates data to be posted in a HTTP post request.
   */
  class PostData {
  public:
    /**
     * Single field in a HTTP post request.
     */
    class Field {
    public:
      Field(const QString &key, const QString &value): mKey(key), mValue(value) {}

      const QString &key() const {
        return mKey;
      }

      const QString &value() const {
        return mValue;
      }

    private:
      QString mKey, mValue;
    };

    /**
     * Single file in a HTTP post request.
     */
    class File {
    public:
      File(const QString &key, const QString &fileName, const QString &mime, const QByteArray &data):
        mKey(key), mFileName(fileName), mMime(mime), mData(data) {}

      const QString &key() const {
        return mKey;
      }

      const QString &fileName() const {
        return mFileName;
      }

      const QString &mime() const {
        return mMime;
      }

      const QByteArray &data() const {
        return mData;
      }

    private:
      QString mKey;
      QString mFileName;
      QString mMime;
      QByteArray mData;
    };

    /**
     * Adds a field to this post data.
     *
     * @param key                      Field key.
     * @param value                    Value for the given key.
     */
    PostData &addField(const QString &key, const QString &value) {
      mFields.push_back(Field(key, value));

      return *this;
    }

    /**
     * Adds a file to this post data.
     *
     * @param key                      File key.
     * @param fileName                 Full path to the file to add.
     * @param mime                     File mime type.
     */
    PostData &addFile(const QString &key, const QString &filePath, const QString &mime) {
      QFile file(filePath);
      file.open(QIODevice::ReadOnly);

      return addFile(key, QFileInfo(filePath).fileName(), mime, file.readAll());
    }

    /**
     * Adds a file to this post data.
     *
     * @param key                      File key.
     * @param fileName                 Name of the file to add.
     * @param mime                     File mime type.
     * @param data                     File bytes.
     */
    PostData &addFile(const QString &key, const QString &fileName, const QString &mime, const QByteArray &data) {
      mFiles.push_back(File(key, fileName, mime, data));

      return *this;
    }

    /**
     * @returns                        List of all fields in this post data.
     */
    const QList<Field> &fields() const {
      return mFields;
    }

    /**
     * @returns                        List of all files in this post data.
     */
    const QList<File> &files() const {
      return mFiles;
    }

  private:
    QList<Field> mFields;
    QList<File> mFiles;
  };

} // namespace shiken

#endif // SHIKEN_POST_DATA_H
