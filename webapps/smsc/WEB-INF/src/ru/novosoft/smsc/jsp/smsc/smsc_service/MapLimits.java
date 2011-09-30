package ru.novosoft.smsc.jsp.smsc.smsc_service;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.AdminException;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 20.03.2008
 */

public class MapLimits extends SmscBean {

  private static final Category logger = Category.getInstance(MapLimits.class);

  private String mbSave = null;
  private String mbReset = null;

  private String init;

  private String errorStr;

  private int dlglimitIn;
  private int dlglimitInsri;
  private int dlglimitUssd;
  private int dlglimitOutSri;
  private int dlglimitNiussd;

  private String ussdNo_sri_codes;
  private String ussdCond_sri_codes;
  private String ussdAti_codes;
  private String ussdOpenRespRealAddr;
  private String ussdParseAlways;
  private String ussdParseOnlyStar;
  private String ussdParseNever;
  private String ussdDefaultParsingMode;

  private Level[] levels = new Level[9];


  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if(init == null || init.length() == 0 || mbReset != null) {
      result = load();
      if (result != RESULT_OK)
        return result;
    }else {
      processLevels(request);
    }

    if (mbSave != null)
      return save(appContext);
    else
      return RESULT_OK;
  }

  public int load() {
    Config config = appContext.getSmsc().getMapLimitsConfig();
    if (config == null)
      return error(SMSCErrors.error.smsc.couldntGetConfig);

    dlglimitIn = getInt("dlglimit.in", config);
    dlglimitInsri = getInt("dlglimit.insri", config);
    dlglimitUssd = getInt("dlglimit.ussd", config);
    dlglimitOutSri = getInt("dlglimit.outsri", config);
    dlglimitNiussd = getInt("dlglimit.niussd", config);


    ussdNo_sri_codes = getString("ussd.no_sri_codes",config);
    ussdCond_sri_codes = getString("ussd.cond_sri_codes",config);
    ussdAti_codes = getString("ussd.ati_codes",config);
    ussdOpenRespRealAddr = getString("ussd.openRespRealAddr",config);
    ussdParseAlways = getString("ussd.parseAlways",config);
    ussdParseOnlyStar = getString("ussd.parseOnlyStar",config);
    ussdParseNever = getString("ussd.parseNever",config);
    ussdDefaultParsingMode = getString("ussd.defaultParsingMode",config);

    for(int i=1;i<=8;i++) {
      Level l = new Level(i);
      l.setDialogsLimit(getInt("clevels.level"+i+".dialogsLimit", config));
      l.setFailLowerLimit(getInt("clevels.level" + i + ".failLowerLimit", config));
      l.setFailUpperLimit(getInt("clevels.level" + i + ".failUpperLimit", config));
      l.setOkToLower(getInt("clevels.level"+i+".okToLower", config));
      levels[i] = l;
    }

    return RESULT_OK;
  }

  private void processLevels(HttpServletRequest request) {
    for(int i=1;i<=8;i++) {
      Level l = new Level(i);
      l.load(request);
      levels[i] = l;
    }
  }

  private int getInt(String param, Config c) {
    try {
      return c.getInt(param);
    } catch (Config.ParamNotFoundException e) {
      return 0;
    } catch (Config.WrongParamTypeException e) {
      logger.error("parameter \"" + param + "\" is not integer.");
      return -1;
    }
  }

  private String getString(String param, Config c) {
    try {
      return c.getString(param);
    } catch (Config.ParamNotFoundException e) {
      return "";
    } catch (Config.WrongParamTypeException e) {
      logger.error("parameter \"" + param + "\" is not string.");
      return null;
    }
  }

  public String getInit() {
    return init;
  }

  public void setInit(String init) {
    this.init = init;
  }

  private int save(SMSCAppContext appContext) {
    if(errorStr != null) {
      return error(errorStr);
    }
    Config c = appContext.getSmsc().getMapLimitsConfig();
    if (c == null)
      return error(SMSCErrors.error.smsc.couldntGetConfig);

    c.setInt("dlglimit.in", dlglimitIn);
    c.setInt("dlglimit.insri", dlglimitInsri);
    c.setInt("dlglimit.ussd", dlglimitUssd);
    c.setInt("dlglimit.outsri", dlglimitOutSri);
    c.setInt("dlglimit.niussd", dlglimitNiussd);

    if(ussdNo_sri_codes != null) {
      c.setString("ussd.no_sri_codes", ussdNo_sri_codes);
    }
    if(ussdCond_sri_codes != null) {
      c.setString("ussd.cond_sri_codes", ussdCond_sri_codes);
    }
    if(ussdAti_codes != null) {
      c.setString("ussd.ati_codes", ussdAti_codes);
    }
    if(ussdOpenRespRealAddr != null) {
      c.setString("ussd.openRespRealAddr", ussdOpenRespRealAddr);
    }
    if(ussdParseAlways != null) {
      c.setString("ussd.parseAlways", ussdParseAlways);
    }
    if(ussdParseOnlyStar != null) {
      c.setString("ussd.parseOnlyStar", ussdParseOnlyStar);
    }
    if(ussdParseNever != null) {
      c.setString("ussd.parseNever", ussdParseNever);
    }
    if(ussdDefaultParsingMode != null) {
      c.setString("ussd.defaultParsingMode", ussdDefaultParsingMode);
    }


    for(int i=1;i<=8;i++) {
      Level l = levels[i];
      c.setInt("clevels.level"+i+".dialogsLimit", l.getDialogsLimit());
      c.setInt("clevels.level" + i + ".failLowerLimit", l.getFailLowerLimit());
      c.setInt("clevels.level" + i + ".failUpperLimit", l.getFailUpperLimit());
      c.setInt("clevels.level"+i+".okToLower", l.getOkToLower());
    }

    try {
      appContext.getSmsc().saveMapLimitsConfig(c);
      journalAppend(SubjectTypes.TYPE_maplimits, null, Actions.ACTION_MODIFY);
    } catch (AdminException e) {
      logger.error("Couldn't save MAP limits config", e);
      return error(SMSCErrors.error.smsc.couldntSave, e.getMessage());
    }

    try {
      appContext.getSmsc().applyMapLimitsConfig();
    } catch (AdminException e) {
      logger.error("Couldn't apply MAP limits config", e);
      return error(SMSCErrors.error.smsc.couldntApply, e.getMessage());
    }
    return RESULT_OK;
  }

  public String getDlglimitIn() {
    return Integer.toString(dlglimitIn);
  }

  public void setDlglimitIn(String dlglimitIn) {
    if(dlglimitIn != null && (dlglimitIn = dlglimitIn.trim()).length()>0) {
      try{
        this.dlglimitIn = Integer.parseInt(dlglimitIn);
      }catch (NumberFormatException e) {
        errorStr = "Illegal integer parameter: "+dlglimitIn;
        logger.error(e,e);
      }
    }
  }

  public String getDlglimitInsri() {
    return Integer.toString(dlglimitInsri);
  }

  public void setDlglimitInsri(String dlglimitInsri) {
    if(dlglimitInsri != null && (dlglimitInsri = dlglimitInsri.trim()).length()>0) {
      try{
        this.dlglimitInsri = Integer.parseInt(dlglimitInsri);
      }catch (NumberFormatException e) {
        errorStr = "Illegal integer parameter: "+dlglimitInsri;
        logger.error(e,e);
      }
    }
  }

  public String getDlglimitUssd() {
    return Integer.toString(dlglimitUssd);
  }

  public void setDlglimitUssd(String dlglimitUssd) {
    if(dlglimitUssd != null && (dlglimitUssd = dlglimitUssd.trim()).length()>0) {
      try{
        this.dlglimitUssd = Integer.parseInt(dlglimitUssd);
      }catch (NumberFormatException e) {
        errorStr = "Illegal integer parameter: "+dlglimitUssd;
        logger.error(e,e);
      }
    }
  }

  public String getDlglimitOutSri() {
    return Integer.toString(dlglimitOutSri);
  }

  public void setDlglimitOutSri(String dlglimitOutSri) {
    if(dlglimitOutSri != null && (dlglimitOutSri = dlglimitOutSri.trim()).length()>0) {
      try{
        this.dlglimitOutSri = Integer.parseInt(dlglimitOutSri);
      }catch (NumberFormatException e) {
        errorStr = "Illegal integer parameter: "+dlglimitOutSri;
        logger.error(e,e);
      }
    }
  }

  public String getDlglimitNiussd() {
    return Integer.toString(dlglimitNiussd);
  }

  public void setDlglimitNiussd(String dlglimitNiussd) {
    if(dlglimitNiussd != null && (dlglimitNiussd = dlglimitNiussd.trim()).length()>0) {
      try{
        this.dlglimitNiussd = Integer.parseInt(dlglimitNiussd);
      }catch (NumberFormatException e) {
        errorStr = "Illegal integer parameter: "+dlglimitNiussd;
        logger.error(e,e);
      }
    }
  }

  public String getUssdNo_sri_codes() {
    return ussdNo_sri_codes;
  }

  public void setUssdNo_sri_codes(String ussdNo_sri_codes) {
    this.ussdNo_sri_codes = trim(ussdNo_sri_codes);
  }

  public String getUssdCond_sri_codes() {
    return ussdCond_sri_codes;
  }

  public void setUssdCond_sri_codes(String ussdCond_sri_codes) {
    this.ussdCond_sri_codes = trim(ussdCond_sri_codes);
  }

  public String getUssdAti_codes() {
    return ussdAti_codes;
  }

  public void setUssdAti_codes(String ussdAti_codes) {
    this.ussdAti_codes = trim(ussdAti_codes);
  }

  public String getUssdOpenRespRealAddr() {
    return ussdOpenRespRealAddr;
  }

  public void setUssdOpenRespRealAddr(String ussdOpenRespRealAddr) {
    this.ussdOpenRespRealAddr = trim(ussdOpenRespRealAddr);
  }

  public String getUssdParseAlways() {
    return ussdParseAlways;
  }

  public void setUssdParseAlways(String ussdParseAlways) {
    this.ussdParseAlways = trim(ussdParseAlways);
  }

  public String getUssdParseOnlyStar() {
    return ussdParseOnlyStar;
  }

  public void setUssdParseOnlyStar(String ussdParseOnlyStar) {
    this.ussdParseOnlyStar = trim(ussdParseOnlyStar);
  }

  public String getUssdParseNever() {
    return ussdParseNever;
  }

  public void setUssdParseNever(String ussdParseNever) {
    this.ussdParseNever = trim(ussdParseNever);
  }

  public String getUssdDefaultParsingMode() {
    return ussdDefaultParsingMode;
  }

  public void setUssdDefaultParsingMode(String ussdDefaultParsingMode) {
    this.ussdDefaultParsingMode = trim(ussdDefaultParsingMode);
  }

  public Level[] getLevels() {
    return levels;
  }

  private static String trim(String s) {
    return  s == null ? null : s.trim();
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbReset()
  {
    return mbReset;
  }

  public void setMbReset(String mbReset)
  {
    this.mbReset = mbReset;
  }

  public class Level {


    private final int level;
    private int dialogsLimit;
    private int failUpperLimit;
    private int failLowerLimit;
    private int okToLower;

    private final String dialogsLimitName;
    private final String failUpperLimitName;
    private final String failLowerLimitName;
    private final String okToLowerName;

    public Level(int level) {
      this.level = level;
      this.dialogsLimitName = "dialogsLimitName"+level;
      this.failUpperLimitName = "failUpperLimitName"+level;
      this.failLowerLimitName = "failLowerLimitName"+level;
      this.okToLowerName = "okToLowerName"+level;
    }

    public String getDialogsLimitName() {
      return dialogsLimitName;
    }

    public String getFailUpperLimitName() {
      return failUpperLimitName;
    }

    public String getFailLowerLimitName() {
      return failLowerLimitName;
    }

    public String getOkToLowerName() {
      return okToLowerName;
    }

    public int getLevel() {
      return level;
    }

    public int getDialogsLimit() {
      return dialogsLimit;
    }

    private void setDialogsLimit(int dialogsLimit) {
      this.dialogsLimit = dialogsLimit;
    }

    public int getFailUpperLimit() {
      return failUpperLimit;
    }

    private void setFailUpperLimit(int failUpperLimit) {
      this.failUpperLimit = failUpperLimit;
    }

    public int getFailLowerLimit() {
      return failLowerLimit;
    }

    private void setFailLowerLimit(int failLowerLimit) {
      this.failLowerLimit = failLowerLimit;
    }

    public int getOkToLower() {
      return okToLower;
    }

    private void setOkToLower(int okToLower) {
      this.okToLower = okToLower;
    }

    private void load(HttpServletRequest request) {
      String s = null;
      try{
        s = request.getParameter(dialogsLimitName);
        if(s != null && (s = s.trim()).length()>0) {
          dialogsLimit = Integer.parseInt(s);
        }

        s = request.getParameter(failLowerLimitName);

        if(s != null && (s = s.trim()).length()>0) {
          failLowerLimit = Integer.parseInt(s);
        }
        s = request.getParameter(failUpperLimitName);

        if(s != null && (s = s.trim()).length()>0) {
          failUpperLimit = Integer.parseInt(s);
        }
        s = request.getParameter(okToLowerName);

        if(s != null && (s = s.trim()).length()>0) {
          okToLower = Integer.parseInt(s);
        }
      }catch (NumberFormatException e) {
        errorStr = "Illegal integer parameter: "+s;
        logger.error(e,e);
      }

    }

  }
}
