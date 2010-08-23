package ru.novosoft.smsc.admin.preferences;

/*
 * Created by igork
 * Date: 05.11.2002
 * Time: 19:06:21
 */

//todo ���� �� ������� ��������� ��� topmonPrefs, perfmonPrefs � ������ ������� ��������� ������, ����� ��� �� �������� ������ ���

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.category.CategoryFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.dl.DlFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.dl.PrincipalsFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.provider.ProviderFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.route.RouteFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.smcstat.StatRouteFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.subject.SubjectFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.user.UserFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.closedgroups.ClosedGroupFilter;
import ru.novosoft.smsc.util.LocaleMessages;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.admin.AdminException;

import java.util.*;
import java.text.SimpleDateFormat;

public class UserPreferences {
  private int profilesPageSize = 20;
  private String profilesSortOrder = null;
  private String profilesFilter = null;

  private int blackNickPageSize = 10;
  private int blackNickMaxSize = 1000;

  private int aliasesPageSize = 20;
  private int maxAliasesTotalSize = 1000;
  private AliasFilter aliasesFilter = new AliasFilter();
  private Vector aliasesSortOrder = new Vector();

  private int subjectsPageSize = 20;
  private SubjectFilter subjectsFilter = new SubjectFilter();
  private Vector subjectsSortOrder = new Vector();

  private int usersPageSize = 20;
  private UserFilter userFilter = new UserFilter();
  private Vector usersSortOrder = new Vector();

  private int providersPageSize = 20;
  private ProviderFilter providerFilter = new ProviderFilter();
  private String providersSortOrder = "id";

  private int categoriesPageSize = 20;
  private CategoryFilter categoryFilter = new CategoryFilter();
  private String categoriesSortOrder = "id";

  private int statRoutesPageSize = 20;
  private StatRouteFilter statRouteFilter = new StatRouteFilter();
  private String statRoutesSortOrder = "Route ID";

  private int routesPageSize = 20;
  private RouteFilter routesFilter = new RouteFilter();
  private Vector routesSortOrder = new Vector();
  private boolean routeShowSrc = false;
  private boolean routeShowDst = false;

  private int closedGroupsPageSize = 20;
  private ClosedGroupFilter closedGroupFilter = new ClosedGroupFilter();
  private String closedGroupsSortOrder = "id";

  private int smsviewPageSize = 20;
  private int smsviewMaxResults = 500;
  private String smsviewSortOrder = "sendDate";

  private int localeResourcesPageSize = 20;
  private String localeResourcesSortOrder = "locale";

  private DlFilter dlFilter = new DlFilter();
  private int dlPageSize = 20;
  private String dlSortOrder = "address";

  private int dlPrincipalsPageSize = 20;
  private String dlPrincipalsSortOrder = "address";
  private PrincipalsFilter dlPrincipalsFilter = new PrincipalsFilter(null);

  private Locale locale = null;

  private HashMap topmonPrefs = new HashMap();
  private HashMap perfmonPrefs = new HashMap();
  private HashSet infosmeRegions = new HashSet();

  private TimeZone timezone = TimeZone.getDefault();

  public final static String[] WEEK_DAYS = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
  private final static String DEFAULT_ACTIVE_WEEK_DAYS = "Mon,Tue,Wed,Thu,Fri";
  private final SimpleDateFormat tf = new SimpleDateFormat("HH:mm:ss");

  // InfoSme
  private int infosmePriority;
  private boolean infosmeReplaceMessage = false;
  private String infosmeSvcType = "";
  private boolean infosmeArchive = false;
  private int infosmeArchiveTimeout = 720;
  private String infosmeSourceAddress = null;
  private Integer deliveryMode;
  private Date infosmePeriodStart = null;
  private Date infosmePeriodEnd = null;
  private Integer infosmeValidityPeriod = null;
  private Collection infosmeWeekDaysSet = new HashSet(7);
  private int infosmeCacheSize = 0;
  private int infosmeCacheSleep = 0;
  private boolean infosmeTrMode = false;
  private boolean infosmeKeepHistory = false;
  private int infosmeUncommitGeneration = 0;
  private int infosmeUncommitProcess = 0;
  private boolean infosmeTrackIntegrity;

