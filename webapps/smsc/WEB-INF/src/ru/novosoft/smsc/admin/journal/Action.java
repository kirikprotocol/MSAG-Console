package ru.novosoft.smsc.admin.journal;

import java.text.DateFormat;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 04.11.2003
 * Time: 17:01:07
 */
public class Action
{
  private final String user;
  private final String sessionId;
  private final byte subjectType;
  private final String subjectId;
  private final byte action;
  private final Date timestamp;
  private final Map additional;

  private static final DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT);

  public Action(String user, String sessionId, byte subjectType, String subjectId, byte action, Date timestamp, Map additional)
  {
    this.user = user == null ? "" : user;
    this.sessionId = sessionId == null ? "" : sessionId;
    this.subjectType = subjectType;
    this.subjectId = subjectId == null ? "<unknown>" : subjectId;
    this.action = action;
    this.timestamp = timestamp == null ? new Date() : timestamp;
    this.additional = additional == null ? new HashMap() : additional;
  }

  public String toString()
  {
    StringBuffer sb = new StringBuffer()
            .append("Action [time:").append(dateFormat.format(timestamp))
            .append(" user:").append(user)
            .append(" session:").append(sessionId)
            .append(" subjType:").append(SubjectTypes.typeToString(subjectType))
            .append(" subjId:").append(subjectId)
            .append(" action:").append(Actions.actionToString(action));
    if (additional.size() > 0) {
      sb.append(" additional:[");
      for (Iterator i = additional.entrySet().iterator(); i.hasNext();) {
        Map.Entry entry = (Map.Entry) i.next();
        sb.append((String)entry.getKey());
        sb.append("=");
        sb.append((String)entry.getValue());
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
    StringBuffer sb = new StringBuffer();
    sb
            .append(" user:").append(user)
            .append(" ").append(Actions.actionToString(action))
            .append(" ").append(SubjectTypes.typeToString(subjectType))
            .append(" ").append(subjectId);
    if (additional.size() > 0) {
      sb.append(" additional:[");
      for (Iterator i = additional.entrySet().iterator(); i.hasNext();) {
        Map.Entry entry = (Map.Entry) i.next();
        sb.append((String)entry.getKey());
        sb.append("=");
        sb.append((String)entry.getValue());
        if (i.hasNext())
          sb.append(", ");
      }
      sb.append(']');
    }
    return sb.toString();
  }

  public void setAdditionalValue(String key, String value)
  {
    additional.put(key, value);
  }

  public String getAdditionalValue(String key)
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
}