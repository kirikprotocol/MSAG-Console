package ru.novosoft.smsc.admin.preferences;

/*
 * Created by igork
 * Date: 05.11.2002
 * Time: 19:06:21
 */

//todo Надо бы сделать контейнер для topmonPrefs, perfmonPrefs и других будущих хэшмапных префов, чтобы код не воротить каждый раз

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
import ru.novosoft.smsc.util.LocaleMessages;

import java.util.*;

import org.w3c.dom.NodeList;
import org.w3c.dom.Element;

public class UserPreferences
{
  private int profilesPageSize = 20;
  private String profilesSortOrder = null;
  private String profilesFilter = null;

  private int aliasesPageSize = 20;
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

	public static String[] getDefaultPrefsNames()
	{
		String[] result = new String[11];
		result[0]  = "locale";
		result[1]  = "topmon.graph.scale";
		result[2]  = "topmon.graph.grid";
		result[3]  = "topmon.graph.higrid";
		result[4]  = "topmon.graph.head";
		result[5]  = "topmon.max.speed";
		result[6]  = "perfmon.pixPerSecond";
		result[7]  = "perfmon.scale";
		result[8]  = "perfmon.block";
		result[9]  = "perfmon.vLightGrid";
		result[10] = "perfmon.vMinuteGrid";
		return result;
	}

	public static String[] getDefaultPrefsValues()
	{
		String[] result = new String[11];
		result[0] = LocaleMessages.DEFAULT_PREFERRED_LANGUAGE;
		result[1]  = "3";
		result[2]  = "2";
		result[3]  = "10";
		result[4]  = "20";
		result[5]  = "50";
		result[6]  = "4";
		result[7]  = "80";
		result[8]  = "8";
		result[9]  = "4";
		result[10] = "6";
		return result;
	}

