package ru.novosoft.smsc.jsp.smsc.smsc_service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.Comparator_CaseInsensitive;
import ru.novosoft.smsc.util.SortedList;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: 22.05.2003
 * Time: 19:10:03
 */
public class Logging extends PageBean
{
  public class LoggerCategoryInfo implements Comparable
  {
    private String name;
    private String fullName;
    private String priority;
    private Map childs;

    public LoggerCategoryInfo(String name, String fullName, String priority)
    {
      this.name = name;
      this.fullName = fullName;
      this.priority = priority;
      this.childs = new TreeMap(new Comparator_CaseInsensitive());
    }

    public void addChild(String childFullName, String childPriority)
    {
      logger.debug("Adding child: name=\"" + fullName + "\" priority=" + priority + ",  childName=\"" + childFullName + "\" childPriority=" + childPriority);
      if (isRoot()) {
        final int dotPos = childFullName.indexOf('.');
        if (dotPos > 0) {
          final String childName = childFullName.substring(0, dotPos);
          LoggerCategoryInfo child = getOrCreateChild(childName, childName, "NOTSET");
          child.addChild(childFullName, childPriority);
        }
        else {
          childs.put(childFullName, new LoggerCategoryInfo(childFullName, childFullName, childPriority));
        }
      }
      else {
        if (childFullName.length() > fullName.length() && childFullName.startsWith(fullName) && childFullName.charAt(fullName.length()) == '.') {
          final int beginIndex = fullName.length() + 1;
          final int endIndex = childFullName.indexOf('.', beginIndex);
          final String childName = endIndex > 0 ? childFullName.substring(beginIndex, endIndex) : childFullName.substring(beginIndex);
          if (endIndex > 0) {
            LoggerCategoryInfo child = getOrCreateChild(childName, childFullName.substring(0, endIndex), "NOTSET");
            child.addChild(childFullName, childPriority);
          }
          else {
            LoggerCategoryInfo child = getOrCreateChild(childName, childFullName, childPriority);
            child.priority = childPriority;
          }
        }
        else {
          logger.debug("Incorrect LoggerCategoryInfo.addChild algorithm");
        }
      }
    }

    public boolean isRoot()
    {
      return fullName == null || fullName.length() == 0;
    }

    public boolean hasChilds()
    {
      return !childs.isEmpty();
    }

    private LoggerCategoryInfo getOrCreateChild(String childName, String childFullName, String childPriority)
    {
      LoggerCategoryInfo child = (LoggerCategoryInfo) childs.get(childName);
      if (child == null) {
        child = new LoggerCategoryInfo(childName, childFullName, childPriority);
        childs.put(childName, child);
      }
      return child;
    }

    public int compareTo(Object o)
    {
      if (o instanceof LoggerCategoryInfo) {
        LoggerCategoryInfo info = (LoggerCategoryInfo) o;
        return name.compareTo(info.name);
      }
      else
        return 0;
    }

    public String getName()
    {
      return name;
    }

    public String getFullName()
    {
      return fullName;
    }

    public String getPriority()
    {
      return priority;
    }

    public Map getChilds()
    {
      return childs;
    }
  }

  public final static String catParamNamePrefix = "category_";

  private String mbSave = null;
  private String mbCancel = null;
  private LoggerCategoryInfo rootCategory;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      Map logCategories = appContext.getSmsc().getLogCategories();
      String rootPriority = (String) logCategories.remove("");
      if (rootPriority == null) rootPriority = "NOTSET";
      rootCategory = new LoggerCategoryInfo("", "", rootPriority);
      Collection keys = new SortedList(logCategories.keySet());
      for (Iterator i = keys.iterator(); i.hasNext();) {
        String key = (String) i.next();
        String value = (String) logCategories.get(key);
        rootCategory.addChild(key, value);
      }
    } catch (AdminException e) {
      rootCategory = new LoggerCategoryInfo("", "", "NOTSET");
      return error(SMSCErrors.error.smsc.couldntGetLogCats, e);
    }
    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbSave != null)
      return save(request.getParameterMap());
    else if (mbCancel != null)
      return RESULT_DONE;

    return result;
  }

  private int save(Map parameters)
  {
    Map cats = new HashMap();
    for (Iterator i = parameters.entrySet().iterator(); i.hasNext();) {
      Map.Entry entry = (Map.Entry) i.next();
      String paramName = (String) entry.getKey();
      if (paramName.startsWith(catParamNamePrefix)) {
        cats.put(paramName.substring(catParamNamePrefix.length()), getParamValue(entry.getValue()));
        logger.info("cat   param: " + paramName + ":=" + getParamValue(entry.getValue()));
      }
      else {
        logger.info("WRONG param: " + paramName + ":=" + getParamValue(entry.getValue()));
      }
    }
    try {
      appContext.getSmsc().setLogCategories(cats);
      journalAppend(SubjectTypes.TYPE_logger, null, Actions.ACTION_MODIFY);
      return RESULT_DONE;
    } catch (AdminException e) {
      return error(SMSCErrors.error.smsc.couldntSetLogCats, e);
    }
  }

  private String getParamValue(Object value)
  {
    if (value instanceof String)
      return (String) value;
    else if (value instanceof String[]) {
      String result = "";
      final String[] values = (String[]) value;
      for (int i = 0; i < values.length; i++) {
        result += values[i];
      }
      return result;
    }
    else
      return value.toString();
  }

  public LoggerCategoryInfo getRootCategory()
  {
    return rootCategory;
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }
}
