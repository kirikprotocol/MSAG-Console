package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * @author Aleksandr Khalitov
 */
class ArchiveDaemonConfig implements ManagedConfigFile<ArchiveDaemonSettings> {

  @Override
  public void save(InputStream oldFile, OutputStream newFile, ArchiveDaemonSettings conf) throws Exception {
    XmlConfig config = new XmlConfig(oldFile);
    XmlConfigSection s = config.getOrCreateSection("informer");
    s.setString("dcpHost", conf.getDcpHost());
    s.setInt("dcpPort", conf.getDcpPort());
    config.save(newFile);
  }

  @Override
  public ArchiveDaemonSettings load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig(is);

    ArchiveDaemonSettings settings = new ArchiveDaemonSettings();

    XmlConfigSection s = config.getSection("informer");
    settings.setDcpHost(s.getString("dcpHost"));
    settings.setDcpPort(s.getInt("dcpPort"));

    return settings;
  }

}
