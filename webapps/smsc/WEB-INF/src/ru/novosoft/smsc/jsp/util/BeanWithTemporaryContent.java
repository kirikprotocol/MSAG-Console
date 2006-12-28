package ru.novosoft.smsc.jsp.util;

import ru.novosoft.smsc.jsp.PageBean;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 27.12.2006
 */
public abstract class BeanWithTemporaryContent extends PageBean {

  private BeanTemporaryContent content = null;

  protected abstract String getBeanId();

  protected abstract int processRequest(HttpServletRequest request);

  protected AttributeName getSessionAttributeName() {
    return AttributeName.BEAN_TEMPORARY_CONTENT;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    content = (BeanTemporaryContent)request.getSession().getAttribute(getSessionAttributeName().getName());
    if (content != null && !content.getName().equals(getBeanId()))
      content = null;

    result = processRequest(request);
    if (result != RESULT_OK)
      return result;

    request.getSession().setAttribute(getSessionAttributeName().getName(), content);

    return result;
  }

  protected boolean hasTemporaryContent() {
    return content != null;
  }

  private BeanTemporaryContent getContent() {
    return (content == null) ? content = new BeanTemporaryContent(getBeanId()) : content;
  }

  protected void addToTemporaryContent(Object key, Object value) {
    getContent().addAttribute(key, value);
  }

  protected Object getFromTemporaryContent(Object key) {
    return (content == null) ? null : content.getAttribute(key);
  }

  public static class AttributeName {

    public static final AttributeName BEAN_TEMPORARY_CONTENT = new AttributeName("BEAN_TEMPORARY_CONTENT");

    private final String name;

    private AttributeName(String name) {
      this.name = name;
    }

    String getName() {
      return name;
    }
  }
}
