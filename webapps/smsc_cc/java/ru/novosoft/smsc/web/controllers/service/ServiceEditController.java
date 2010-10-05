package ru.novosoft.smsc.web.controllers.service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.sme.Sme;
import ru.novosoft.smsc.web.controllers.SmscController;

/**
 * @author Artem Snopkov
 */
public class ServiceEditController extends ServiceController {

  private String systemId;
  private Sme sme;
  private boolean add = true;

  public ServiceEditController() {
    systemId = getRequestParameter("smeId");
    if (systemId != null) {
      try {
        sme = mngr.getSme(systemId);
        add = false;
      } catch (AdminException e) {
        addError(e);
      }
    }

    if (sme == null) {
      sme = new Sme();
      try {
        sme.setReceiptSchemeName("default");
      } catch (AdminException e) {        
      }
    }
  }

  public void setAdd(boolean add) {
    this.add = add;
  }

  public boolean isAdd() {
    return add;
  }

  public Sme getSme() {
    return sme;
  }

  public String getSystemId() {
    return systemId;
  }

  public void setSystemId(String systemId) {
    this.systemId = systemId;
  }

  public String save() {
    try {
      mngr.addSme(systemId, sme);
      return "SERVICES";
    } catch (AdminException e) {
      addError(e);
      return null;
    }
  }


}
