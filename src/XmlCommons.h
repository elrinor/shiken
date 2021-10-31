#ifndef XML_COMMONS_H
#define XML_COMMONS_H

#include "config.h"
#include <QString>
#include <QDomDocument>
#include <QDomElement>

inline QDomElement appendElement(QDomNode &node, const QString &tag) {
  return node.appendChild(node.ownerDocument().createElement(tag)).toElement();
}

inline QDomElement appendElement(QDomNode &node, const QString &tag, const QString &text) {
  QDomElement result = appendElement(node, tag);
  result.appendChild(node.ownerDocument().createTextNode(text));
  return result;
}

inline QDomDocument newDocument() {
  QDomDocument document;
  document.insertBefore(document.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""), document.firstChild());
  return document;
}

#endif // XML_COMMONS_H
