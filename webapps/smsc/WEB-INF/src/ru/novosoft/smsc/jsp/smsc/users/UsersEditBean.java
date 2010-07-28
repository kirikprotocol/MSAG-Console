package ru.novosoft.smsc.jsp.smsc.users;

/**
 * Created by igork
 * Date: Dec 2, 2002
 * Time: 4:59:33 PM
 */

import ru.novosoft.smsc.admin.users.UserManager;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.xml.WebXml;

import javax.servlet.http.HttpServletRequest;
import java.util.*;
import java.text.SimpleDateFormat;
import java.lang.reflect.InvocationTargetException;

public abstract class UsersEditBean extends SmscBean {
  protected UserManager userManager = null;
  protected String login = null;
  protected String password = null;
  protected String confirmPassword = null;
  protected String[] roles = null;
  protected String firstName = null;
  protected String lastName = null;
  protected String dept = null;
  protected String workPhone = null;
  protected String homePhone = null;
  protected String cellPhone = null;
  protected String email = null;
  protected String mbCancel = null;
  protected String mbSave = null;
  protected Set rolesSet = new HashSet();
  protected List serviceRoles = new LinkedList();
  protected String[] prefsNames = null;
  protected String[] prefsValues = null;
  protected String timezone;
  protected UserPreferences userPrefs;

  protected SimpleDateFormat tf = new SimpleDateFormat("HH:mm:ss");

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    userManager = appContext.getUserManager();
    serviceRoles = appContext.getWebXmlConfig().getRoleNames();
    return RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    User user = userManager.getUser(login);
    if (user != null)
      userPrefs = user.getPrefs();



    if (mbCancel != null)
      return RESULT_DONE;
    else if (mbSave != null)
      return save(request);

