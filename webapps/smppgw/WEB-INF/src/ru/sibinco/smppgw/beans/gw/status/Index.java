package ru.sibinco.smppgw.beans.gw.status;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.beans.SmppgwBean;
import ru.sibinco.smppgw.beans.SmppgwJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;


/**
 * Created by IntelliJ IDEA. User: igork Date: 03.03.2004 Time: 18:39:37
 */
public class Index extends SmppgwBean
{
  private String mbApply;
  private String mbRestore;
  private String[] subj;

  public void process(HttpServletRequest request, HttpServletResponse response) throws SmppgwJspException
  {
    super.process(request, response);
    if (mbRestore != null)
      restore();
    else if (mbApply != null)
      apply();
  }

  private void restore()
  {
    if (subj != null && subj.length > 0)
      for (int i = 0; i < subj.length; i++) {
        String s = subj[i];
        if ("config".equals(s))
          restoreConfig();
      }
  }

  private void restoreConfig()
  {
  }

  private void apply() throws SmppgwJspException
  {
    if (subj != null && subj.length > 0)
      for (int i = 0; i < subj.length; i++) {
        String s = subj[i];
        if ("config".equals(s))
          applyConfig();
      }
  }

  private void applyConfig() throws SmppgwJspException
  {
    try {
      appContext.getGwConfig().save();
      appContext.getGateway().apply();
      appContext.getStatuses().setConfigChanged(false);
    } catch (SibincoException e) {
      logger.debug("Couldn't apply config", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_CONFIG, e);
    } catch (Config.WrongParamTypeException e) {
      logger.debug("Couldn't save config", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
    } catch (IOException e) {
      logger.debug("Couldn't save config", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
    }
  }

  public String getMbApply()
  {
    return mbApply;
  }

  public void setMbApply(String mbApply)
  {
    this.mbApply = mbApply;
  }

  public String getMbRestore()
  {
    return mbRestore;
  }

  public void setMbRestore(String mbRestore)
  {
    this.mbRestore = mbRestore;
  }

  public String[] getSubj()
  {
    return subj;
  }

  public void setSubj(String[] subj)
  {
    this.subj = subj;
  }

  public boolean isConfigChanged()
  {
    return appContext.getStatuses().isConfigChanged();
  }
}