  public static String[] getDefaultPrefsNames() {
    String[] result = new String[11];
    result[0] = "locale";
    result[1] = "topmon.graph.scale";
    result[2] = "topmon.graph.grid";
    result[3] = "topmon.graph.higrid";
    result[4] = "topmon.graph.head";
    result[5] = "topmon.max.speed";
    result[6] = "perfmon.pixPerSecond";
    result[7] = "perfmon.scale";
    result[8] = "perfmon.block";
    result[9] = "perfmon.vLightGrid";
    result[10] = "perfmon.vMinuteGrid";
    return result;
  }

  public static String[] getDefaultPrefsValues() {
    String[] result = new String[11];
    result[0] = LocaleMessages.DEFAULT_PREFERRED_LANGUAGE;
    result[1] = "3";
    result[2] = "2";
    result[3] = "10";
    result[4] = "20";
    result[5] = "50";
    result[6] = "4";
    result[7] = "80";
    result[8] = "8";
    result[9] = "4";
    result[10] = "6";
    return result;
  }

  public UserPreferences() {
    locale = new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE);
    this.profilesSortOrder = "mask";
    this.aliasesSortOrder.add("Alias");
    this.subjectsSortOrder.add("Name");
    this.routesSortOrder.add("Route ID");
    this.usersSortOrder.add("login");
    topmonPrefs.put("topmon.graph.scale", "3");
    topmonPrefs.put("topmon.graph.grid", "2");
    topmonPrefs.put("topmon.graph.higrid", "10");
    topmonPrefs.put("topmon.graph.head", "20");
    topmonPrefs.put("topmon.max.speed", "50");
    perfmonPrefs.put("perfmon.pixPerSecond", "4");
    perfmonPrefs.put("perfmon.scale", "80");
    perfmonPrefs.put("perfmon.block", "8");
    perfmonPrefs.put("perfmon.vLightGrid", "4");
    perfmonPrefs.put("perfmon.vMinuteGrid", "6");
  }

  public UserPreferences(NodeList values) throws AdminException{
    this();
    this.setValues(values);
  }

