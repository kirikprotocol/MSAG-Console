package ru.novosoft.smsc.jsp.smsc.reshedule;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import java.util.*;

/**
 * Created by igork
 * Date: Aug 26, 2003
 * Time: 3:03:54 PM
 */
class Reshedules
{
  private static final String RESHEDULE_TABLE_PREFIX = "core.reshedule table";
  private static final String RESHEDULE_DEFAULT = "core.reschedule_table";
  private static final String ERR_CODES_PREFIX = "smsc.errcode.";
  private static final String ERR_CODE_UNKNOWN = ERR_CODES_PREFIX + "unknown";
  public static final String DEFAULT_RESHEDULE_NAME = "DEFAULT_RESHEDULE_NAME";
/*  public Set err_codes_all = new HashSet();
  {
    "8", "20", "69", "88", "100", "255", "1027", "1028", "1136", "1139", "1143", "1157", "1179", "1183", "1184", "1186"
    0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 17, 19, 20, 21, 51, 52, 64, 66, 67, 68, 69, 72, 73,
    80, 81, 83, 84, 85, 88, 97, 98, 99, 100, 101, 102, 103, 192, 193, 194, 195, 196, 254, 255, 260,
    1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1134, 1136, 1137, 1138, 1139, 1140, 1141, 1142, 1143,
    1144, 1145, 1146, 1147, 1148, 1149, 1150, 1151, 1153, 1154, 1155, 1157, 1158, 1160, 1161, 1163, 1164,
    1165, 1173, 1179, 1183, 1184, 1185, 1186, 1187, 1188

  };
*/

  private static Reshedules instance = null;

  public static Reshedules getInstance(SMSCAppContext appContext) throws Throwable
  {
    return instance == null ? instance = new Reshedules(appContext) : instance;
  }

  private Category logger = Category.getInstance(this.getClass());
  private SMSCAppContext appContext = null;
  private Config config = null;
  private Set assignedErrorsSet = new HashSet();

  public Reshedules(SMSCAppContext appContext) throws Throwable
  {
    try {
      this.appContext = appContext;
      config = appContext.getSmsc().getSmscConfig();
      recalculateAssignedErrorsSet();
    } catch (Throwable e) {
      logger.debug("Couldn't get smsc config", e);
      throw e;
    }
  }

  public Collection getShedules()
  {
    if (config == null) {
      logger.debug("getErrCodes: not initialized");
      return new LinkedList();
    }
    return new SortedList(config.getSectionChildShortParamsNames(RESHEDULE_TABLE_PREFIX));
  }

  public List getErrCodes(String shedule) throws AdminException
  {
    List result = new LinkedList();
    if (config == null) {
      logger.debug("getErrCodes: not initialized");
      return result;
    }
    String sheduleString = null;
    try {
      sheduleString = config.getString(RESHEDULE_TABLE_PREFIX + '.' + shedule);
    } catch (Config.ParamNotFoundException e) {
      return result;
    } catch (Config.WrongParamTypeException e) {
      throw new AdminException("Incorrect config file");
    }
    if (sheduleString != null) {
      for (StringTokenizer tokenizer = new StringTokenizer(StringEncoderDecoder.decodeDot(sheduleString), ",", false); tokenizer.hasMoreTokens();) {
        result.add(tokenizer.nextToken().trim());
      }
    }
    return result;
  }


  public void putShedule(String shedule, Collection errCodes) throws AdminException
  {
    if (config == null) {
      logger.debug("putShedule: not initialized");
      return;
    }
    StringBuffer errCodesString = new StringBuffer();
    for (Iterator i = errCodes.iterator(); i.hasNext();) {
      String errCode = (String) i.next();
      errCodesString.append(StringEncoderDecoder.encodeDot(errCode));
      if (i.hasNext())
        errCodesString.append(',');
    }
    config.setString(RESHEDULE_TABLE_PREFIX + '.' + shedule, errCodesString.toString());
    recalculateAssignedErrorsSet();
  }

  public void removeShedule(String shedule)
  {
    if (config == null) {
      logger.debug("removeShedule: not initialized");
      return;
    }
    config.removeParam(RESHEDULE_TABLE_PREFIX + '.' + shedule);
    recalculateAssignedErrorsSet();
  }

  public void save() throws AdminException
  {
    Config currentConfig = appContext.getSmsc().getSmscConfig();
    currentConfig.removeSection(RESHEDULE_TABLE_PREFIX);
    for (Iterator i = config.getSectionChildParamsNames(RESHEDULE_TABLE_PREFIX).iterator(); i.hasNext();) {
      String fullSheduleName = (String) i.next();
      try {
        currentConfig.setString(fullSheduleName, config.getString(fullSheduleName));
      } catch (Config.ParamNotFoundException e) {
        logger.debug("Impossible: parameter returned by Config.getSectionChildShortParamsNames(...) not found...");
      } catch (Config.WrongParamTypeException e) {
        logger.error("Misformatted reshedule table: param \"" + fullSheduleName + "\" is not string, skipped");
      }
    }
    appContext.getSmsc().saveSmscConfig(config = currentConfig);
    appContext.getStatuses().setSmscChanged(true);
  }

  public String getErrorString(Locale locale, String errorCode)
  {
    String result = appContext.getLocaleString(locale, ERR_CODES_PREFIX + errorCode);
    if (result == null) {
      result = appContext.getLocaleString(locale, ERR_CODE_UNKNOWN);
      if (result == null)
        result = "unknown";
    }
    return result;
  }

  protected boolean isShedulePresent(String shedule)
  {
    return config.getParameter(RESHEDULE_TABLE_PREFIX + '.' + shedule) != null;
  }

  public void reset()
  {
    config = appContext.getSmsc().getSmscConfig();
    recalculateAssignedErrorsSet();
  }

  private void recalculateAssignedErrorsSet()
  {
    assignedErrorsSet.clear();
    for (Iterator i = getShedules().iterator(); i.hasNext();) {
      String shedule = (String) i.next();
      try {
        assignedErrorsSet.addAll(getErrCodes(shedule));
      } catch (AdminException e) {
        // skip it
      }
    }
  }

  public boolean isErrorAssigned(String errorCode)
  {
    return assignedErrorsSet.contains(errorCode);
  }

  public boolean isAllErrorsAssigned(Locale locale)
  {
    return assignedErrorsSet.size() == getAllErrorCodes(locale).size();
  }

  public String getDefaultReshedule()
  {
    try {
      logger.debug("getDef: " + config.getString(RESHEDULE_DEFAULT));
      return config.getString(RESHEDULE_DEFAULT);
    } catch (Config.ParamNotFoundException e) {
      return "";
    } catch (Config.WrongParamTypeException e) {
      logger.error("default reshedule \"" + RESHEDULE_DEFAULT + "\" is not string");
      return "";
    }
  }

  public void setDefaultReshedule(String defaultReshedule)
  {
    config.setString(RESHEDULE_DEFAULT, defaultReshedule);
  }

  public Set getAllErrorCodes(Locale locale)
  {
    return appContext.getLocaleStrings(locale, ERR_CODES_PREFIX);
  }
}
