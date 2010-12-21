package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.infosme.protogen.protocol.*;

/**
 * Администрирование Informer
 *
 * @author Aleksandr Khalitov
 */
public class InfosmeImpl implements Infosme {

  private final InfosmeClient client;

  public InfosmeImpl(String host, int port) {
    this.client = new InfosmeClient(host, port);
  }

  protected void checkResponse(int status) throws InfosmeException {
    switch (status) {
      case 0 : return;
      case 6 : throw new InfosmeException("server_not_ready");
      default: throw new InfosmeException("interaction_error", status + "");
    }
  }

  public void addSmsc(String smscId) throws AdminException {
    if (smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciSmsc);
    configOp.setOp(ConfigOpId.coAdd);
    configOp.setObjName(smscId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void removeSmsc(String smscId) throws AdminException {
    if (smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciSmsc);
    configOp.setOp(ConfigOpId.coRemove);
    configOp.setObjName(smscId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void updateSmsc(String smscId) throws AdminException {
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciSmsc);
    configOp.setOp(ConfigOpId.coUpdate);
    configOp.setObjName(smscId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void addRegion(int regionId) throws AdminException {
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciRegion);
    configOp.setOp(ConfigOpId.coAdd);
    configOp.setObjId(regionId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void updateRegion(int regionId) throws AdminException {
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciRegion);
    configOp.setOp(ConfigOpId.coUpdate);
    configOp.setObjId(regionId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void removeRegion(int regionId) throws AdminException {
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciRegion);
    configOp.setOp(ConfigOpId.coRemove);
    configOp.setObjId(regionId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void addUser(String userId) throws AdminException {
    if (userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciUser);
    configOp.setOp(ConfigOpId.coAdd);
    configOp.setObjName(userId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void updateUser(String userId) throws AdminException {
    if (userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciUser);
    configOp.setOp(ConfigOpId.coUpdate);
    configOp.setObjName(userId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void removeUser(String userId) throws AdminException {
    if (userId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    ConfigOp configOp = new ConfigOp();
    configOp.setCfgId(ConfigId.ciUser);
    configOp.setOp(ConfigOpId.coRemove);
    configOp.setObjName(userId);
    checkResponse(client.send(configOp).getStatus());
  }

  public void setDefaultSmsc(String smscId) throws AdminException {
    if (smscId == null) {
      throw new IllegalArgumentException("Id is null");
    }
    SetDefaultSmsc s = new SetDefaultSmsc();
    s.setId(smscId);
    checkResponse(client.send(s).getStatus());
  }

  public void sendTestSms(TestSms sms) throws AdminException {
    SendTestSms req = new SendTestSms();
    req.setAbonent(sms.getDestAddr().getSimpleAddress());
    req.setSourceAddr(sms.getSourceAddr().getSimpleAddress());
    req.setDeliveryMode(DeliveryMode.valueOf(sms.getMode().toString()));
    req.setFlash(sms.isFlash());
    req.setText(sms.getText());
    checkResponse(client.send(req).getRespCode());
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void shutdown() {
    if (client != null) {
      try {
        client.shutdown();
      } catch (Exception e) {
      }
    }
  }


}
