package ru.novosoft.smsc.admin.journal;

import org.apache.log4j.Category;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 04.11.2003
 * Time: 17:01:33
 */
public class Journal
{
  private List actions = new LinkedList();
  private Category logger = Category.getInstance(this.getClass());
  private Category journalLogger = Category.getInstance("journal");

  public Journal()
  {
    logger.info("Journal created");
  }

  public synchronized void append(Action action)
  {
    actions.add(action);
    final String actionStr = action.toString();
    journalLogger.info(actionStr);
    logger.debug("APPEND " + actionStr);
  }

  public synchronized void append(String userName, String sessionId, byte subjectType, String subjectId, byte action)
  {
    append(userName, sessionId, subjectType, subjectId, action, null);
  }

  public synchronized void append(String userName, String sessionId, byte subjectType, String subjectId, byte action,
                                  Date timestamp)
  {
    append(userName, sessionId, subjectType, subjectId, action, timestamp, null);
  }

  public synchronized void append(String userName, String sessionId, byte subjectType, String subjectId, byte action,
                                  Date timestamp, String additionalKey, String additionalValue)
  {
    Map additional = new HashMap();
    additional.put(additionalKey, additionalValue);
    append(userName, sessionId, subjectType, subjectId, action, timestamp, additional);
  }

  public synchronized void append(String userName, String sessionId, byte subjectType, String subjectId, byte action,
                                  Date timestamp, Map additional)
  {
    append(new Action(userName, sessionId, subjectType, subjectId, action, timestamp, additional));
  }

  public synchronized void clear(byte subjectType)
  {
    logger.debug("CLEAR(subjectType)");
    for (Iterator i = new LinkedList(actions).iterator(); i.hasNext();) {
      Action action = (Action) i.next();
      if (action.getSubjectType() == subjectType) {
        actions.remove(action);
        if (logger.isDebugEnabled())
          logger.debug("removed: " + action.toString());
      }
    }
    logger.debug("CLEAR(subjectType) FINISHED");
  }

  public synchronized void clear(Collection subjectTypes)
  {
    logger.debug("CLEAR(subjectTypes)");
    for (Iterator i = new LinkedList(actions).iterator(); i.hasNext();) {
      Action action = (Action) i.next();
      if (subjectTypes.contains(new Byte(action.getSubjectType()))) {
        actions.remove(action);
        if (logger.isDebugEnabled())
          logger.debug("removed: " + action.toString());
      }
    }
    logger.debug("CLEAR(subjectTypes) FINISHED");
  }

  public synchronized void clear(byte subjectType, String subjectId)
  {
    logger.debug("CLEAR(subjectType, subjectID)");
    for (Iterator i = new LinkedList(actions).iterator(); i.hasNext();) {
      Action action = (Action) i.next();
      if (action.getSubjectType() == subjectType && action.getSubjectId().equals(subjectId)) {
        actions.remove(action);
        if (logger.isDebugEnabled())
          logger.debug("removed: " + action.toString());
      }
    }
    logger.debug("CLEAR(subjectType, subjectID) FINISHED");
  }

  public synchronized List getActions()
  {
    return new LinkedList(actions);
  }

  public synchronized List getActions(byte subjectType)
  {
    List result = new LinkedList();
    for (Iterator i = actions.iterator(); i.hasNext();) {
      Action action = (Action) i.next();
      if (action.getSubjectType() == subjectType)
        result.add(action);
    }
    return result;
  }

  public synchronized List getActions(Collection subjectTypes)
  {
    List result = new LinkedList();
    for (Iterator i = actions.iterator(); i.hasNext();) {
      Action action = (Action) i.next();
      if (subjectTypes.contains(new Byte(action.getSubjectType())))
        result.add(action);
    }
    return result;
  }
}