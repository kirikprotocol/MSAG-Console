package ru.sibinco.smppgw.beans.stat.stat;

import ru.sibinco.smppgw.beans.SmppgwJspException;
import ru.sibinco.smppgw.beans.SmppgwBean;
import ru.sibinco.smppgw.backend.stat.stat.StatQuery;
import ru.sibinco.smppgw.backend.stat.stat.Statistics;
import ru.sibinco.smppgw.backend.stat.stat.Stat;
import ru.sibinco.smppgw.backend.sme.Provider;
import ru.sibinco.smppgw.backend.SmppGWAppContext;
import ru.sibinco.smppgw.Constants;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.Functions;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.*;
import java.security.Principal;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.08.2004
 * Time: 14:32:38
 * To change this template use File | Settings | File Templates.
 */
public class Index extends SmppgwBean
{
  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
  private static final String ALL_PROVIDERS = "ALL PROVIDERS";

  private String mbQuery = null;

  private Stat stat = new Stat();
  private StatQuery query = new StatQuery();
  private Statistics statistics = null;

  private boolean administrator = false;
  private long userProviderId = StatQuery.ALL_PROVIDERS;

  private String providerName = null;
  private String[] providerIds = null;
  private String[] providerNames = null;

  private void init() throws SmppgwJspException
  {
    SmppGWAppContext context = getAppContext();
    stat.setDataSource(context.getDataSource());
    Principal userPrincipal = super.getLoginedPrincipal();
    if (userPrincipal == null)
      throw new SmppgwJspException(
          Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
    User user = (User)context.getUserManager().getUsers().get(userPrincipal.getName());
    if (user == null)
      throw new SmppgwJspException(
          Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '"+userPrincipal.getName()+"'");

    userProviderId = user.getProviderId();
    System.out.println("User: "+userPrincipal.getName()+", Proviedr id: "+userProviderId);
    administrator = (userProviderId == StatQuery.ALL_PROVIDERS);
    if (administrator)
    {
      Map providers = context.getProviderManager().getProviders();
      ArrayList ids = new ArrayList(100);
      ArrayList names = new ArrayList(100);
      for (Iterator i=providers.values().iterator(); i.hasNext(); ) {
        Object obj = i.next();
        if (obj != null && obj instanceof Provider) {
          Provider provider = (Provider)obj;
          ids.add(Long.toString(provider.getId()));
          names.add(provider.getName());
        }
      }
      ids.add(0, Long.toString(StatQuery.ALL_PROVIDERS));
      names.add(0, ALL_PROVIDERS); // TODO: sort names ?
      providerIds = (String[])(ids.toArray(new String[0]));
      providerNames = (String[])(names.toArray(new String[0]));
    } else {
      query.setProviderId(userProviderId);
      providerName = user.getName();
    }
  }

  public void process(HttpServletRequest request, HttpServletResponse response)
      throws SmppgwJspException
  {
    super.process(request, response);
    this.init();

    if (mbQuery != null) {
      try {
        if (userProviderId != StatQuery.ALL_PROVIDERS && userProviderId != query.getProviderId())
          throw new Exception("Permission denied for user '"+providerName+
                              "' to access other providers's statistics");
        statistics = stat.getStatistics(query);
      } catch (Exception exc) {
        statistics = null; mbQuery = null;
        throw new SmppgwJspException(Constants.errors.stat.GET_STATISTICS_FAILED, exc);
      }
    } else if (!query.isFromDateEnabled()) {
      query.setFromDate(Functions.truncateTime(new Date()));
      query.setFromDateEnabled(true);
    }
    mbQuery = null;
  }

  public boolean isAdministrator() {
    return administrator;
  }

  public long getProviderId() {
    return query.getProviderId();
  }
  public void setProviderId(long providerId) {
    query.setProviderId(providerId);
  }

  public String[] getProviderIds() {
    return providerIds;
  }
  public String[] getProviderNames() {
    return providerNames;
  }
  public String getProviderName() {
    return providerName;
  }

  private Date convertStringToDate(String date)
  {
    Date converted = new Date();
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      converted = formatter.parse(date);
    } catch (ParseException e) {
      e.printStackTrace();
    }
    return converted;
  }
  private String convertDateToString(Date date)
  {
    SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
    return formatter.format(date);
  }

  public String getFromDate() {
    return (query.isFromDateEnabled()) ?  convertDateToString(query.getFromDate()) : "";
  }
  public void setFromDate(String fromDate)
  {
    if (fromDate != null && fromDate.trim().length() > 0) {
      query.setFromDate(convertStringToDate(fromDate));
      query.setFromDateEnabled(true);
    } else {
      query.setFromDateEnabled(false);
    }
  }
  public String getTillDate()  {
    return (query.isTillDateEnabled()) ?  convertDateToString(query.getTillDate()) : "";
  }
  public void setTillDate(String tillDate)
  {
    if (tillDate != null && tillDate.trim().length() > 0) {
      query.setTillDate(convertStringToDate(tillDate));
      query.setTillDateEnabled(true);
    } else {
      query.setTillDateEnabled(false);
    }
  }

  public Statistics getStatistics() {
    return statistics;
  }

  public String getMbQuery() {
    return mbQuery;
  }
  public void setMbQuery(String mbQuery) {
    this.mbQuery = mbQuery;
  }

}
