package mobi.eyeline.informer.web.controllers.cdr;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.cdr.CdrDaemon;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import java.util.Properties;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class CdrController extends InformerController{

  private String cdrDir;

  private String warn;

  public CdrController() {
    super();
    Properties p = getConfig().getCdrProperties();
    cdrDir = p.getProperty(CdrDaemon.CDR_DIR);
    if(!getConfig().isCdrStarted()) {
      ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", getLocale());
      warn = bundle.getString("informer.cdr.daemon.offline");
    }
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
      Properties p = config.getCdrProperties();
      p.setProperty(CdrDaemon.CDR_DIR, cdrDir);
      config.setCdrProperties(p, getUserName());
      return "INDEX";
    }catch (AdminException e){
      addError(e);
      return null;
    }

  }
}