/*	public UserPreferences(SMSCAppContext appContext)
	{
		this.profilesSortOrder = "mask";
		this.aliasesSortOrder.add("Alias");
		this.subjectsSortOrder.add("Name");
		this.routesSortOrder.add("Route ID");
		this.usersSortOrder.add("login");
		this.routesFilter.setProviderManager(appContext.getProviderManager());
		this.routesFilter.setCategoryManager(appContext.getCategoryManager());
	}*/

  public int getProfilesPageSize() {
    return profilesPageSize;
  }

  public String getProfilesSortOrder() {
    return profilesSortOrder;
  }

  public void setProfilesSortOrder(String profilesSortOrder) {
    this.profilesSortOrder = profilesSortOrder;
  }

  public String getProfilesFilter() {
    return profilesFilter;
  }

  public void setProfilesFilter(String profilesFilter) {
    this.profilesFilter = profilesFilter;
  }

  public int getBlackNickPageSize() {
    return blackNickPageSize;
  }

  public int getBlackNickMaxSize() {
    return blackNickMaxSize;
  }

  public int getAliasesPageSize() {
    return aliasesPageSize;
  }

  public int getMaxAliasesTotalSize() {
    return maxAliasesTotalSize;
  }

  public AliasFilter getAliasesFilter() {
    return aliasesFilter;
  }

  public Vector getAliasesSortOrder() {
    return aliasesSortOrder;
  }

  public int getSubjectsPageSize() {
    return subjectsPageSize;
  }

  public SubjectFilter getSubjectsFilter() {
    return subjectsFilter;
  }

  public Vector getSubjectsSortOrder() {
    return subjectsSortOrder;
  }

  public int getRoutesPageSize() {
    return routesPageSize;
  }

  public RouteFilter getRoutesFilter(SMSCAppContext appContext) {
    this.routesFilter.setProviderManager(appContext.getProviderManager());
    this.routesFilter.setCategoryManager(appContext.getCategoryManager());

    return routesFilter;
  }

  public Vector getRoutesSortOrder() {
    return routesSortOrder;
  }

  public boolean isRouteShowSrc() {
    return routeShowSrc;
  }

  public void setRouteShowSrc(boolean routeShowSrc) {
    this.routeShowSrc = routeShowSrc;
  }

  public boolean isRouteShowDst() {
    return routeShowDst;
  }

  public void setRouteShowDst(boolean routeShowDst) {
    this.routeShowDst = routeShowDst;
  }

  public int getUsersPageSize() {
    return usersPageSize;
  }

  public UserFilter getUserFilter() {
    return userFilter;
  }

  public Vector getUsersSortOrder() {
    return usersSortOrder;
  }

  public int getProvidersPageSize() {
    return providersPageSize;
  }

  public void setProvidersPageSize(int providersPageSize) {
    this.providersPageSize = providersPageSize;
  }

  public ProviderFilter getProviderFilter() {
    return providerFilter;
  }

  public void setProviderFilter(ProviderFilter providerFilter) {
    this.providerFilter = providerFilter;
  }

  public String getProvidersSortOrder() {
    return providersSortOrder;
  }

  public void setProvidersSortOrder(String providersSortOrder) {
    this.providersSortOrder = providersSortOrder;
  }

  public int getCategoriesPageSize() {
    return categoriesPageSize;
  }

  public void setCategoriesPageSize(int categoriesPageSize) {
    this.categoriesPageSize = categoriesPageSize;
  }

  public CategoryFilter getCategoryFilter() {
    return categoryFilter;
  }

  public void setCategoryFilter(CategoryFilter categoryFilter) {
    this.categoryFilter = categoryFilter;
  }

  public String getCategoriesSortOrder() {
    return categoriesSortOrder;
  }

  public StatRouteFilter getStatRouteFilter() {
    return statRouteFilter;
  }

  public void setStatRouteFilter(StatRouteFilter statRouteFilter) {
    this.statRouteFilter = statRouteFilter;
  }

  public int getStatRoutesPageSize() {
    return statRoutesPageSize;
  }

  public void setStatRoutesPageSize(int statRoutesPageSize) {
    this.statRoutesPageSize = statRoutesPageSize;
  }

  public String getStatRoutesSortOrder() {
    return statRoutesSortOrder;
  }

  public void setStatRoutesSortOrder(String statRoutesSortOrder) {
    this.statRoutesSortOrder = statRoutesSortOrder;
  }

  public void setCategoriesSortOrder(String categoriesSortOrder) {
    this.categoriesSortOrder = categoriesSortOrder;
  }

  public int getSmsviewPageSize() {
    return smsviewPageSize;
  }

  public void setSmsviewPageSize(int smsviewPageSize) {
    this.smsviewPageSize = smsviewPageSize;
  }

  public int getSmsviewMaxResults() {
    return smsviewMaxResults;
  }

  public void setSmsviewMaxResults(int smsviewMaxResults) {
    this.smsviewMaxResults = smsviewMaxResults;
  }

  public String getSmsviewSortOrder() {
    return smsviewSortOrder;
  }

  public void setSmsviewSortOrder(String smsviewSortOrder) {
    this.smsviewSortOrder = smsviewSortOrder;
  }

  public Locale getLocale() {
    return locale;
  }

  public void setLocale(Locale loc) {
    locale = loc;
  }

  public int getLocaleResourcesPageSize() {
    return localeResourcesPageSize;
  }

  public void setLocaleResourcesPageSize(int localeResourcesPageSize) {
    this.localeResourcesPageSize = localeResourcesPageSize;
  }

  public String getLocaleResourcesSortOrder() {
    return localeResourcesSortOrder;
  }

  public void setLocaleResourcesSortOrder(String localeResourcesSortOrder) {
    this.localeResourcesSortOrder = localeResourcesSortOrder;
  }

  public int getDlPageSize() {
    return dlPageSize;
  }

  public void setDlPageSize(int dlPageSize) {
    this.dlPageSize = dlPageSize;
  }

  public String getDlSortOrder() {
    return dlSortOrder;
  }

  public void setDlSortOrder(String dlSortOrder) {
    this.dlSortOrder = dlSortOrder;
  }

  public DlFilter getDlFilter() {
    return dlFilter;
  }

  public void setDlFilter(DlFilter dlFilter) {
    this.dlFilter = dlFilter;
  }

  public int getDlPrincipalsPageSize() {
    return dlPrincipalsPageSize;
  }

  public void setDlPrincipalsPageSize(int dlPrincipalsPageSize) {
    this.dlPrincipalsPageSize = dlPrincipalsPageSize;
  }

  public String getDlPrincipalsSortOrder() {
    return dlPrincipalsSortOrder;
  }

  public void setDlPrincipalsSortOrder(String dlPrincipalsSortOrder) {
    this.dlPrincipalsSortOrder = dlPrincipalsSortOrder;
  }

  public PrincipalsFilter getDlPrincipalsFilter() {
    return dlPrincipalsFilter;
  }

  public void setDlPrincipalsFilter(PrincipalsFilter dlPrincipalsFilter) {
    this.dlPrincipalsFilter = dlPrincipalsFilter;
  }

  public void setValues(String[] names, String[] values) {
    for (int i = 0; i < values.length; i++) {
      if (names[i].equals("locale")) {
        locale = new Locale(values[i]);
      }
      if (names[i].startsWith("topmon.")) {
        if (topmonPrefs.containsKey(names[i])) topmonPrefs.remove(names[i]);
        topmonPrefs.put(names[i], values[i]);
      }
      if (names[i].startsWith("perfmon.")) {
        if (perfmonPrefs.containsKey(names[i])) perfmonPrefs.remove(names[i]);
        perfmonPrefs.put(names[i], values[i]);
      }
    }
  }

  public boolean isInfoSmeRegionAllowed(String regionId) {
    return infosmeRegions.contains(regionId);
  }

  public void setInfoSmeAllowedRegions(Set regions) {
    infosmeRegions.clear();
    infosmeRegions.addAll(regions);
  }

  public Collection getInfoSmeRegions() {
    return new LinkedList(infosmeRegions);
  }

  public TimeZone getTimezone() {
    return timezone;
  }

  public void setTimezone(TimeZone timezone) {
    this.timezone = timezone;
  }

  public void setValues(NodeList values) throws AdminException {

    try{
      infosmeReplaceMessage = false;
      infosmeSvcType = "dlvr";
      infosmePeriodStart = tf.parse("10:00:00");
      infosmePeriodEnd = tf.parse("21:00:00");

      infosmeValidityPeriod = new Integer(1);
      infosmeCacheSize = 2000;
      infosmeCacheSleep = 10;
      infosmeUncommitGeneration = 100;
      infosmeUncommitProcess = 100;
      infosmeTrackIntegrity = true;
      infosmeKeepHistory = true;
      infosmeWeekDaysSet = new HashSet(WEEK_DAYS.length);
      Functions.addValuesToCollection(this.infosmeWeekDaysSet, DEFAULT_ACTIVE_WEEK_DAYS, ",", true);
      infosmePriority = 10;
      infosmeSourceAddress="";


      if (values != null) {
        for (int i = 0; i < values.getLength(); i++) {
          Element elem = (Element) values.item(i);
          String name = elem.getAttribute("name");
          String value = elem.getAttribute("value");
          if (name.equals("locale")) {
            locale = new Locale(value);
          } else if (name.startsWith("topmon.")) {
            if (topmonPrefs.containsKey(name)) topmonPrefs.remove(name);
            topmonPrefs.put(name, value);
          } else if (name.startsWith("perfmon.")) {
            if (perfmonPrefs.containsKey(name)) perfmonPrefs.remove(name);
            perfmonPrefs.put(name, value);
          } else if (name.startsWith("infosme.region.")) {
            infosmeRegions.add(name.substring("infosme.region.".length()));
          } else if (name.equals("timezone")) {
            timezone = TimeZone.getTimeZone(value);
            if (timezone == null)
              timezone = TimeZone.getDefault();
          } else if(name.equals("infosme.activePeriodEnd")) {
            infosmePeriodEnd = tf.parse(value);
          } else if(name.equals("infosme.activePeriodStart")) {
            infosmePeriodStart = tf.parse(value);
          } else if(name.equals("infosme.activeWeekDays")) {
            infosmeWeekDaysSet = new HashSet(WEEK_DAYS.length);
            Functions.addValuesToCollection(infosmeWeekDaysSet, value, ",", true);
          } else if(name.equals("infosme.keepHistory")) {
            infosmeKeepHistory = Boolean.valueOf(value).booleanValue();
          } else if(name.equals("infosme.messagesCacheSize")) {
            infosmeCacheSize = Integer.parseInt(value);
          } else if(name.equals("infosme.messagesCacheSleep")) {
            infosmeCacheSleep = Integer.parseInt(value);
          } else if(name.equals("infosme.replaceMessage")) {
            infosmeReplaceMessage = Boolean.valueOf(value).booleanValue();
          } else if(name.equals("infosme.svcType")) {
            infosmeSvcType = value;
          } else if(name.equals("infosme.archive")) {
            infosmeArchive = Boolean.valueOf(value).booleanValue();
          } else if(name.equals("infosme.archiveTimeout")) {
            infosmeArchiveTimeout = Integer.parseInt(value);
          } else if(name.equals("infosme.trackIntegrity")) {
            infosmeTrackIntegrity = Boolean.valueOf(value).booleanValue();
          } else if(name.equals("infosme.transactionMode")) {
            infosmeTrMode = Boolean.valueOf(value).booleanValue();
          } else if(name.equals("infosme.uncommitedInGeneration")) {
            infosmeUncommitGeneration = Integer.parseInt(value);
          } else if(name.equals("infosme.uncommitedInProcess")) {
            infosmeUncommitProcess = Integer.parseInt(value);
          } else if(name.equals("infosme.validityPeriod")) {
            infosmeValidityPeriod = Integer.valueOf(value.substring(0, value.indexOf(":")));
          } else if(name.equals("infosme.priority")) {
            infosmePriority = Integer.parseInt(value);
          } else if(name.equals("infosme.sourceAddress")) {
            infosmeSourceAddress = value;
          } else if(name.equals("infosme.ussdPush")) {
            Boolean infosmeUssdPush = Boolean.valueOf(value);
            if (infosmeUssdPush.booleanValue() && deliveryMode == null)
              deliveryMode = new Integer(1);
          } else if(name.equals("infosme.deliveryMode")) {
            deliveryMode = new Integer(value);
          }
        }
      }
    }catch(Throwable e) {
      throw new AdminException("Error during set preferences",e);
    }
  }

  public String getXmlText() {
    String result = "";
    result += "<pref name=\"locale\" value=\"" + this.locale.getLanguage() + "\"/>\n";
    Set t = topmonPrefs.keySet();
    for (Iterator i = t.iterator(); i.hasNext();) {
      String name = (String) i.next();
      result += "\t\t<pref name=\"" + name + "\" value=\"" + topmonPrefs.get(name) + "\"/>\n";
    }
    t = perfmonPrefs.keySet();
    for (Iterator i = t.iterator(); i.hasNext();) {
      String name = (String) i.next();
      result += "\t\t<pref name=\"" + name + "\" value=\"" + perfmonPrefs.get(name) + "\"/>\n";
    }
    t = infosmeRegions;
    for (Iterator i = t.iterator(); i.hasNext();) {
      String name = (String) i.next();
      result += "\t\t<pref name=\"infosme.region." + name + "\" value=\"true\"/>\n";
    }
    result += "\t\t<pref name=\"timezone\" value=\"" + timezone.getID() + "\"/>\n";

    result += "\t\t<pref name=\"infosme.activePeriodEnd\" value=\"" + tf.format(infosmePeriodEnd) + "\"/>\n";
    result += "\t\t<pref name=\"infosme.activePeriodStart\" value=\"" + tf.format(infosmePeriodStart) + "\"/>\n";
    result += "\t\t<pref name=\"infosme.activeWeekDays\" value=\"" + Functions.collectionToString(infosmeWeekDaysSet, ",") + "\"/>\n";
    result += "\t\t<pref name=\"infosme.keepHistory\" value=\"" + infosmeKeepHistory + "\"/>\n";
    result += "\t\t<pref name=\"infosme.messagesCacheSize\" value=\"" + infosmeCacheSize + "\"/>\n";
    result += "\t\t<pref name=\"infosme.messagesCacheSleep\" value=\"" + infosmeCacheSleep + "\"/>\n";
    result += "\t\t<pref name=\"infosme.replaceMessage\" value=\"" + infosmeReplaceMessage + "\"/>\n";
    result += "\t\t<pref name=\"infosme.svcType\" value=\"" + infosmeSvcType + "\"/>\n";
    result += "\t\t<pref name=\"infosme.archive\" value=\"" + infosmeArchive + "\"/>\n";
    result += "\t\t<pref name=\"infosme.archiveTimeout\" value=\"" + infosmeArchiveTimeout + "\"/>\n";
    result += "\t\t<pref name=\"infosme.sourceAddress\" value=\"" + infosmeSourceAddress + "\"/>\n";
    if(deliveryMode != null) {
      result += "\t\t<pref name=\"infosme.deliveryMode\" value=\"" + deliveryMode + "\"/>\n";
    }
    result += "\t\t<pref name=\"infosme.trackIntegrity\" value=\"" + infosmeTrackIntegrity + "\"/>\n";
    result += "\t\t<pref name=\"infosme.transactionMode\" value=\"" + infosmeTrMode + "\"/>\n";
    result += "\t\t<pref name=\"infosme.uncommitedInGeneration\" value=\"" + infosmeUncommitGeneration + "\"/>\n";
    result += "\t\t<pref name=\"infosme.uncommitedInProcess\" value=\"" + infosmeUncommitProcess + "\"/>\n";
    result += "\t\t<pref name=\"infosme.validityPeriod\" value=\"" + infosmeValidityPeriod+":00:00" + "\"/>\n";
    result += "\t\t<pref name=\"infosme.priority\" value=\"" + infosmePriority + "\"/>\n";

    return result;
  }

  public String[] getPrefsValues() {
    String[] result = getDefaultPrefsValues();
    result[0] = locale.getLanguage();
    if (topmonPrefs.containsKey("topmon.graph.scale")) result[1] = (String) topmonPrefs.get("topmon.graph.scale");
    if (topmonPrefs.containsKey("topmon.graph.grid")) result[2] = (String) topmonPrefs.get("topmon.graph.grid");
    if (topmonPrefs.containsKey("topmon.graph.higrid")) result[3] = (String) topmonPrefs.get("topmon.graph.higrid");
    if (topmonPrefs.containsKey("topmon.graph.head")) result[4] = (String) topmonPrefs.get("topmon.graph.head");
    if (topmonPrefs.containsKey("topmon.max.speed")) result[5] = (String) topmonPrefs.get("topmon.max.speed");
    if (perfmonPrefs.containsKey("perfmon.pixPerSecond")) result[6] = (String) perfmonPrefs.get("perfmon.pixPerSecond");
    if (perfmonPrefs.containsKey("perfmon.scale")) result[7] = (String) perfmonPrefs.get("perfmon.scale");
    if (perfmonPrefs.containsKey("perfmon.block")) result[8] = (String) perfmonPrefs.get("perfmon.block");
    if (perfmonPrefs.containsKey("perfmon.vLightGrid")) result[9] = (String) perfmonPrefs.get("perfmon.vLightGrid");
    if (perfmonPrefs.containsKey("perfmon.vMinuteGrid")) result[10] = (String) perfmonPrefs.get("perfmon.vMinuteGrid");
    return result;
  }

  public HashMap getTopmonPrefs() {
    return topmonPrefs;
  }

  public HashMap getPerfmonPrefs() {
    return perfmonPrefs;
  }

  public int getClosedGroupsPageSize() {
    return closedGroupsPageSize;
  }

  public void setClosedGroupFilter(ClosedGroupFilter closedGroupFilter) {
    this.closedGroupFilter = closedGroupFilter;
  }

  public ClosedGroupFilter getClosedGroupFilter() {
    return closedGroupFilter;
  }

  public String getClosedGroupsSortOrder() {
    return closedGroupsSortOrder;
  }

  public void setClosedGroupsSortOrder(String sort) {
    this.closedGroupsSortOrder = sort;
  }

  public boolean isInfosmeReplaceMessage() {
    return infosmeReplaceMessage;
  }

  public void setInfosmeReplaceMessage(boolean infosmeReplaceMessage) {
    this.infosmeReplaceMessage = infosmeReplaceMessage;
  }

  public String getInfosmeSvcType() {
    return infosmeSvcType;
  }

  public void setInfosmeSvcType(String infosmeSvcType) {
    this.infosmeSvcType = infosmeSvcType;
  }

  public boolean isInfosmeArchive() {
    return infosmeArchive;
  }

  public void setInfosmeArchive(boolean infosmeArchive) {
    this.infosmeArchive = infosmeArchive;
  }

  public int getInfosmeArchiveTimeout() {
    return infosmeArchiveTimeout;
  }

  public void setInfosmeArchiveTimeout(int infosmeArchiveTimeout) {
    this.infosmeArchiveTimeout = infosmeArchiveTimeout;
  }

  public Date getInfosmePeriodStart() {
    return new Date(infosmePeriodStart.getTime());
  }

  public void setInfosmePeriodStart(Date infosmePeriodStart) {
    this.infosmePeriodStart = infosmePeriodStart;
  }

  public Date getInfosmePeriodEnd() {
    return new Date(infosmePeriodEnd.getTime());
  }

  public void setInfosmePeriodEnd(Date infosmePeriodEnd) {
    this.infosmePeriodEnd = infosmePeriodEnd;
  }

  public Collection getInfosmeWeekDaysSet() {
    return new LinkedList(infosmeWeekDaysSet);
  }

  public void setInfosmeWeekDaysSet(Collection infosmeWeekDaysSet) {
    this.infosmeWeekDaysSet = infosmeWeekDaysSet;
  }

  public int getInfosmeCacheSize() {
    return infosmeCacheSize;
  }

  public void setInfosmeCacheSize(int infosmeCacheSize) {
    this.infosmeCacheSize = infosmeCacheSize;
  }

  public int getInfosmeCacheSleep() {
    return infosmeCacheSleep;
  }

  public void setInfosmeCacheSleep(int infosmeCacheSleep) {
    this.infosmeCacheSleep = infosmeCacheSleep;
  }

  public boolean isInfosmeTrMode() {
    return infosmeTrMode;
  }

  public void setInfosmeTrMode(boolean infosmeTrMode) {
    this.infosmeTrMode = infosmeTrMode;
  }

  public boolean isInfosmeKeepHistory() {
    return infosmeKeepHistory;
  }

  public void setInfosmeKeepHistory(boolean infosmeKeepHistory) {
    this.infosmeKeepHistory = infosmeKeepHistory;
  }

  public int getInfosmeUncommitGeneration() {
    return infosmeUncommitGeneration;
  }

  public void setInfosmeUncommitGeneration(int infosmeUncommitGeneration) {
    this.infosmeUncommitGeneration = infosmeUncommitGeneration;
  }

  public String getInfosmeSourceAddress() {
    return infosmeSourceAddress;
  }

  public void setInfosmeSourceAddress(String infosmeSourceAddress) {
    this.infosmeSourceAddress = infosmeSourceAddress;
  }

  public Integer getDeliveryMode() {
    return deliveryMode;
  }

  public void setDeliveryMode(int deliveryMode) {
    this.deliveryMode = new Integer(deliveryMode);
  }

  public int getInfosmeUncommitProcess() {
    return infosmeUncommitProcess;
  }

  public void setInfosmeUncommitProcess(int infosmeUncommitProcess) {
    this.infosmeUncommitProcess = infosmeUncommitProcess;
  }

  public boolean isInfosmeTrackIntegrity() {
    return infosmeTrackIntegrity;
  }

  public void setInfosmeTrackIntegrity(boolean infosmeTrackIntegrity) {
    this.infosmeTrackIntegrity = infosmeTrackIntegrity;
  }

  public Integer getInfosmeValidityPeriod() {
    return infosmeValidityPeriod;
  }

  public void setInfosmeValidityPeriod(Integer infosmeValidityPeriod) {
    this.infosmeValidityPeriod = infosmeValidityPeriod;
  }

  public int getInfosmePriority() {
    return infosmePriority;
  }

  public void setInfosmePriority(int infosmePriority) {
    this.infosmePriority = infosmePriority;
  }
}
