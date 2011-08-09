package mobi.eyeline.informer.web.controllers.smppgw;


import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smppgw.SmppGWProvider;
import mobi.eyeline.informer.admin.smppgw.SmppGWProviderSettings;
import mobi.eyeline.informer.web.controllers.InformerController;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWProviderEditController extends InformerController{


  private ProviderEditPage activePage;

  private SmppGWProvider provider;

  private boolean isNew;

  public String setProvider() {
    String e = getRequestParameter("provider");
    SmppGWProviderSettings ss = getConfig().getSmppGWProviderSettings();
    if(e != null && e.length() > 0) {
      provider = ss.getProvider(e);
      isNew = false;
    }else {
      provider = new SmppGWProvider();
      isNew = true;
    }
    activePage = new ProviderSettingsPage(provider, isNew);
    return "SMPPGW_PROVIDER_EDIT";
  }


  public String next() {
    try {
      ProviderEditPage page = activePage.nextPage();
      if (page != null) {
        activePage = page;
      }
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String back() {
    try {
      ProviderEditPage page = activePage.backPage();
      if (page != null) {
        activePage = page;
      }
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String cancel() {
    if(provider != null) {
      activePage = new ProviderSettingsPage(provider, isNew);
    }
    return null;
  }

  public ProviderEditPage getActivePage() {
    return activePage;
  }
}