    return RESULT_OK;
  }

  protected Set getInfoSmeRegions(HttpServletRequest request) {
    Set infosmeRegions = new HashSet(100);
    for (Iterator iter = request.getParameterMap().entrySet().iterator(); iter.hasNext();) {
      Map.Entry e = (Map.Entry)iter.next();
      String key = (String)e.getKey();
      if (key.startsWith("infosme.region."))
        infosmeRegions.add(key.substring("infosme.region.".length()));
    }
    return infosmeRegions;
  }

  protected abstract int save(final HttpServletRequest request);

  public abstract boolean isNew();

  public boolean isUserInRole(String rolename) {
    return rolesSet.contains(rolename);
  }

  public String getServiceIdFromRole(String roleName) {
    return WebXml.getServiceIdFromRole(roleName);
  }

  /**
   * *************************************** properties ***************************************************
   */
  public String getLogin() {
    return login;
  }

  public void setLogin(String login) {
    this.login = login;
  }

  public void setPassword(String password) {
    this.password = password;
  }

  public void setConfirmPassword(String confirmPassword) {
    this.confirmPassword = confirmPassword;
  }

  public String[] getRoles() {
    return roles;
  }

  public void setRoles(String[] roles) {
    if (roles == null)
      roles = new String[0];
    this.roles = roles;
    rolesSet.clear();
    rolesSet.addAll(Arrays.asList(roles));
  }

  public String getFirstName() {
    return firstName;
  }

  public void setFirstName(String firstName) {
    this.firstName = firstName;
  }

  public String getLastName() {
    return lastName;
  }

  public void setLastName(String lastName) {
    this.lastName = lastName;
  }

  public String getDept() {
    return dept;
  }

  public void setDept(String dept) {
    this.dept = dept;
  }

  public String getWorkPhone() {
    return workPhone;
  }

  public void setWorkPhone(String workPhone) {
    this.workPhone = workPhone;
  }

  public String getHomePhone() {
    return homePhone;
  }

  public void setHomePhone(String homePhone) {
    this.homePhone = homePhone;
  }

  public String getCellPhone() {
    return cellPhone;
  }

  public void setCellPhone(String cellPhone) {
    this.cellPhone = cellPhone;
  }

  public String getEmail() {
    return email;
  }

  public void setEmail(String email) {
    this.email = email;
  }

  public String getMbCancel() {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }

  public String getMbSave() {
    return mbSave;
  }

  public void setMbSave(String mbSave) {
    this.mbSave = mbSave;
  }

  public List getServiceRoles() {
    return serviceRoles;
  }

  public String[] getPrefsNames() {
    return prefsNames;
  }

  public void setPrefsNames(String[] names) {
    for (int i = 0; i<names.length; i++)
      System.out.println("Pref = " + names[i]);
    this.prefsNames = names;
  }

  public String[] getPrefsValues() {
    return prefsValues;
  }

  public void setPrefsValues(String[] values) {
    for (int i = 0; i<values.length; i++)
      System.out.println("Pref Value = " + values[i]);
    this.prefsValues = values;
  }

  public String getPref(String prefName) {
    String result = "";
    for (int i = 0; i < prefsNames.length; i++) {
      if (prefsNames[i].equals(prefName)) {
        result = prefsValues[i];
        break;
      }
    }
    return result;
  }

  public Collection getRegions() {
    return appContext.getRegionsManager().getRegions();
  }

  public boolean isInfoSmeRegionAllowed(String regionId) {
    return userPrefs != null && userPrefs.isInfoSmeRegionAllowed(regionId);
  }

  public String getTimezone() {
    return timezone;
  }

  public void setTimezone(String timezone) {
    this.timezone = timezone;
  }

  public Collection getTimezones() {
    List c = Arrays.asList(TimeZone.getAvailableIDs());
    Collections.sort(c);
    return c;
  }


  protected int infosmePriority = 10;
  protected String infosmeValidityPeriod = "1";
  protected boolean infosmeReplaceMessage = false;
  protected String infosmeSvcType = "";
  protected String infosmeActivePeriodStart = "10:00:00";
  protected String infosmeActivePeriodEnd = "20:00:00";
  protected String infosmeSourceAddress = "";
  protected Integer deliveryMode;
  protected String[] infosmeActiveWeekDays = new String[]{"Mon","Tue","Wed","Thu","Fri"};

  protected int infosmeMessagesCacheSize = 2000;
  protected int infosmeMessagesCacheSleep = 10;
  protected boolean infosmeTransactionMode = false;
  protected int infosmeUncommitedInGeneration = 100;
  protected int infosmeUncommitedInProcess = 100;
  protected boolean infosmeTrackIntegrity = false;
  protected boolean infosmeKeepHistory = false;

  public boolean isInfosmeTransactionMode() {
    return infosmeTransactionMode;
  }

  public void setInfosmeTransactionMode(boolean infosmeTransactionMode) {
    this.infosmeTransactionMode = infosmeTransactionMode;
  }

  public String getInfosmeActivePeriodEnd() {
    return infosmeActivePeriodEnd;
  }

  public void setInfosmeActivePeriodEnd(String infosmeActivePeriodEnd) {
    this.infosmeActivePeriodEnd = infosmeActivePeriodEnd;
  }

  public String getInfosmeActivePeriodStart() {
    return infosmeActivePeriodStart;
  }

  public void setInfosmeActivePeriodStart(String infosmeActivePeriodStart) {
    this.infosmeActivePeriodStart = infosmeActivePeriodStart;
  }

  public String[] getInfosmeActiveWeekDays() {
    return infosmeActiveWeekDays;
  }
  public void setInfosmeActiveWeekDays(String[] activeWeekDays) {
    this.infosmeActiveWeekDays = activeWeekDays;
  }

  public boolean isWeekDayActive(String weekday) {
    for(int i=0; i<infosmeActiveWeekDays.length; i++)
      if (infosmeActiveWeekDays[i].equals(weekday))
        return true;
    return false;
  }

  public boolean isInfosmeReplaceMessage() {
    return infosmeReplaceMessage;
  }

  public void setInfosmeReplaceMessage(boolean replaceMessage) {
    this.infosmeReplaceMessage = replaceMessage;
  }

  public String getInfosmeSvcType() {
    return infosmeSvcType;
  }

  public void setInfosmeSvcType(String svcType) {
    this.infosmeSvcType = svcType;
  }

  public String getInfosmeSourceAddress() {
    return infosmeSourceAddress;
  }

  public void setInfosmeSourceAddress(String infosmeSourceAddress) {
    this.infosmeSourceAddress = infosmeSourceAddress;
  }

  public int getDeliveryMode() {
    return deliveryMode == null ? 0 : deliveryMode.intValue();
  }

  public void setDeliveryMode(int deliveryMode) {
    this.deliveryMode = new Integer(deliveryMode);
  }

  public String getInfosmeMessagesCacheSize() {
    return Integer.toString(infosmeMessagesCacheSize);
  }

  public String getInfosmePriority() {
    return Integer.toString(infosmePriority);
  }

  public void setInfosmePriority(String infosmePriority) {
    try{
      this.infosmePriority = Integer.parseInt(infosmePriority);
    }catch(NumberFormatException e) {
      logger.error(e,e);
    }
  }

  public String getInfosmeValidityPeriod() {
    return infosmeValidityPeriod;
  }

  public void setInfosmeValidityPeriod(String infosmeValidityPeriod) {
    this.infosmeValidityPeriod = infosmeValidityPeriod;
  }

  public void setInfosmeMessagesCacheSize(String infosmeMessagesCacheSize) {
    if(infosmeMessagesCacheSize != null && infosmeMessagesCacheSize.length()>0) {
      try{
        this.infosmeMessagesCacheSize = Integer.parseInt(infosmeMessagesCacheSize);
      }catch(NumberFormatException e) {
        logger.error(e,e);
      }
    }
  }

  public String getInfosmeMessagesCacheSleep() {
    return Integer.toString(infosmeMessagesCacheSleep);
  }

  public void setInfosmeMessagesCacheSleep(String infosmeMessagesCacheSleep) {
    if(infosmeMessagesCacheSleep != null && infosmeMessagesCacheSleep.length()>0) {
      try{
        this.infosmeMessagesCacheSleep = Integer.parseInt(infosmeMessagesCacheSleep);
      }catch(NumberFormatException e) {
        logger.error(e,e);
      }
    }
  }

  public String getInfosmeUncommitedInGeneration() {
    return Integer.toString(infosmeUncommitedInGeneration);
  }

  public void setInfosmeUncommitedInGeneration(String infosmeUncommitedInGeneration) {
    if(infosmeUncommitedInGeneration != null && infosmeUncommitedInGeneration.length()>0) {
      try{
        this.infosmeUncommitedInGeneration = Integer.parseInt(infosmeUncommitedInGeneration);
      }catch(NumberFormatException e) {
        logger.error(e,e);
      }
    }
  }

  public String getInfosmeUncommitedInProcess() {
    return Integer.toString(infosmeUncommitedInProcess);
  }

  public void setInfosmeUncommitedInProcess(String infosmeUncommitedInProcess) {
    if(infosmeUncommitedInProcess != null && infosmeUncommitedInProcess.length()>0) {
      try{
        this.infosmeUncommitedInProcess = Integer.parseInt(infosmeUncommitedInProcess);
      }catch(NumberFormatException e) {
        logger.error(e,e);
      }
    }
  }

  public boolean isInfosmeTrackIntegrity() {
    return infosmeTrackIntegrity;
  }

  public void setInfosmeTrackIntegrity(boolean infosmeTrackIntegrity) {
    this.infosmeTrackIntegrity = infosmeTrackIntegrity;
  }

  public boolean isInfosmeKeepHistory() {
    return infosmeKeepHistory;
  }

  public void setInfosmeKeepHistory(boolean infosmeKeepHistory) {
    this.infosmeKeepHistory = infosmeKeepHistory;
  }

  private static Object infoSmeContext;

  public boolean isInfoSmeEmbeded() {
    return loadInfoSme() != null;
  }

  protected String validateInfosmePrefs(UserPreferences preferences) throws Exception{
    try {
      Object ctx = loadInfoSme();
      if (ctx != null)
        return (String)ctx.getClass().getMethod("validateInfosmePrefs", new Class[]{UserPreferences.class}).invoke(infoSmeContext, new Object[]{preferences});
      else
        return null;
    } catch (NoSuchMethodException e) {
      return null;
    } catch (IllegalAccessException e) {
      return null;
    } catch (InvocationTargetException e) {
      return null;
    }
  }

  private Object loadInfoSme()  {
    if(infoSmeContext == null) {
      try {
        infoSmeContext = Class.forName("ru.novosoft.smsc.infosme.backend.InfoSmeContext").
            getMethod("getInstance", new Class[]{SMSCAppContext.class, String.class}).
            invoke(null, new Object[]{appContext, "InfoSme"});
      } catch (IllegalAccessException e) {
        return null;
      } catch (InvocationTargetException e) {
        return null;
      } catch (NoSuchMethodException e) {
        return null;
      } catch (ClassNotFoundException e) {
        return null;
      }
    }
    return infoSmeContext;
  }

  public boolean isUssdPushFeature() {
    try{
      Object ctx = loadInfoSme();
      if (ctx == null)
        return false;
      Boolean result = (Boolean)loadInfoSme().getClass().getMethod("getUssdFeature", new Class[]{}).invoke(infoSmeContext, new Object[]{});
      return result != null && result.booleanValue();
    } catch(Exception e) {
      return false;
    }
  }


}
