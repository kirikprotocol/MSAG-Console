package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.admin.AdminException;

import java.security.Principal;
import java.util.List;
import java.io.IOException;

import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;


/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 2:07:44 PM
 */
public class Index extends InfoSmeBean {
  private String mbApplyAll = null;
  private String mbResetAll = null;

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbApplyAll != null)
      return applyAll();
    if (mbResetAll != null)
      return resetAll();

    return result;
  }

  private int resetAll()
  {
    try {
      getInfoSmeContext().resetConfig();
    } catch (Throwable e) {
      logger.debug("Couldn't reload InfoSME config", e);
      return error("Could not reload InfoSME config", e);
    }
    return RESULT_DONE;
  }

  private int applyAll()
  {
    try {
      logger.debug("Apply all...");
      getConfig().save();
    } catch (Throwable e) {
      logger.error("Couldn't save InfoSME config", e);
      return error("Could not save InfoSME config", e);
    }
    return RESULT_DONE;
  }

  public String getMbApplyAll()
  {
    return mbApplyAll;
  }

  public void setMbApplyAll(String mbApplyAll)
  {
    this.mbApplyAll = mbApplyAll;
  }

  public String getMbResetAll()
  {
    return mbResetAll;
  }

  public void setMbResetAll(String mbResetAll)
  {
    this.mbResetAll = mbResetAll;
  }
}
