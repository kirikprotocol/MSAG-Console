package ru.novosoft.smsc.admin.journal;


/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 04.11.2003
 * Time: 17:17:19
 */
public abstract class Actions
{
  public static final byte ACTION_UNKNOWN = 0;
  public static final byte ACTION_ADD = 1;
  public static final byte ACTION_DEL = 2;
  public static final byte ACTION_MODIFY = 3;

  public static final String actionToString(byte action)
  {
    switch (action) {
      case ACTION_ADD:
        return "add";
      case ACTION_DEL:
        return "delete";
      case ACTION_MODIFY:
        return "modify";
      default:
        return "unknown";
    }
  }

  public static final byte stringToAction(String action)
  {
    if ("add".equals(action))
      return ACTION_ADD;
    if ("delete".equals(action))
      return ACTION_DEL;
    if ("modify".equals(action))
      return ACTION_MODIFY;
    return ACTION_UNKNOWN;
  }
}