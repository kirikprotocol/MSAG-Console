package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * @author Aleksandr Khalitov
 */
class InformerConfig implements ManagedConfigFile<InformerSettings> {

  protected InformerSettings load(XmlConfig config) throws XmlConfigException, AdminException {
    InformerSettings s = new InformerSettings();
    s.load(config);
    return s;
  }

  public InformerSettings load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig(is);
    return load(config);
  }

  protected void save(XmlConfig config, InformerSettings settings) throws XmlConfigException {
    settings.save(config);
  }

  public void save(InputStream is, OutputStream os, InformerSettings settings) throws Exception {
    XmlConfig config = new XmlConfig(is);
    save(config, settings);
    config.save(os);
  }
}
