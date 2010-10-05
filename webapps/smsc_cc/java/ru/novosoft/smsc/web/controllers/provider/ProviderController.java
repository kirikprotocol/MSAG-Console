package ru.novosoft.smsc.web.controllers.provider;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.provider.ProviderSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import javax.faces.application.FacesMessage;
import java.util.Collection;
import java.util.Map;
import java.util.TreeMap;


public class ProviderController extends SettingsMController<ProviderSettings> {

  private Map<Long, String> providers;
  private String newProvider;
  private boolean initError;

  public ProviderController() {
    super(WebContext.getInstance().getProviderManager());
    try {
      init();
    } catch (AdminException e) {
      addError(e);
      initError = true;
      return;
    }
    resetProviders();
  }

  public boolean isInitError() {
    return initError;
  }

  private void checkChanges() {
    if (isSettingsChanged())
      addLocalizedMessage(FacesMessage.SEVERITY_INFO, "smsc.submit.hint");
  }

  private void updateProviders(ProviderSettings s) throws AdminException {
    for (Long categoryId : providers.keySet()) {
      Provider p = s.getProvider(categoryId);
      p.setName(providers.get(categoryId));
      s.updateProvider(p);
    }
  }

  private void resetProviders() {
    providers = new TreeMap<Long, String>();
    for (Provider c : getSettings().getProviders())
      providers.put(c.getId(), c.getName());
  }

  public Collection<Long> getIds() {
    return providers.keySet();
  }

  public Map<Long, String> getProviders() {
    return providers;
  }

  public String addProvider() {
    try {
      ProviderSettings s = getSettings();
      updateProviders(s);
      s.addProvider(newProvider);
      setSettings(s);
      resetProviders();
      checkChanges();
      newProvider = null;
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String submit() {
    try {
      ProviderSettings s = getSettings();
      updateProviders(s);
      setSettings(s);
      submitSettings();
    } catch (AdminException e) {
      addError(e);
      return null;
    }
    return "INDEX";
  }

  public String reset() {
    try {
      resetSettings();
      resetProviders();
    } catch (AdminException e) {
      addError(e);
    }
    return "PROVIDERS";
  }

  public String getNewProvider() {
    return newProvider;
  }

  public void setNewProvider(String newProvider) {
    this.newProvider = newProvider;
  }
}