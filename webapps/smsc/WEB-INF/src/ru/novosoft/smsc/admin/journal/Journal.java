package ru.novosoft.smsc.admin.journal;

import org.apache.log4j.Category;

import java.util.*;


/**
 * Created by IntelliJ IDEA. User: igork Date: 04.11.2003 Time: 17:01:33
 */
public class Journal
{
  private List actions = new LinkedList();
  private Category logger = Category.getInstance(this.getClass());
  private Category journalLogger = Category.getInstance("journal");

	private static final Set nonAppliableSubjectTypes = new HashSet();

	static
	{
		nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_dl));
		nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_locale));
		nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_profile));
		nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_service));
		nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_logger));
		nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_msc));
	}

  public Journal()
  {
    logger.info("Journal created");
  }

  public synchronized void append(final Action action)
  {
    assert Actions.ACTION_UNKNOWN != action.getAction()
           && SubjectTypes.TYPE_UNKNOWN != action.getSubjectType()
           && null != action.getSubjectId() && 0 < action.getSubjectId().trim().length()
        : "action not initialized properly";
    actions.add(action);
    final String actionStr = action.toString();
    journalLogger.info(action.toJournalString());
    logger.debug("APPEND " + actionStr);
  }

  public synchronized void append(final String userName, final String sessionId, final byte subjectType, final String subjectId, final byte action)
  {
    append(userName, sessionId, subjectType, subjectId, action, null);
  }

  public synchronized void append(final String userName, final String sessionId, final byte subjectType, final String subjectId, final byte action,
                                  final String additionalKey, final String additionalValue)
  {
    final Map additional = new HashMap();
    additional.put(additionalKey, additionalValue);
    append(userName, sessionId, subjectType, subjectId, action, additional);
  }

  public synchronized void append(final String userName, final String sessionId, final byte subjectType, final String subjectId, final byte action,
                                  final Map additional)
  {
    append(new Action(userName, sessionId, subjectType, subjectId, action, additional));
  }

  public synchronized void clear(final byte subjectType)
  {
    logger.debug("CLEAR(subjectType)");
    for (Iterator i = new LinkedList(actions).iterator(); i.hasNext();) {
      final Action action = (Action) i.next();
      if (action.getSubjectType() == subjectType) {
        actions.remove(action);
        if (logger.isDebugEnabled())
          logger.debug("removed: " + action.toString());
      }
    }
    logger.debug("CLEAR(subjectType) FINISHED");
  }

  public synchronized void clear(final Collection subjectTypes)
  {
    logger.debug("CLEAR(subjectTypes)");
    for (Iterator i = new LinkedList(actions).iterator(); i.hasNext();) {
      final Action action = (Action) i.next();
      if (subjectTypes.contains(new Byte(action.getSubjectType()))) {
        actions.remove(action);
        if (logger.isDebugEnabled())
          logger.debug("removed: " + action.toString());
      }
    }
    logger.debug("CLEAR(subjectTypes) FINISHED");
  }

  public synchronized void clear(final byte subjectType, final String subjectId)
  {
    logger.debug("CLEAR(subjectType, subjectID)");
    for (Iterator i = new LinkedList(actions).iterator(); i.hasNext();) {
      final Action action = (Action) i.next();
      if (action.getSubjectType() == subjectType && action.getSubjectId().equals(subjectId)) {
        actions.remove(action);
        if (logger.isDebugEnabled())
          logger.debug("removed: " + action.toString());
      }
    }
    logger.debug("CLEAR(subjectType, subjectID) FINISHED");
  }

	public synchronized void nonAppliableClear()
	{
		clear(nonAppliableSubjectTypes);
	}

  public synchronized List getActions()
  {
    return new LinkedList(actions);
  }

  public synchronized List getActions(final byte subjectType)
  {
    final List result = new LinkedList();
    for (Iterator i = actions.iterator(); i.hasNext();) {
      final Action action = (Action) i.next();
      if (action.getSubjectType() == subjectType)
        result.add(action);
    }
    return result;
  }

  public synchronized List getActions(final Collection subjectTypes)
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