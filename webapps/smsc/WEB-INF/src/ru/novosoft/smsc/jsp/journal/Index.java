package ru.novosoft.smsc.jsp.journal;

import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.admin.journal.Action;
import ru.novosoft.smsc.admin.journal.SubjectTypes;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 04.11.2003
 * Time: 18:25:11
 */
public class Index extends IndexBean
{
  private static final byte SORT_user = 1;
  private static final byte SORT_sessionId = 2;
  private static final byte SORT_subjectType = 3;
  private static final byte SORT_subjectId = 4;
  private static final byte SORT_action = 5;
  private static final byte SORT_timestamp = 6;

  private static final Set nonAppliableSubjectTypes = new HashSet();
  static {
    nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_dl));
    nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_locale));
    nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_profile));
    nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_service));
    nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_logger));
    nonAppliableSubjectTypes.add(new Byte(SubjectTypes.TYPE_msc));
  }

  List actions = null;
  private String mbClearNonappliable = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (mbClearNonappliable != null)
      return clearNonappliable();

    queryJournalEntries();

    return result;
  }

  private void queryJournalEntries()
  {
    if (sort == null)
      sort = "timestamp";
    if (pageSize == 0)
      pageSize = 50;

    final byte sortField = getSortField(sort);
    final List allActions = appContext.getJournal().getActions();
    Collections.sort(allActions, new Comparator()
    {
      public int compare(Object o1, Object o2)
      {
        Action a1 = (Action) o1;
        Action a2 = (Action) o2;
        switch (sortField) {
          case SORT_timestamp:
            return a1.getTimestamp().compareTo(a2.getTimestamp());
          case SORT_user:
            return a1.getUser().compareTo(a2.getUser());
          case SORT_sessionId:
            return a1.getSessionId().compareTo(a2.getSessionId());
          case SORT_subjectType:
            return a1.getSubjectType() - a2.getSubjectType();
          case SORT_subjectId:
            return a1.getSubjectId().compareTo(a2.getSubjectId());
          case SORT_action:
            return a1.getAction() - a2.getAction();

          case -SORT_timestamp:
            return -a1.getTimestamp().compareTo(a2.getTimestamp());
          case -SORT_user:
            return -a1.getUser().compareTo(a2.getUser());
          case -SORT_sessionId:
            return -a1.getSessionId().compareTo(a2.getSessionId());
          case -SORT_subjectType:
            return -(a1.getSubjectType() - a2.getSubjectType());
          case -SORT_subjectId:
            return -a1.getSubjectId().compareTo(a2.getSubjectId());
          case -SORT_action:
            return -(a1.getAction() - a2.getAction());

          default:
            return a1.getTimestamp().compareTo(a2.getTimestamp());
        }
      }
    });
    totalSize = allActions.size();
    actions = allActions.subList(startPosition, Math.min(startPosition + pageSize, totalSize));
  }

  private int clearNonappliable()
  {
    appContext.getJournal().clear(nonAppliableSubjectTypes);
    return RESULT_DONE;
  }

  private static byte getSortField(String sortField)
  {
    boolean ascending = sortField == null || sortField.length() < 1 || sortField.charAt(0) != '-';
    String s = ascending ? sortField : sortField.substring(1);
    if ("timestamp".equals(s)) return ascending ? SORT_timestamp : -SORT_timestamp;
    if ("user".equals(s)) return ascending ? SORT_user : -SORT_user;
    if ("sessionId".equals(s)) return ascending ? SORT_sessionId : -SORT_sessionId;
    if ("subjectType".equals(s)) return ascending ? SORT_subjectType : -SORT_subjectType;
    if ("subjectId".equals(s)) return ascending ? SORT_subjectId : -SORT_subjectId;
    if ("action".equals(s)) return ascending ? SORT_action : -SORT_action;
    return SORT_timestamp;
  }

  public List getActions()
  {
    return actions;
  }

  public String getMbClearNonappliable()
  {
    return mbClearNonappliable;
  }

  public void setMbClearNonappliable(String mbClearNonappliable)
  {
    this.mbClearNonappliable = mbClearNonappliable;
  }
}
