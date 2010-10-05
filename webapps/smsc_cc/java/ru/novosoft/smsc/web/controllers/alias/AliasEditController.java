package ru.novosoft.smsc.web.controllers.alias;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 01.10.2010
 * Time: 14:56:14
 */
public class AliasEditController extends SmscController {

  String oldAliasSimpleAddr=null;
  String simpleAlias=null;
  String simpleAddress =null;
  boolean hide = false;

  public AliasEditController() {
    super();
    try {
      init();
    }
    catch (AdminException e) {
      addError(e);
    }
  }

  private void init() throws AdminException {
    oldAliasSimpleAddr = getRequestParameter("aliasToEdit");
    Alias a = getAliasBySimpleAddr(oldAliasSimpleAddr);
    if(a!=null) {
      simpleAddress = a.getAddress().getSimpleAddress();
      simpleAlias  = a.getAlias().getSimpleAddress();
      hide         = a.isHide();
    }
    else {
      simpleAddress=null;
      simpleAlias=null;
      hide=false;
    }
  }




  private Alias getAliasBySimpleAddr(String oldAliasSimpleAddr) throws AdminException {
    if(oldAliasSimpleAddr!=null && oldAliasSimpleAddr.length()>0) {
      AliasSet aset = null;
      try {
        aset = WebContext.getInstance().getAliasManager().getAliases();
        while(aset.next()) {
          Alias a = aset.get();
          if(a.getAlias().getSimpleAddress().equals(oldAliasSimpleAddr)) {
            return a;
          }
        }
      }
      finally {
        if(aset!=null) aset.close();
      }
    }
    return null;
  }

  public String getOldAliasSimpleAddr() {
    return oldAliasSimpleAddr;
  }

  public void setOldAliasSimpleAddr(String oldAliasSimpleAddr) {
    this.oldAliasSimpleAddr = oldAliasSimpleAddr;
  }

  public String getSimpleAlias() {
    return simpleAlias;
  }

  public void setSimpleAlias(String simpleAlias) {
    this.simpleAlias = simpleAlias;
  }

  public String getSimpleAddress() {
    return simpleAddress;
  }

  public void setSimpleAddress(String simpleAddress) {
    this.simpleAddress = simpleAddress;
  }

  public boolean isHide() {
    return hide;
  }

  public void setHide(boolean hide) {
    this.hide = hide;
  }


  public String submit() throws AdminException {
    if(!check(simpleAlias)) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.alias.edit.alias.checkFail");
      return null;
    }
    if(!check(simpleAddress)){
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.alias.edit.address.checkFail");
      return null;
    }
    remove();
    Alias a = new Alias(new Address(simpleAddress),new Address(simpleAlias),hide);
    WebContext.getInstance().getAliasManager().addAlias(a);
    return "ALIASES";
  }

  private boolean check(String simpleAddress) {
    return Address.validate(simpleAddress);
  }

  public String remove() throws AdminException {
    if(oldAliasSimpleAddr!=null && oldAliasSimpleAddr.length()>0) {
      Alias a = getAliasBySimpleAddr(oldAliasSimpleAddr);
      WebContext.getInstance().getAliasManager().deleteAlias(a.getAlias());
    }
    return "ALIASES";
  }

  public String reset() throws AdminException {
    init();
    return null;
  }
}
