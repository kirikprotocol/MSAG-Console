package mobi.eyeline.informer.web.controllers.cdr;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.cdr.CdrSettings;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class CdrController extends InformerController{

  private String cdrDir;

  private String warn;

  private boolean init;

  public CdrController() {
    super();
    if(getRequestParameter("cdr_init") == null) {
      CdrSettings p = getConfig().getCdrProperties();
      cdrDir = p.getCdrDir();
    }
    if(!getConfig().isCdrStarted()) {
      ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", getLocale());
      warn = bundle.getString("informer.cdr.daemon.offline");
    }
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }

  public String getWarn() {
    return warn;
  }

  public String getCdrDir() {
    return cdrDir;
  }

  public void setCdrDir(String cdrDir) {
    this.cdrDir = cdrDir;
  }

  public String save() {
    try{
      Configuration config = getConfig();
      CdrSettings p = config.getCdrProperties();
      p.setCdrDir(cdrDir);
      config.setCdrSettings(p, getUserName());
      return "INDEX";
    }catch (AdminException e){
      addError(e);
      return null;
    }

  }
}
