package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.monitoring.MBean;
import mobi.eyeline.informer.admin.smppgw.protogen.protocol.UpdateConfig;
import org.apache.log4j.Logger;

import java.util.Properties;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWImpl implements SmppGW {

  private final SmppGWClient client;

  private static final Logger logger = Logger.getLogger("SMPPGW");

  private String url;

  public SmppGWImpl(Properties p) throws InitException{
    String host;
    int port;
    try{
      host = p.getProperty("update.config.server.host");
      port = Integer.parseInt(p.getProperty("update.config.server.port"));
    }catch (Exception e){
      throw new InitException(e);
    }
    if(host == null || host.length() == 0) {
      throw new InitException("Host is empty");
    }
    url = host+':'+port;
    this.client = new SmppGWClient(host, port, logger);
  }

  void checkResponse(int status) throws SmppGWException {
    switch (status) {
      case 0 : return;
      case 6 : throw new SmppGWException("server_not_ready");
      default: throw new SmppGWException("interaction_error", status + "");
    }
  }

  private static MBean getMBean() {
    return MBean.getInstance(MBean.Source.SMPPGW);
  }

  @Override
  public void updateConfig() throws AdminException {
    try{
      UpdateConfig uc = new UpdateConfig();
      checkResponse(client.send(uc).getStatus());
      getMBean().notifyInteractionOk(url);
    }catch (AdminException e) {
      getMBean().notifyInteractionError(url, e.getMessage());
      throw e;
    }
  }

  @Override
  public void shutdown() {
    client.shutdown();
  }
}