	public UserPreferences()
	{
		locale = new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE);
		this.profilesSortOrder = "mask";
		this.aliasesSortOrder.add("Alias");
		this.subjectsSortOrder.add("Name");
		this.routesSortOrder.add("Route ID");
		this.usersSortOrder.add("login");
		topmonPrefs.put("topmon.graph.scale","3");
		topmonPrefs.put("topmon.graph.grid","2");
		topmonPrefs.put("topmon.graph.higrid","10");
		topmonPrefs.put("topmon.graph.head","20");
		topmonPrefs.put("topmon.max.speed","50");
		perfmonPrefs.put("perfmon.pixPerSecond","4");
		perfmonPrefs.put("perfmon.scale","80");
		perfmonPrefs.put("perfmon.block","8");
		perfmonPrefs.put("perfmon.vLightGrid","4");
		perfmonPrefs.put("perfmon.vMinuteGrid","6");
	}

	public UserPreferences(NodeList values)
	{
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

  public int getProfilesPageSize()
  {
    return profilesPageSize;
  }

  public String getProfilesSortOrder()
  {
    return profilesSortOrder;
  }

  public void setProfilesSortOrder(String profilesSortOrder)
  {
    this.profilesSortOrder = profilesSortOrder;
  }

  public String getProfilesFilter()
  {
    return profilesFilter;
  }

  public void setProfilesFilter(String profilesFilter)
  {
    this.profilesFilter = profilesFilter;
  }

  public int getAliasesPageSize()
  {
    return aliasesPageSize;
  }

  public AliasFilter getAliasesFilter()
  {
    return aliasesFilter;
  }

  public Vector getAliasesSortOrder()
  {
    return aliasesSortOrder;
  }

  public int getSubjectsPageSize()
  {
    return subjectsPageSize;
  }

  public SubjectFilter getSubjectsFilter()
  {
    return subjectsFilter;
  }

  public Vector getSubjectsSortOrder()
  {
    return subjectsSortOrder;
  }

  public int getRoutesPageSize()
  {
    return routesPageSize;
  }

	public RouteFilter getRoutesFilter(SMSCAppContext appContext)
	{
		this.routesFilter.setProviderManager(appContext.getProviderManager());
		this.routesFilter.setCategoryManager(appContext.getCategoryManager());

		return routesFilter;
	}

  public Vector getRoutesSortOrder()
  {
    return routesSortOrder;
  }

  public boolean isRouteShowSrc()
  {
    return routeShowSrc;
  }

  public void setRouteShowSrc(boolean routeShowSrc)
  {
    this.routeShowSrc = routeShowSrc;
  }

  public boolean isRouteShowDst()
  {
    return routeShowDst;
  }

  public void setRouteShowDst(boolean routeShowDst)
  {
    this.routeShowDst = routeShowDst;
  }

  public int getUsersPageSize()
  {
    return usersPageSize;
  }

  public UserFilter getUserFilter()
  {
    return userFilter;
  }

  public Vector getUsersSortOrder()
  {
    return usersSortOrder;
  }

  public int getProvidersPageSize()
  {
    return providersPageSize;
  }

  public void setProvidersPageSize(int providersPageSize)
  {
    this.providersPageSize = providersPageSize;
  }

  public ProviderFilter getProviderFilter()
  {
    return providerFilter;
  }

  public void setProviderFilter(ProviderFilter providerFilter)
  {
    this.providerFilter = providerFilter;
  }

  public String getProvidersSortOrder()
  {
    return providersSortOrder;
  }

  public void setProvidersSortOrder(String providersSortOrder)
  {
    this.providersSortOrder = providersSortOrder;
  }

  public int getCategoriesPageSize()
  {
    return categoriesPageSize;
  }

  public void setCategoriesPageSize(int categoriesPageSize)
  {
    this.categoriesPageSize = categoriesPageSize;
  }

  public CategoryFilter getCategoryFilter()
  {
    return categoryFilter;
  }

  public void setCategoryFilter(CategoryFilter categoryFilter)
  {
    this.categoryFilter = categoryFilter;
  }

  public String getCategoriesSortOrder()
  {
    return categoriesSortOrder;
  }

  public StatRouteFilter getStatRouteFilter()
  {
    return statRouteFilter;
  }

  public void setStatRouteFilter(StatRouteFilter statRouteFilter)
  {
    this.statRouteFilter = statRouteFilter;
  }

  public int getStatRoutesPageSize()
  {
    return statRoutesPageSize;
  }

  public void setStatRoutesPageSize(int statRoutesPageSize)
  {
    this.statRoutesPageSize = statRoutesPageSize;
  }

  public String getStatRoutesSortOrder()
  {
    return statRoutesSortOrder;
  }

  public void setStatRoutesSortOrder(String statRoutesSortOrder)
  {
    this.statRoutesSortOrder = statRoutesSortOrder;
  }

  public void setCategoriesSortOrder(String categoriesSortOrder)
  {
    this.categoriesSortOrder = categoriesSortOrder;
  }

  public int getSmsviewPageSize()
  {
    return smsviewPageSize;
  }

  public void setSmsviewPageSize(int smsviewPageSize)
  {
    this.smsviewPageSize = smsviewPageSize;
  }

  public int getSmsviewMaxResults()
  {
    return smsviewMaxResults;
  }

  public void setSmsviewMaxResults(int smsviewMaxResults)
  {
    this.smsviewMaxResults = smsviewMaxResults;
  }

  public String getSmsviewSortOrder()
  {
    return smsviewSortOrder;
  }

  public void setSmsviewSortOrder(String smsviewSortOrder)
  {
    this.smsviewSortOrder = smsviewSortOrder;
  }

  public Locale getLocale()
  {
    return locale;
  }

  public void setLocale(Locale loc)
  {
    locale = loc;
  }

  public int getLocaleResourcesPageSize()
  {
    return localeResourcesPageSize;
  }

  public void setLocaleResourcesPageSize(int localeResourcesPageSize)
  {
    this.localeResourcesPageSize = localeResourcesPageSize;
  }

  public String getLocaleResourcesSortOrder()
  {
    return localeResourcesSortOrder;
  }

  public void setLocaleResourcesSortOrder(String localeResourcesSortOrder)
  {
    this.localeResourcesSortOrder = localeResourcesSortOrder;
  }

  public int getDlPageSize()
  {
    return dlPageSize;
  }

  public void setDlPageSize(int dlPageSize)
  {
    this.dlPageSize = dlPageSize;
  }

  public String getDlSortOrder()
  {
    return dlSortOrder;
  }

  public void setDlSortOrder(String dlSortOrder)
  {
    this.dlSortOrder = dlSortOrder;
  }

  public DlFilter getDlFilter()
  {
    return dlFilter;
  }

  public void setDlFilter(DlFilter dlFilter)
  {
    this.dlFilter = dlFilter;
  }

  public int getDlPrincipalsPageSize()
  {
    return dlPrincipalsPageSize;
  }

  public void setDlPrincipalsPageSize(int dlPrincipalsPageSize)
  {
    this.dlPrincipalsPageSize = dlPrincipalsPageSize;
  }

  public String getDlPrincipalsSortOrder()
  {
    return dlPrincipalsSortOrder;
  }

  public void setDlPrincipalsSortOrder(String dlPrincipalsSortOrder)
  {
    this.dlPrincipalsSortOrder = dlPrincipalsSortOrder;
  }

  public PrincipalsFilter getDlPrincipalsFilter()
  {
    return dlPrincipalsFilter;
  }

  public void setDlPrincipalsFilter(PrincipalsFilter dlPrincipalsFilter)
  {
    this.dlPrincipalsFilter = dlPrincipalsFilter;
  }
	public void setValues(String[] names, String[] values)
	{
		for (int i = 0; i < values.length; i++)
		{
			if (names[i].equals("locale")) {locale = new Locale(values[i]);}
			if (names[i].startsWith("topmon."))
			{
				if (topmonPrefs.containsKey(names[i])) topmonPrefs.remove(names[i]);
				topmonPrefs.put(names[i], values[i]);
			}
			if (names[i].startsWith("perfmon."))
			{
				if (perfmonPrefs.containsKey(names[i])) perfmonPrefs.remove(names[i]);
				perfmonPrefs.put(names[i], values[i]);
			}
		}
	}

	public void setValues(NodeList values)
	{
		if (values != null) 
		{
			for (int i = 0; i < values.getLength(); i++)
			{
				Element elem = (Element) values.item(i);
				String name = elem.getAttribute("name");
				String value = elem.getAttribute("value");
				if (name.equals("locale")) {locale = new Locale(value);}
				if (name.startsWith("topmon."))
				{
					if (topmonPrefs.containsKey(name)) topmonPrefs.remove(name);
					topmonPrefs.put(name, value);
				}
				if (name.startsWith("perfmon."))
				{
					if (perfmonPrefs.containsKey(name)) perfmonPrefs.remove(name);
					perfmonPrefs.put(name, value);
				}
			}
		}
	}
	public String getXmlText()
	{
		String result = "";
		result += "<pref name=\"locale\" value=\"" + this.locale.getLanguage() + "\"/>\n";
		Set t = topmonPrefs.keySet();
		for(Iterator i = t.iterator(); i.hasNext();)
		{
			String name = (String) i.next();
			result += "\t\t<pref name=\"" + name + "\" value=\"" + topmonPrefs.get(name) + "\"/>\n";
		}
		t = perfmonPrefs.keySet();
		for(Iterator i = t.iterator(); i.hasNext();)
		{
			String name = (String) i.next();
			result += "\t\t<pref name=\"" + name + "\" value=\"" + perfmonPrefs.get(name) + "\"/>\n";
		}

		return result;
	}

	public String[] getPrefsValues()
	{
		String[] result = new String[11];
		result = getDefaultPrefsValues();
		result[0]  = locale.getLanguage();
		if (topmonPrefs.containsKey("topmon.graph.scale"))  result[1]  = (String) topmonPrefs.get("topmon.graph.scale");
		if (topmonPrefs.containsKey("topmon.graph.grid"))   result[2]  = (String) topmonPrefs.get("topmon.graph.grid");
		if (topmonPrefs.containsKey("topmon.graph.higrid")) result[3]  = (String) topmonPrefs.get("topmon.graph.higrid");
		if (topmonPrefs.containsKey("topmon.graph.head"))   result[4]  = (String) topmonPrefs.get("topmon.graph.head");
		if (topmonPrefs.containsKey("topmon.max.speed"))    result[5]  = (String) topmonPrefs.get("topmon.max.speed");
		if (perfmonPrefs.containsKey("perfmon.pixPerSecond")) result[6]  = (String) perfmonPrefs.get("perfmon.pixPerSecond");
		if (perfmonPrefs.containsKey("perfmon.scale"))        result[7]  = (String) perfmonPrefs.get("perfmon.scale");
		if (perfmonPrefs.containsKey("perfmon.block"))        result[8]  = (String) perfmonPrefs.get("perfmon.block");
		if (perfmonPrefs.containsKey("perfmon.vLightGrid"))   result[9]  = (String) perfmonPrefs.get("perfmon.vLightGrid");
		if (perfmonPrefs.containsKey("perfmon.vMinuteGrid"))  result[10] = (String) perfmonPrefs.get("perfmon.vMinuteGrid");
		return result;
	}

	public HashMap getTopmonPrefs() {
		return topmonPrefs;
	}

	public HashMap getPerfmonPrefs() {
		return perfmonPrefs;
	}
}
