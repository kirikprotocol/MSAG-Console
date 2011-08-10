package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

/**
 * author: Aleksandr Khalitov
 */
class SmppGWProviderConfig implements ManagedConfigFile<SmppGWProviderSettings> {

  private final static String PROVIDERS_SECTION = "providers";

  private SmppGWConfigManagerContext context;

  SmppGWProviderConfig(SmppGWConfigManagerContext context) {
    this.context = context;
  }

  @Override
  public void save(InputStream oldFile, OutputStream newFile, SmppGWProviderSettings providerSettings) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(oldFile);
    providerSettings.save(config.getOrCreateSection(PROVIDERS_SECTION));

    Set<String> users = new HashSet<String>();
    for(SmppGWProvider p : providerSettings.getProviders()) {
      for(SmppGWRoute r : p.getRoutes()) {
        users.add(r.getUser());
      }
    }
    Collection<User> us = new ArrayList<User>(users.size());
    for(String u : users) {
      us.add(context.getUser(u));
    }
    saveUsers(config, us);
    config.save(newFile);
  }


  private void saveUsers(XmlConfigSection parent, Collection<User> users) throws XmlConfigException {
    XmlConfigSection s = parent.getOrCreateSection("users");
    s.clear();
    for(User u : users) {
      XmlConfigSection us = s.getOrCreateSection(u.getLogin());
      us.setString("password", u.getPassword());
    }
  }

  @Override
  public SmppGWProviderSettings load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(is);
    SmppGWProviderSettings providerSettings = new SmppGWProviderSettings();
    providerSettings.load(config.getOrCreateSection(PROVIDERS_SECTION));
    return providerSettings;
  }
}
