#include "Shiken.h"
#include <cassert>
#include <arx/StaticBlock.h>
#include <arx/Foreach.h>
#include <QDomDocument>
#include <shiken/dao/DataAccessDriver.h>
#include <shiken/dao/QuizDao.h>
#include <shiken/dao/UserDao.h>
#include <shiken/entities/Quiz.h>
#include <shiken/entities/User.h>
#include <shiken/network/ConnectionManager.h>
#include <shiken/utility/GuidCompressor.h>
#include <shiken/utility/Log.h>

namespace shiken {
  Shiken::Shiken(DataAccessDriver *model): 
    mModel(model)
  {
    assert(model != NULL);

    mConnectionManager.reset(new ConnectionManager(this));
  }

  Shiken::~Shiken() {
    return;
  }

  UsersReply Shiken::parseQuizXml(const QByteArray &data, bool fromFile) {
    SHIKEN_LOG_MESSAGE("parseQuizXml(" << data << ")");

    QDomDocument doc;
    doc.setContent(data);

    /* Find examlist node. */
    QDomElement node = doc.documentElement();
    while(node.nodeName() != "examlist") {
      node = node.nextSiblingElement();
      if(node.isNull())
        return UsersReply();
    }

    /* Deserialize quiz. */
    Quiz quiz;
    quiz.setGuid(node.attribute("guid"));
    quiz.setDiscStr(node.attribute("projname"));
    quiz.setTitle(node.attribute("name"));
    quiz.setPrjGuid(node.attribute("project"));
    quiz.setQsgGuid(node.attribute("qsgguid"));
    quiz.setLoadedFromXml(fromFile);
    quiz.fixup();

    /* Deserialize users. */
    QList<User> users;
    for(node = node.firstChildElement(); !node.isNull(); node = node.nextSiblingElement()) {
      if(node.nodeName() != "user")
        continue;

      /* node.attribute("variant") is currently ignored. */

      User user;
      user.setLogin(node.attribute("login"));
      user.setGuid(node.attribute("guid"));
      user.setCompressedGuid(GuidCompressor()(quiz.guidToCompress(), user.guid()));
      user.setUnknown(false);
      users.push_back(user);
    }

    return UsersReply(quiz, users);
  }

  Quiz Shiken::storeQuizData(const UsersReply &usersReply) {
    Quiz quiz = usersReply.quiz();
    QList<User> users = usersReply.users();

    /* Check whether such quiz exists in the database and update it
     * correspondingly. */
    Quiz existingQuiz = model()->quizDao()->selectByGuid(quiz.guid());
    if(!existingQuiz.isNull()) {
      existingQuiz.setPrjGuid(quiz.prjGuid());
      existingQuiz.setQsgGuid(quiz.qsgGuid());
      quiz = existingQuiz;
      model()->quizDao()->update(quiz);
    } else {
      model()->quizDao()->insert(quiz);
    }

    /* Update users' quiz ids. */
    foreach(User &user, users)
      user.setQuizId(quiz.quizId());

    /* Store users into the database. */
    foreach(User &user, users) {
      User existingUser = model()->userDao()->selectByLoginAndQuizId(user.login(), user.quizId());

      if(existingUser.isNull())
        model()->userDao()->insert(user);
    }

    return quiz;
  }

} // namespace shiken
