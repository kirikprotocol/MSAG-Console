package ru.sibinco.mci.profile;

import java.util.Hashtable;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 09.09.2004
 * Time: 15:08:26
 * To change this template use File | Settings | File Templates.
 */
public class ProfileScenarioConstants
{
  public final static String TAG_PROFILE      = "profile";

  public final static String TAG_INFO          = "info";
  public final static String TAG_CHANGE        = "change";
  public final static String TAG_FLAGS         = "flags";
  public final static String TAG_SELECT_FORMAT = "select_format";
  public final static String TAG_SHOW_FORMAT   = "show_format";

  public final static String VALUE_PREFIX     = "value_";
  public final static String ERROR_PREFIX     = "error_";
  public final static String NO_SUFFIX        = "no_";

  public final static String INFORM           = "inform";
  public final static String NO_INFORM        = NO_SUFFIX + INFORM;
  public final static String NOTIFY           = "notify";
  public final static String NO_NOTIFY        = NO_SUFFIX + NOTIFY;

  public final static String VALUE_INFORM     = VALUE_PREFIX + INFORM;
  public final static String VALUE_NO_INFORM  = VALUE_PREFIX + NO_INFORM;
  public final static String VALUE_NOTIFY     = VALUE_PREFIX + NOTIFY;
  public final static String VALUE_NO_NOTIFY  = VALUE_PREFIX + NO_NOTIFY;

  public final static String VALUE_GRANTED       = VALUE_PREFIX + "granted";
  public final static String VALUE_DENIED        = VALUE_PREFIX + "denied";
  public final static String VALUE_MESSAGES      = VALUE_PREFIX + "messages";
  public final static String VALUE_NOTIFICATIONS = VALUE_PREFIX + "notifications";
  public final static String VALUE_FOR_ABONENT   = VALUE_PREFIX + "for_abonent";
  public final static String VALUE_FOR_CALLERS   = VALUE_PREFIX + "for_callers";

  public final static String ATTR_TYPE        = "type";
  public final static String ATTR_PROFILE     = "profile";

  protected final static Hashtable optionsMapping = new Hashtable();
  static {
    optionsMapping.put("1", INFORM);
    optionsMapping.put("2", NOTIFY);
    optionsMapping.put("3", INFORM);
    optionsMapping.put("4", NOTIFY);
  };

  public final static String getOption(String key) {
    return ((key == null) ? null:(String)optionsMapping.get(key.trim()));
  }

}
