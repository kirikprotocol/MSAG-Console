package mobi.eyeline.welcomesms.beans;

import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

import mobi.eyeline.welcomesms.WelcomeSMSContext;

/**
 * author: alkhal
 */
public class WelcomeSMSBean extends PageBean {

  public static final int RESULT_OPTIONS = PageBean.PRIVATE_RESULT;
  public static final int RESULT_NETWORKS = PageBean.PRIVATE_RESULT + 1;
  public static final int RESULT_ZONES = PageBean.PRIVATE_RESULT + 2;
  public static final int RESULT_UPLOAD = PageBean.PRIVATE_RESULT + 3;
  protected static final int PRIVATE_RESULT = PageBean.PRIVATE_RESULT + 4;

  protected WelcomeSMSContext welcomeSMSContext;
  protected Config config;

  protected String mbMenu = null;


  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      welcomeSMSContext = WelcomeSMSContext.getInstance(appContext);
      config = welcomeSMSContext.getConfig();
    } catch (Throwable e) {
      logger.error("Couldn't get Quiz config", e);
      return error("infosme.error.config_load", e);
    }

    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;
    if ("options".equals(mbMenu)) result = RESULT_OPTIONS;
    else if ("networks".equals(mbMenu)) result = RESULT_NETWORKS;
    else if ("zones".equals(mbMenu)) result = RESULT_ZONES;
    else if ("upload".equals(mbMenu)) result = RESULT_UPLOAD;

    mbMenu = null;
    return result;
  }


  protected Config getConfig() {
    return config;
  }

  public WelcomeSMSContext getWelcomeSMSContext() {
    return welcomeSMSContext;
  }

  public String getMbMenu() {
    return mbMenu;
  }

  public void setMbMenu(String mbMenu) {
    this.mbMenu = mbMenu;
  }
}
