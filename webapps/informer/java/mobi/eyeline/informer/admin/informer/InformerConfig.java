package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * @author Aleksandr Khalitov
 */
class InformerConfig implements ManagedConfigFile<InformerSettings> {

  public InformerSettings load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig(is);

    InformerSettings settings = new InformerSettings();

    XmlConfigSection s = config.getSection("informer");
    settings.setAdminHost(s.getString("adminHost"));
    settings.setAdminPort(s.getInt("adminPort"));

    settings.setDcpHost(s.getString("dcpHost"));
    settings.setDcpPort(s.getInt("dcpPort"));
    
    settings.setStatDir(s.getString("statPath", null));
    settings.setStoreDir(s.getString("storePath"));

    s = config.getSection("pvss");
    settings.setPersHost(s.getString("host"));
    settings.setPersPort(s.getInt("port"));

    return settings;
  }

  public void save(InputStream is, OutputStream os, InformerSettings settings) throws Exception {
    XmlConfig config = new XmlConfig(is);
    XmlConfigSection s = config.getOrCreateSection("informer");
    s.setString("adminHost", settings.getAdminHost());
    s.setInt("adminPort", settings.getAdminPort());
    s.setString("dcpHost", settings.getDcpHost());
    s.setInt("dcpPort", settings.getDcpPort());
    s.setString("statPath", settings.getStatDir());
    s.setString("storePath", settings.getStoreDir());

    s = config.getOrCreateSection("pvss");
    s.setString("host", settings.getPersHost());
    s.setInt("port", settings.getPersPort());

    config.save(os);
  }
}
