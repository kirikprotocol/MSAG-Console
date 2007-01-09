package ru.novosoft.smsc.jsp.util;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 09.01.2007
 */
public class SessionContentManager {

  public static void putContentIntoSession(HttpServletRequest request, SessionItemId itemId, Class classId, Object content) {
    if (request == null || itemId == null || classId == null)
      return;

    request.getSession().setAttribute(itemId.getId(), new SessionContentItem(classId, content));
  }

  public static Object getContentFromSession(HttpServletRequest request, SessionItemId itemId, Class classId) {
    if (request == null || itemId == null || classId == null)
      return null;

    final SessionContentItem item = (SessionContentItem)request.getSession().getAttribute(itemId.getId());
    return (item != null && item.getClassId().equals(classId))  ? item.getContent() : null;
  }

  public static void removeContentFromSession(HttpServletRequest request, SessionItemId itemId, Class classId) {
    if (request == null || itemId == null || classId == null)
      return;

    final SessionContentItem item = (SessionContentItem)request.getSession().getAttribute(itemId.getId());
    if (item != null && item.getClassId().equals(classId))
      request.getSession().removeAttribute(itemId.getId());
  }


  public static class SessionItemId {

    public static final SessionItemId BEAN_TEMPORARY_CONTENT = new SessionItemId("BEAN_TEMPORARY_CONTENT");


    private final String id;

    private SessionItemId(String id) {
      this.id = id;
    }

    public String getId() {
      return id;
    }
  }

  private static class SessionContentItem {
    private final Class classId;
    private final Object content;

    public SessionContentItem(Class classId, Object content) {
      this.classId = classId;
      this.content = content;
    }

    public Class getClassId() {
      return classId;
    }

    public Object getContent() {
      return content;
    }
  }
}
