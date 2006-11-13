package ru.novosoft.smsc.jsp.smsc.smscenters;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.io.File;
import java.util.*;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 09.11.2006
 */

public class SMSCenters {
  private static final Category logger = Category.getInstance(SMSCenters.class);

  private static final String SMS_CENTERS_CONFIG = "core.smscenters_config";
  public static final String SMS_CENTERS_SUBJECTS_PREFIX = "subjects";
  public static final String SMS_CENTERS_MASKS_PREFIX = "masks";
  private static final char DOT_REPLACER = '^';

  private final Config config;

  private static SMSCenters instance = null;

  public static SMSCenters getInstance(SMSCAppContext appContext) throws Throwable {
    if (instance == null)
      instance = new SMSCenters(appContext);
    return instance;
  }

  private SMSCenters(SMSCAppContext appContext) throws Throwable {
    try {
      config = getSMSCentersConfig(appContext);
      if (config == null)
        throw new AdminException(SMSCErrors.error.smsc.smscenters.couldntGetSMSCentersConfig);

    } catch (Throwable e) {
      logger.debug("Couldn't get sms centers config", e);
      throw e;
    }
  }

  private Collection getItems(String key) {
    if (config == null) return new LinkedList();

    Collection c = config.getSectionChildShortParamsNames(key);
    SortedList result = new SortedList();
    if (c != null)
      for (Iterator i = c.iterator(); i.hasNext();) {
        String s = (String) i.next();
        if (s.indexOf(DOT_REPLACER) != -1)
          s = s.replace(DOT_REPLACER, '.');
        result.add(s);
      }

    return result;
  }

  private Config getSMSCentersConfig(SMSCAppContext appContext) throws Throwable {
    Config smscConfig = appContext.getSmsc().getSmscConfig();
    if (smscConfig == null)
      throw new AdminException(SMSCErrors.error.smsc.couldntGetConfig);

    String configName = smscConfig.getString(SMS_CENTERS_CONFIG);
    File confFile = new File(configName);
    return new Config(confFile);
  }

  public void save() throws Exception {
    config.save();
  }

  public Collection getMasks() {
    return getItems(SMS_CENTERS_MASKS_PREFIX);
  }

  public String getMasksSMSCNumber(String mask) {
    return (String)config.getParameter(SMS_CENTERS_MASKS_PREFIX + "." + mask.replace('.', DOT_REPLACER));
  }

  public String getRoutesSMSCNumber(String route) {
    return (String)config.getParameter(SMS_CENTERS_SUBJECTS_PREFIX + "." + route.replace('.', DOT_REPLACER));
  }

  public Collection getRoutes() {
    return getItems(SMS_CENTERS_SUBJECTS_PREFIX);
  }

  public void addMask(final String mask, final String smscNumber) {
    config.setString(SMS_CENTERS_MASKS_PREFIX + "." + mask.replace('.', DOT_REPLACER), smscNumber);
  }

  public void addSubject(final String name, final String smscNumber) {
    config.setString(SMS_CENTERS_SUBJECTS_PREFIX + "." + name.replace('.', DOT_REPLACER), smscNumber);
  }

  public void removeMask(final String mask) {
    config.removeParam(SMS_CENTERS_MASKS_PREFIX + "." + mask.replace('.', DOT_REPLACER));
  }

  public void removeSubject(final String name) {
    config.removeParam(SMS_CENTERS_SUBJECTS_PREFIX + "." + name.replace('.', DOT_REPLACER));
  }
}
