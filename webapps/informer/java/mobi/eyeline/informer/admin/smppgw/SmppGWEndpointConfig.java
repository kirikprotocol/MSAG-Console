package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.config.XmlConfig;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWEndpointConfig implements ManagedConfigFile< SmppGWEndpointSettings> {

  private final static String ENDPOINTS_SECTION = "endpoints";

  @Override
  public void save(InputStream oldFile, OutputStream newFile,  SmppGWEndpointSettings settings) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(oldFile);
    settings.save(config.getOrCreateSection(ENDPOINTS_SECTION));
    config.save(newFile);
  }

  @Override
  public SmppGWEndpointSettings load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(is);
    SmppGWEndpointSettings settings = new SmppGWEndpointSettings();
    settings.load(config.getOrCreateSection(ENDPOINTS_SECTION));
    return settings;
  }
}
