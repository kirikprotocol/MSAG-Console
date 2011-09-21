package mobi.eyeline.informer.web.controllers.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smppgw.SmppGWEndpoint;
import mobi.eyeline.informer.admin.smppgw.SmppGWEndpointSettings;
import mobi.eyeline.informer.admin.smppgw.SmppGWProvider;
import mobi.eyeline.informer.web.controllers.InformerController;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWEndpointEditController extends InformerController{

  private String providerName;

  private SmppGWEndpoint endpoint;

  private String oldSysID;

  private String newSysID;

  public SmppGWEndpointEditController() {
    String e = getRequestParameter("endpoint");
    if(e != null && e.length() != 0) {
      endpoint = getConfig().getSmppGWEndpointSettings().getEndpoint(e);
      if(endpoint != null) {
        oldSysID = e;
        SmppGWProvider p = getConfig().getSmppGWProviderSettings().getProviderByEndpoint(endpoint.getSystemId());
        if(p != null) {
          providerName = p.getName();
        }
      }
    }
    if(endpoint == null){
      endpoint = new SmppGWEndpoint();
    }
  }

  public String getNewSysID() {
    return newSysID;
  }

  public void setNewSysID(String newSysID) {
    this.newSysID = newSysID;
  }
  public SmppGWEndpoint getEndpoint() {
    return endpoint;
  }

  public void setEndpoint(SmppGWEndpoint endpoint) {
    this.endpoint = endpoint;
  }

  public String getProviderName() {
    return providerName;
  }

  public String getOldSysID() {
    return oldSysID;
  }

  public void setOldSysID(String oldSysID) {
    this.oldSysID = oldSysID;
  }


  public String save() {
    if(oldSysID == null) {
      endpoint.setSystemId(newSysID);
    }
    try{
      endpoint.validate();
      SmppGWEndpointSettings s = getConfig().getSmppGWEndpointSettings();
      if(oldSysID != null) {
        s.removeEndpoint(endpoint.getSystemId());
      }
      s.addEndpoint(endpoint);
      getConfig().updateSmppGWSettings(s, getUserName());
    }catch (AdminException e){
      addError(e);
      return null;
    }
    return "SMPPGW_ENDPOINTS";
  }
}
