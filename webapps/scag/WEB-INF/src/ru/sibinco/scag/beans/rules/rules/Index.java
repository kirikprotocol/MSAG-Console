package ru.sibinco.scag.beans.rules.rules;

import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.users.User;

import javax.servlet.http.HttpSession;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;
import java.security.Principal;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.08.2005
 * Time: 19:23:50
 * To change this template use File | Settings | File Templates.
 */
public class Index extends TabledBeanImpl implements TabledBean {
  //private final Map rules = Collections.synchronizedMap(new HashMap());
  private static final String ALL_PROVIDERS = "ALL PROVIDERS";
  private static final String ALL_TRANSPORTS = "ALL TRANSPORTS";
  private static final long ALL = -1;
  private long transportId = 0;
  private String[] transportIdsWithAll = new String[Transport.transportIds.length + 1];
  private String[] transportTitlesWithAll = new String[Transport.transportTitles.length + 1];
  private long providerId = 0;
  private String[] providerIdsWithAll = null;  //{"0","1" ,"2","3","4"};
  private String[] providerNamesWithAll = null; //{"All","hello_test","mark","burger","provider4"};
  private boolean newRule = false;
  private boolean administrator = false;
  private long userProviderId = -1;
  protected HttpSession session = null;

  private void init() throws SCAGJspException {
    //PROVIDERS
    SCAGAppContext appContext = getAppContext();
    Principal userPrincipal = super.getLoginedPrincipal();
    if (userPrincipal == null)
      throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to  obtain user principal(s)");
    User user = (User) appContext.getUserManager().getUsers().get(userPrincipal.getName());
    if (user == null)
      throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");

    userProviderId = user.getProviderId();
    administrator = (userProviderId == ALL);
    if (administrator) {
      Map providers = appContext.getProviderManager().getProviders();
      ArrayList ids = new ArrayList();
      ArrayList names = new ArrayList();
      ids.add("0");
      names.add(ALL_PROVIDERS);
      for (Iterator i = providers.values().iterator(); i.hasNext();) {
        Object obj = i.next();
        if (obj != null && obj instanceof Provider) {
          Provider provider = (Provider) obj;
          ids.add(Long.toString(provider.getId()));
          names.add(provider.getName());
        }
      }
      providerIdsWithAll = (String[]) (ids.toArray(new String[ids.size()]));
      providerNamesWithAll = (String[]) (names.toArray(new String[names.size()]));
    } else {
      setProviderId(userProviderId);
      Object obj = appContext.getProviderManager().getProviders().get(new Long(userProviderId));
      if (obj == null || !(obj instanceof Provider))
        throw new SCAGJspException(Constants.errors.providers.PROVIDER_NOT_FOUND,
          "Failed to locate provider for id=" + userProviderId);
      providerId = ((Provider) obj).getId();
    }
    //TRANSPORTS
    System.arraycopy(Transport.transportIds, 0, transportIdsWithAll, 1, Transport.transportIds.length);
    transportIdsWithAll[0] = String.valueOf(0);
    System.arraycopy(Transport.transportTitles, 0, transportTitlesWithAll, 1, Transport.transportTitles.length);
    transportTitlesWithAll[0] = ALL_TRANSPORTS;
  }

  public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
    session = request.getSession(false);
    super.process(request, response);
    this.init();
    super.process(request, response); //bug - the same situation like in ru.sibinco.scag.beans.routing.routes.Index!!!
  }

  protected Collection getDataSource() {
    RuleManager ruleManager = appContext.getRuleManager();
    newRule = (session.getAttribute("newRule") != null);
    return ruleManager.getRules().values();
  }

  public SortedList filter(SortedList s) {
    s = filterByTransport(s);
    s = filterByProvider(s);
    return s;
  }

  public SortedList filterByTransport(SortedList s) {
    //System.out.println("transportId: " + transportId);
    if (transportId == 0) return s;
    SortedList temp = new SortedList();
    for (int i = 0; i < s.size(); i++) {
      Rule cur_rule = (Rule) s.get(i);
      //System.out.println("rule transport: " + cur_rule.getTransport());
      if (cur_rule.getTransport().equals(Transport.transportTitles[(int) transportId - 1])) {
        temp.add(cur_rule);
      }
    }
    return temp;
  }

  public SortedList filterByProvider(SortedList s) {
    //System.out.println("providerId: " + providerId);
    if (providerId == 0) return s;
    SortedList temp = new SortedList();
    for (int i = 0; i < s.size(); i++) {
      Rule cur_rule = (Rule) s.get(i);
      //System.out.println("rule provider: " + cur_rule.getProvider());
      if (cur_rule.getProvider().getId() == providerId) {
        temp.add(cur_rule);
      }
    }
    return temp;
  }

  public long getTransportId() {
    return transportId;
  }

  public String[] getTransportIds() {
    return transportIdsWithAll;
  }

  public String[] getTransportTitles() {
    return transportTitlesWithAll;
  }

  public void setTransportId(long transportId) {
    this.transportId = transportId;
  }

  public String[] getProviderIds() {
    return providerIdsWithAll;
  }

  public String[] getProviderNames() {
    return providerNamesWithAll;
  }

  public long getProviderId() {
    return providerId;
  }

  public void setProviderId(long providerId) {
    this.providerId = providerId;
  }

  public boolean isAdministrator() {
    return administrator;
  }

  public boolean isNewRule() {
    return newRule;
  }

  protected void delete() {
    RuleManager ruleManager = appContext.getRuleManager();
    final Map rules = ruleManager.getRules();
    //rules.keySet().removeAll(checkedSet);
    for (Iterator iterator = checkedSet.iterator(); iterator.hasNext();) {
      final String ruleId = (String) iterator.next();
      Rule rule = (Rule) rules.get(Long.decode(ruleId));
      try {
        if (rule != null) ruleManager.removeRule(ruleId, rule.getTransport());
        rules.remove(Long.decode(ruleId));
      } catch (SibincoException se) {
          if (se instanceof StatusDisconnectedException) rules.remove(Long.decode(ruleId));
          else /*PRINT ERROR ON THE SCREEN*/;
        //se.printStackTrace();
      }
    }
  }
}

