package ru.sibinco.mci.profile;

import java.util.Hashtable;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 07.09.2004
 * Time: 13:17:08
 * To change this template use File | Settings | File Templates.
 */
public class DivertScenarioConstants
{
  public final static String REASON_PREFIX  = "reason_";
  public final static String VALUE_PREFIX   = "value_";
  public final static String ERROR_PREFIX   = "error_";

  public final static String BUSY     = "busy";
  public final static String ABSENT   = "absent";
  public final static String NOTAVAIL = "notavail";
  public final static String UNCOND   = "uncond";

  public final static String REASON_BUSY     = REASON_PREFIX + BUSY;
  public final static String REASON_ABSENT   = REASON_PREFIX + ABSENT;
  public final static String REASON_NOTAVAIL = REASON_PREFIX + NOTAVAIL;
  public final static String REASON_UNCOND   = REASON_PREFIX + UNCOND;

  public final static String TAG_SET        = "set";
  public final static String TAG_INFO       = "info";
  public final static String TAG_DIVERT     = "divert";

  public final static String ATTR_TYPE      = "type";

  protected final static Hashtable reasonsMapping = new Hashtable();
  static {
    reasonsMapping.put("1", BUSY);
    reasonsMapping.put("2", ABSENT);
    reasonsMapping.put("3", NOTAVAIL);
    reasonsMapping.put("4", UNCOND);
  };

  public final static String getReason(String key) {
    return ((key == null) ? null:(String)reasonsMapping.get(key.trim()));
  }
}
