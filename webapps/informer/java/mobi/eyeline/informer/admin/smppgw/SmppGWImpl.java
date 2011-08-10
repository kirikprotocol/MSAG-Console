package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.smppgw.protogen.protocol.UpdateConfig;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWImpl implements SmppGW {

  private final SmppGWClient client;

  private static final Logger logger = Logger.getLogger("SMPPGW");

  public SmppGWImpl(File configFile, FileSystem fileSystem) throws InitException{
    InputStream is = null;
    Properties p = new Properties();
    String host;
    int port;
    try{
      try{
        is = fileSystem.getInputStream(configFile);
        p.load(is);
      }finally {
        if(is != null) {
          try{
            is.close();
          }catch (IOException ignored){}
        }
      }
      host = p.getProperty("update.config.server.host");
      port = Integer.parseInt(p.getProperty("update.config.server.port"));
    }catch (Exception e){
      throw new InitException(e);
    }
    if(host == null || host.length() == 0) {
      throw new InitException("Host is empty");
    }
    this.client = new SmppGWClient(host, port, logger);
  }


  void checkResponse(int status) throws SmppGWException {
    switch (status) {
      case 0 : return;
      case 6 : throw new SmppGWException("server_not_ready");
      default: throw new SmppGWException("interaction_error", status + "");
    }
  }

  @Override
  public void updateConfig() throws AdminException {
    UpdateConfig uc = new UpdateConfig();
    checkResponse(client.send(uc).getStatus());
  }

  @Override
  public void shutdown() {
    client.shutdown();
  }
}
