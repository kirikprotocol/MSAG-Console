/**
 * Created by igork
 * Date: Dec 5, 2002
 * Time: 5:47:27 PM
 */
package ru.novosoft.smsc.admin;

public class Constants
{
  public static final int ROUTE_ID_MAXLENGTH = 32;
  public static final String SMSC_SME_ID_PREFIX = "SMSC.";
  public static String SMSC_SME_ID = "SMSC";

  public static final String TomcatRealmName = "SMSC.SmscRealm";
  public static final long ServicesRefreshTimeoutMillis = 1000;

  public static final int MAX_PRIORITY = 0x7FFF;
  public static final String SMSC_ERROR_MESSAGES_ATTRIBUTE_NAME = "SMSC_ERROR_MESSAGES_ATTRIBUTE";

  public static final String ARCHIVE_DAEMON_SVC_ID = "ArchiveDaemon";

  public static final String CONSOLE_SESSION_ID = "Console Session";
  public static final long Day = 86400000;
  public static final long Hour = 3600000;
}
