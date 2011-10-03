package ru.novosoft.smsc.web.controllers.provider;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.provider.ProviderSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import java.util.*;


public class ProviderController extends SettingsMController<ProviderSettings> {

  private List<Provider> providers;
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


  private void updateProviders(ProviderSettings s) throws AdminException {
    for(Provider provider : providers) {
      Provider p = s.getProvider(provider.getId());
      p.setName(provider.getName());
      s.updateProvider(p);
    }
  }

  private void resetProviders() {
    providers = new LinkedList<Provider>();
    for (Provider c : getSettings().getProviders())
      providers.add(c);
  }

  public List<Provider> getProviders() {
    return providers;
  }

  public String addProvider() {
    try {
      ProviderSettings s = getSettings();
      s.addProvider(newProvider);
      setSettings(s);
      resetProviders();
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