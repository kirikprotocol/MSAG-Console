package ru.sibinco.smppgw.backend.protocol.journal;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 18:40:33
 * To change this template use File | Settings | File Templates.
 */

import java.text.DateFormat;
import java.util.*;


/**
 * Created by IntelliJ IDEA. User: igork Date: 04.11.2003 Time: 17:01:07
 */
public class Action
{
  private final String user;
  private final String sessionId;
  private byte subjectType ;
  private String subjectId ;
  private byte action ;
  private final Date timestamp;
  private final Map additional;

  private static final DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT);

  public Action(final String user, final String sessionId)
  {
    this(user, sessionId, "[unknown]");
  }

  public Action(final String user, final String sessionId, final String subjectId)
  {
    this(user, sessionId, subjectId, new HashMap());
  }

  public Action(final String user, final String sessionId, final String subjectId, final String additionalKey, final String additionalValue)
  {
    this(user, sessionId, subjectId, new HashMap());
    additional.put(additionalKey, additionalValue);
  }

  public Action(final String user, final String sessionId, final String subjectId, final Map additional)
  {
    this(user, sessionId, SubjectTypes.TYPE_UNKNOWN, subjectId, Actions.ACTION_UNKNOWN, additional);
  }

  protected Action(final String user, final String sessionId, final byte subjectType, final String subjectId, final byte action)
  {
    this(user, sessionId, subjectType, subjectId, action, new HashMap());
  }

  protected Action(final String user, final String sessionId, final byte subjectType, final String subjectId, final byte action, final String additionalKey, final String additionalValue)
  {
    this(user, sessionId, subjectType, subjectId, action, new HashMap());
    additional.put(additionalKey, additionalValue);
  }

  protected Action(final String user, final String sessionId, final byte subjectType, final String subjectId, final byte action, final Map additional)
  {
    this.user = null == user ? "" : user;
    this.sessionId = null == sessionId ? "" : sessionId;
    this.subjectId = null == subjectId ? "<unknown>" : subjectId;
    this.timestamp = new Date();
    this.additional = null == additional ? new HashMap() : additional;
    this.subjectType = subjectType;
    this.action = action;
  }

  public String toString()
  {
    final StringBuffer sb = new StringBuffer()
        .append("Action [time:").append(dateFormat.format(timestamp))
        .append(" user:").append(user)
        .append(" session:").append(sessionId)
        .append(" subjType:").append(SubjectTypes.typeToString(subjectType))
        .append(" subjId:").append(subjectId)
        .append(" action:").append(Actions.actionToString(action));
    if (0 < additional.size()) {
      sb.append(" additional:[");
      for (Iterator i = additional.entrySet().iterator(); i.hasNext();) {
        final Map.Entry entry = (Map.Entry) i.next();
        sb.append((String) entry.getKey());
        sb.append("=");
        sb.append((String) entry.getValue());
        if (i.hasNext())
          sb.append(", ");
      }
      sb.append(']');
    }
    sb.append(']');
    return sb.toString();
  }

  public String toJournalString()
  {
    final StringBuffer sb = new StringBuffer();
    sb
        .append(" user:").append(user)
        .append(" ").append(Actions.actionToString(action))
        .append(" ").append(SubjectTypes.typeToString(subjectType))
        .append(" ").append(subjectId);
    if (0 < additional.size()) {
      sb.append(" additional:[");
      for (Iterator i = additional.entrySet().iterator(); i.hasNext();) {
        final Map.Entry entry = (Map.Entry) i.next();
        sb.append((String) entry.getKey());
        sb.append("=");
        sb.append((String) entry.getValue());
        if (i.hasNext())
          sb.append(", ");
      }
      sb.append(']');
    }
    return sb.toString();
  }

  public void setAdditionalValue(final String key, final String value)
  {
    additional.put(key, value);
  }

  public String getAdditionalValue(final String key)
  {
    return (String) additional.get(key);
  }

  public Set getAdditionalKeys()
  {
    return additional.keySet();
  }

  public String getUser()
  {
    return user;
  }

  public String getSessionId()
  {
    return sessionId;
  }

  public byte getSubjectType()
  {
    return subjectType;
  }

  public String getSubjectId()
  {
    return subjectId;
  }

  public byte getAction()
  {
    return action;
  }

  public Date getTimestamp()
  {
    return timestamp;
  }

  public void setSubjectType(final byte subjectType)
  {
    this.subjectType = subjectType;
  }

  public void setAction(final byte action)
  {
    this.action = action;
  }

  public void setSubjectId(String subjectId)
  {
    this.subjectId = subjectId;
  }
}