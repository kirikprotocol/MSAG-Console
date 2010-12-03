package mobi.eyeline.informer.web.controllers.config;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.config.Revision;
import mobi.eyeline.informer.web.controllers.SettingsController;

import javax.faces.application.FacesMessage;

/**
 * Контроллер для конфигурации Informer
 *
 * @author Aleksandr Khalitov
 */
public class InformerConfigController extends SettingsController<InformerSettings> {

  private static final org.apache.log4j.Logger logger = org.apache.log4j.Logger.getLogger(InformerConfigController.class);

  private InformerSettings settings;

  private boolean additionalOpened;

  public InformerConfigController() {

    try {
      super.init(Configuration.ConfigType.CONFIG);
    } catch (AdminException e) {
      addError(e);
      return;
    }

    if(getRequestParameter("revision") == null) {
        settings = getConfig().getConfigSettings();
    }
  }

  public boolean isAdditionalOpened() {
    return additionalOpened;
  }

  public void setAdditionalOpened(boolean additionalOpened) {
    this.additionalOpened = additionalOpened;
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
      additionalOpened = true;
      logger.warn(e, e);
      addError(e);
      return null;
    }
  }

  public String reset() {
    try {
      resetSettings();
    } catch (AdminException e) {
      logger.warn(e, e);
      addError(e);
    }
    return "CONFIG";
  }

  public InformerSettings getSettings() {
    return settings;
  }

  @Override
  protected InformerSettings loadSettings() throws AdminException {
    return getConfig().getConfigSettings();
  }

  @Override
  protected void saveSettings(InformerSettings settings) throws AdminException {
    getConfig().setConfigSettings(settings, getUserPrincipal().getName());
  }

  @Override
  protected InformerSettings cloneSettings(InformerSettings settings) {
    return settings.cloneSettings();
  }
}
