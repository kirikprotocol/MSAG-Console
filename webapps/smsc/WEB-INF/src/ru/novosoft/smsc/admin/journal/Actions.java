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
  public static final byte ACTION_LOCK = 4;
  public static final byte ACTION_UNLOCK = 5;
  public static final byte ACTION_RESTORE = 6;
  public static final byte ACTION_LOAD = 7;

  public static final String actionToString(byte action)
  {
    switch (action) {
      case ACTION_ADD:
        return "add";
      case ACTION_DEL:
        return "delete";
      case ACTION_MODIFY:
        return "modify";
      case ACTION_LOCK:
        return "lock";
      case ACTION_UNLOCK:
        return "unlock";
      case ACTION_RESTORE:
        return "restore";
      case ACTION_LOAD:
        return "load";
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
    if ("lock".equals(action))
      return ACTION_LOCK;
    if ("unlock".equals(action))
      return ACTION_UNLOCK;
    if ("restore".equals(action))
      return ACTION_RESTORE;
    if ("load".equals(action))
      return ACTION_LOAD;
    return ACTION_UNKNOWN;
  }
}