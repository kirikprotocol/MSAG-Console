package ru.sibinco.mci.profile;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 10.09.2004
 * Time: 14:30:46
 * To change this template use File | Settings | File Templates.
 */
public class Constants
{
  public final static String BUNDLE_HELP    = "help";
  public final static String BUNDLE_SYSTEM  = "system";
  public final static String BUNDLE_DIVERT  = "divert";
  public final static String BUNDLE_PROFILE = "profile";

  public final static String PAGE_PREFIX    = "page_";
  public final static String VALUE_PREFIX   = "value_";

  public final static String PAGE_MAIN = PAGE_PREFIX + "main";
  public final static String PAGE_EXIT = PAGE_PREFIX + "exit";
  public final static String PAGE_INFO = PAGE_PREFIX + "info";
  public final static String PAGE_SET  = PAGE_PREFIX + "set";
  public final static String PAGE_MAN  = PAGE_PREFIX + "man";

  public final static String OFF        = "off";
  public final static String SERVICE    = "service";
  public final static String VOICEMAIL  = "voicemail";

  public final static String YES        = "yes";
  public final static String NO         = "no";
  public final static String INFORM     = "inform";
  public final static String NOTIFY     = "notify";

  public final static String VALUE_OFF        = VALUE_PREFIX + OFF;
  public final static String VALUE_SERVICE    = VALUE_PREFIX + SERVICE;
  public final static String VALUE_VOICEMAIL  = VALUE_PREFIX + VOICEMAIL;

  public final static String VALUE_YES        = VALUE_PREFIX + YES;
  public final static String VALUE_NO         = VALUE_PREFIX + NO;
  public final static String VALUE_INFORM     = VALUE_PREFIX + INFORM;
  public final static String VALUE_NOTIFY     = VALUE_PREFIX + NOTIFY;

  public final static String ATTR_MAIN    = "MAIN";
  public final static String ATTR_DIVERT  = "DIVERT";
  public final static String ATTR_PROFILE = "PROFILE";
  public final static String ATTR_REASON  = "REASON";
  public final static String ATTR_FORMATS = "FORMATS";

  public final static String OPTION_EXIT = "0";

  public final static String MCI_PROF_MTF_FILE    = "templates.properties";
  public final static String MCI_PROF_MSC_FILE    = "commutator.properties";
}
