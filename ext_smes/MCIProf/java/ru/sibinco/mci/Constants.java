package ru.sibinco.mci;

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
  public final static String ERROR_PREFIX   = "error_";

  public final static int RELEASE_PREFIXED_STRATEGY = 0x01;
  public final static int RELEASE_REDIRECT_STRATEGY = 0x02; // MTS default strategy
  public final static String PAGE_PREFIXED_STRATEGY = "pagePrefixedStrategy";
  public final static String PAGE_REDIRECT_STRATEGY = "pageRedirectStrategy";

  public final static String PARAM_SHORTCUT = "shortcut";

  public final static String PAGE_MAIN = PAGE_PREFIX + "main";
  public final static String PAGE_EXIT = PAGE_PREFIX + "exit";
  public final static String PAGE_INFO = PAGE_PREFIX + "info";
  public final static String PAGE_SET  = PAGE_PREFIX + "set";
  public final static String PAGE_MAN  = PAGE_PREFIX + "man";
  public final static String PAGE_ERR  = PAGE_PREFIX + "err";
  public final static String PAGE_SHORTCUTS = PAGE_PREFIX  + "shortcuts";
  public final static String PAGE_MAIN_ALT  = PAGE_MAIN + "_alt";
  public final static String PAGE_INFO_ALT  = PAGE_INFO + "_alt";
  public final static String PAGE_MAN_PREFIXED  = PAGE_MAN + "_prefixed";
  public final static String PAGE_MAN_REDIRECT  = PAGE_MAN + "_redirect";
  public final static String PAGE_MAN_SHORTCUTS = PAGE_MAN + "_shortcuts";
  public final static String PAGE_ERR_SHORTCUT      = PAGE_ERR + "_shortcut";
  public final static String PAGE_INFO_SHORTCUT     = PAGE_INFO     + "_shortcut";
  public final static String PAGE_INFO_ALT_SHORTCUT = PAGE_INFO_ALT + "_shortcut";

  public final static String OFF        = "off";
  public final static String SERVICE    = "service";
  public final static String VOICEMAIL  = "voicemail";

  public final static String YES        = "yes";
  public final static String NO         = "no";
  public final static String INFORM     = "inform";
  public final static String NOTIFY     = "notify";

  public final static String VALUE_OFF        = VALUE_PREFIX + OFF;
  public final static String VALUE_OFF_FULL   = VALUE_OFF + "_full";
  public final static String VALUE_SERVICE    = VALUE_PREFIX + SERVICE;
  public final static String VALUE_VOICEMAIL  = VALUE_PREFIX + VOICEMAIL;
  public final static String VALUE_YES        = VALUE_PREFIX + YES;
  public final static String VALUE_NO         = VALUE_PREFIX + NO;
  public final static String VALUE_INFORM     = VALUE_PREFIX + INFORM;
  public final static String VALUE_NOTIFY     = VALUE_PREFIX + NOTIFY;
  public final static String VALUE_ABSENT     = VALUE_PREFIX + "absent";
  public final static String VALUE_BUSY       = VALUE_PREFIX + "busy";
  public final static String VALUE_NOREPLY    = VALUE_PREFIX + "noreply";
  public final static String VALUE_UNCOND     = VALUE_PREFIX + "uncond";
  public final static String VALUE_CHANGED    = VALUE_PREFIX + "changed";

  public final static String ERROR_COMMUNICATION = ERROR_PREFIX + "communication";
  public final static String ERROR_NOT_ACCEPTED  = ERROR_PREFIX + "not_accepted";
  public final static String ERROR_CONNECT       = ERROR_PREFIX + "connect";
  public final static String ERROR_UNKNOWN       = ERROR_PREFIX + "unknown";
  public final static String ERROR_DB            = ERROR_PREFIX + "db";
  public final static String ERROR_DENIED        = ERROR_PREFIX + "denied";

  public final static String ATTR_MAIN     = "MAIN";
  public final static String ATTR_ERROR    = "ERROR";
  public final static String ATTR_DIVERT   = "DIVERT";
  public final static String ATTR_PROFILE  = "PROFILE";
  public final static String ATTR_REASON   = "REASON";
  public final static String ATTR_REASONS  = "REASONS";
  public final static String ATTR_FORMATS  = "FORMATS";
  public final static String ATTR_OPTIONS  = "OPTIONS";
  public final static String ATTR_STRATEGY = "STRATEGY";

  public final static String OPTION_EXIT  = "0";

  public final static String MCI_PROF_MTF_FILE    = "templates.properties";
  public final static String MCI_PROF_MSC_FILE    = "commutator.properties";
}
