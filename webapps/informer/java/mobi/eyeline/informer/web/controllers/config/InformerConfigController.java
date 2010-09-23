package mobi.eyeline.informer.web.controllers.config;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.WebContext;
import mobi.eyeline.informer.web.controllers.SettingsController;
import mobi.eyeline.informer.web.informer.InformerSettings;

import javax.faces.application.FacesMessage;

/**
 * author: alkhal
 */
public class InformerConfigController extends SettingsController<InformerSettings> {

  private static final org.apache.log4j.Logger logger = org.apache.log4j.Logger.getLogger(InformerConfigController.class);


  private InformerSettings settings;

  public InformerConfigController() {
    super(ConfigType.Logger);

    if (isSettingsChanged())
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.configuration.locally.changed");

    if(getRequestParameter("revision") == null) {
      try{
        settings = WebContext.getInstance().getConfiguration().getConfigSettings();
      }catch (AdminException e){
        addError(e);
      }
    }
  }

  public String save() {
    try {
      setSettings(settings);
      Revision rev = submitSettings();
      if (rev != null) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "informer.config.not.actual", rev.getUser());
        return null;
      }
      return "INDEX";
    } catch (AdminException e) {
      logger.warn(e, e);
      addError(e);
      return null;
    }
  }

  public InformerSettings getSettings() {
    return settings;
  }

  @Override
  protected InformerSettings loadSettings() throws AdminException {
    return WebContext.getInstance().getConfiguration().getConfigSettings();
  }

  @Override
  protected void saveSettings(InformerSettings settings) throws AdminException {
    WebContext.getInstance().getConfiguration().setConfigSettings(settings, getUserPrincipal().getName());
  }

  @Override
  protected InformerSettings cloneSettings(InformerSettings settings) {
    return settings.cloneSettings();
  }
}
