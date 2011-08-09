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

  private String oldName;

  private String newName;

  public SmppGWEndpointEditController() {
    String e = getRequestParameter("endpoint");
    if(e != null && e.length() != 0) {
      endpoint = getConfig().getSmppGWEndpointSettings().getEndpoint(e);
      if(endpoint != null) {
        oldName = e;
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

  public String getNewName() {
    return newName;
  }

  public void setNewName(String newName) {
    this.newName = newName;
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

  public String getOldName() {
    return oldName;
  }

  public void setOldName(String oldName) {
    this.oldName = oldName;
  }


  public String save() {
    if(oldName == null) {
      endpoint.setName(newName);
    }
    try{
      endpoint.validate();
      SmppGWEndpointSettings s = getConfig().getSmppGWEndpointSettings();
      if(oldName != null) {
        s.removeEndpoint(endpoint.getName());
      }
      s.addEndpoint(endpoint);
      getConfig().updateSmppGWSettings(s);
    }catch (AdminException e){
      addError(e);
      return null;
    }
    return "SMPPGW_ENDPOINTS";
  }
}
