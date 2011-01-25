package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigParam;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 */
class UsersConfig implements ManagedConfigFile<UsersSettings> {


  public UsersSettings load(InputStream is) throws Exception {
    UsersSettings us = new UsersSettings();
    XmlConfig config = new XmlConfig();
    config.load(is);
    XmlConfigSection users = config.getSection("USERS");
    Collection<XmlConfigSection> sections = users.sections();
    List<User> result = new ArrayList<User>(sections.size());

    for (XmlConfigSection section : sections) {
      User u = loadUser(section);
      result.add(u);
    }
    us.setUsers(result);
    return us;
  }

  private User loadUser(XmlConfigSection section) throws XmlConfigException, AdminException {
    User u = new User();
    u.setLogin(section.getName());
    u.setPassword(section.getString("password"));
    u.setStatus(User.Status.valueOf(section.getString("status")));
    u.setFirstName(section.getString("firstName"));
    u.setLastName(section.getString("lastName"));
    u.setPhone(new Address(section.getString("phone")));
    u.setEmail(section.getString("email"));
    u.setOrganization(section.getString("organization"));
    u.setCreateCDR(section.getBool("createCDR", false));
    u.setCdrDestination(section.getString("cdrDestination", null));
    u.setCdrOriginator(section.getString("cdrOriginator", null));
    String lang = section.getString("locale", "en");
    if (lang != null) {
      u.setLocale(new Locale(lang));
    }
    String delivStart = section.getString("deliveryStartTime", null);
    if (delivStart != null) {
      u.setDeliveryStartTime(new Time(delivStart));
    }
    String delivEnd = section.getString("deliveryEndTime", null);
    if (delivEnd != null) {
      u.setDeliveryEndTime(new Time(delivEnd));
    }
    u.setSmsPerSec(section.getInt("smsPerSec", u.getSmsPerSec()));
    u.setSourceAddr(new Address(section.getString("sourceAddr")));
    u.setValidityPeriod(new Time(section.getString("validityPeriod", "01:00:00")));
    u.setRoles(loadUserRoles(section));
    if (u.getRoles().isEmpty())
      u.getRoles().add(User.INFORMER_USER_ROLE);

    u.setDeliveryDays(loadDeliveryDays(section));
    u.setDeliveryType(User.DeliveryType.valueOf(section.getString("deliveryType")));
    u.setTransactionMode(section.getBool("transactionMode", false));
    u.setRetryOnFail(section.getBool("retryOnFail", false));
    u.setPolicyId(section.getString("policyId", null));
    u.setAllRegionsAllowed(section.getBool("allRegionsAllowed", false));
    u.setRegions(loadUserRegions(section));
    u.setPriority(section.getInt("priority"));
    u.setEmailNotification(section.getBool("emailNotification", false));
    u.setSmsNotification(section.getBool("smsNotification", false));
    u.setCreateArchive(section.getBool("createArchive", false));
    u.setDeliveryLifetime(section.getInt("deliveryLifetime"));

    u.setCpSettings(loadUserCpSettings(section));
    return u;
  }

  private List<UserCPsettings> loadUserCpSettings(XmlConfigSection section) throws XmlConfigException, AdminException {
    if (section.containsSection("CPSETTINGS")) {
      List<UserCPsettings> ucpsList = new ArrayList<UserCPsettings>();
      XmlConfigSection cpSectionsRoot = section.getSection("CPSETTINGS");
      for(XmlConfigSection s : cpSectionsRoot.sections()) {
        UserCPsettings ucps = new UserCPsettings();
        UserCPsettings.Protocol protocol = UserCPsettings.Protocol.valueOf(s.getString("protocol","sftp"));
        ucps.setProtocol(protocol);
        if(protocol!=UserCPsettings.Protocol.file) {
          if(ucps.getProtocol() != UserCPsettings.Protocol.localFtp) {        //todo refactoring (maybe strategy)
            ucps.setHost(s.getString("host"));
            if(s.containsParam("port")) {ucps.setPort(s.getInt("port"));}
          }else {
            ucps.setDirectoryMaxSize(s.getInt("directoryMaxSize"));
          }
          ucps.setLogin(s.getString("login"));
          ucps.setPassword(s.getString("password"));
        }
        String wT = s.getString("workType", null);
        ucps.setWorkType(wT == null || wT.length() == 0 ? UserCPsettings.WorkType.detailed : UserCPsettings.WorkType.valueOf(wT));

        String period = s.getString("activePeriodStart", null);
        ucps.setActivePeriodStart(new Time(period == null || period.length() == 0 ? "10:00:00" : period));

        period = s.getString("activePeriodEnd", null);
        ucps.setActivePeriodEnd(new Time(period == null || period.length() == 0 ? "20:00:00" : period));

        period = s.getString("periodInMin", null);
        ucps.setPeriodInMin(period == null || period.length() == 0 ? 5 : Long.parseLong(period));

        if(ucps.getProtocol() != UserCPsettings.Protocol.localFtp) {       //todo refactoring (maybe strategy)
          ucps.setDirectory(s.getString("directory"));
        }
        ucps.setEncoding(s.getString("encoding","UTF-8"));
        ucps.setSourceAddress(new Address(s.getString("sourceAddress")));
        ucps.setCreateReports(s.getBool("createReports", false));
        ucps.setName(s.getString("name"));
        ucpsList.add(ucps);
      }
      return ucpsList.isEmpty() ? null : ucpsList;
    }
    return null;
  }


  private List<Integer> loadUserRegions(XmlConfigSection section) throws XmlConfigException {
    if (section.containsSection("REGIONS")) {
      List<Integer> result = new ArrayList<Integer>();
      XmlConfigSection regions = section.getSection("REGIONS");
      Collection<XmlConfigParam> params = regions.params();
      for (XmlConfigParam p : params) {
        if (p.getBool()) {
          result.add(Integer.parseInt(p.getName()));
        }
      }
      return result.isEmpty() ? null : result;
    }
    return null;
  }


  private List<Integer> loadDeliveryDays(XmlConfigSection section) throws XmlConfigException {
    List<Integer> result = new ArrayList<Integer>();
    if (section.containsSection("DELIVERY_DAYS")) {
      XmlConfigSection daysSection = section.getSection("DELIVERY_DAYS");
      Collection<XmlConfigParam> params = daysSection.params();

      for (XmlConfigParam p : params) {
        if (p.getBool()) {
          result.add(Integer.valueOf(p.getName()));
        }
      }

    }
    return result;
  }

  private Set<String> loadUserRoles(XmlConfigSection section) throws XmlConfigException {
    XmlConfigSection roles = section.getSection("ROLES");
    Collection<XmlConfigParam> params = roles.params();
    TreeSet<String> result = new TreeSet<String>();
    for (XmlConfigParam p : params) {
      if (p.getBool()) {
        result.add(p.getName());
      }
    }
    return result;
  }

  public void save(InputStream oldFile, OutputStream newFile, UsersSettings settings) throws XmlConfigException, AdminException {

    XmlConfig config = new XmlConfig();
    config.load(oldFile);


    XmlConfigSection users = new XmlConfigSection("USERS");

    for (User user : settings.getUsers()) {
      XmlConfigSection section = createUserSection(user);
      users.addSection(section);
    }

    config.addSection(users);
    config.save(newFile);
  }

  private XmlConfigSection createUserSection(User user) throws AdminException {
    XmlConfigSection userSection = new XmlConfigSection(user.getLogin());
    userSection.setString("password",user.getPassword());
    userSection.setString("status",user.getStatus().toString());
    userSection.setString("firstName",user.getFirstName());
    userSection.setString("lastName",user.getLastName());
    userSection.setString("phone",user.getPhone() == null ? null : user.getPhone().getSimpleAddress());
    userSection.setString("email",user.getEmail());
    userSection.setString("organization",user.getOrganization());
    if(user.getCdrDestination() != null) {
      userSection.setString("cdrDestination",user.getCdrDestination());
    }
    if(user.getCdrOriginator() != null) {
      userSection.setString("cdrOriginator",user.getCdrOriginator());
    }
    if(user.isCreateCDR()) {
      userSection.setBool("createCDR",true);
    }
    if(user.getLocale()!=null) {
      userSection.setString("locale",user.getLocale().getLanguage());
    }
    userSection.setInt("smsPerSec",user.getSmsPerSec());
    userSection.setString("sourceAddr",user.getSourceAddr().getSimpleAddress());
    if(user.getValidityPeriod() != null) {
      userSection.setString("validityPeriod",user.getValidityPeriod().getTimeString());
    }
    userSection.setString("deliveryType",user.getDeliveryType().toString());
    if(user.isTransactionMode())  {
      userSection.setBool("transactionMode",true);
    }
    userSection.setBool("retryOnFail", user.isRetryOnFail());
    if (user.getPolicyId() != null) {
      userSection.setString("policyId", user.getPolicyId());
    }
    if (user.getDeliveryStartTime() != null) {
      userSection.setString("deliveryStartTime", user.getDeliveryStartTime().getTimeString());
    }
    if (user.getDeliveryEndTime() != null) {
      userSection.setString("deliveryEndTime", user.getDeliveryEndTime().getTimeString());
    }

    userSection.addSection(createUserRolesSection(user));

    if (!user.getDeliveryDays().isEmpty()) {
      userSection.addSection(createDeliveryDaysSection(user));
    }
    userSection.setBool("allRegionsAllowed", user.isAllRegionsAllowed());
    if (user.getRegions() != null && !user.getRegions().isEmpty()) {
      userSection.addSection(createRegionsSection(user));
    }

    userSection.setInt("priority", user.getPriority());
    if (user.isEmailNotification()) {
      userSection.setBool("emailNotification", true);
    }
    if (user.isSmsNotification()) {
      userSection.setBool("smsNotification", true);
    }
    if (user.isCreateArchive()) {
      userSection.setBool("createArchive", true);
    }

    userSection.setInt("deliveryLifetime", user.getDeliveryLifetime());


    if (user.getCpSettings() != null && !user.getCpSettings().isEmpty()) {
      userSection.addSection(createCpSettingsSection(user));
    }

    return userSection;
  }

  private XmlConfigSection createRegionsSection(User user) {
    XmlConfigSection regionsSection = new XmlConfigSection("REGIONS");
    List<Integer> regions = user.getRegions();
    for (Integer region : regions) {
      regionsSection.setBool(region.toString(), true);
    }
    return regionsSection;
  }

  private XmlConfigSection createCpSettingsSection(User user) throws AdminException {
    XmlConfigSection section = new XmlConfigSection("CPSETTINGS");
    List<UserCPsettings> cpSettings = user.getCpSettings();
    for (int i = 0, cpSettingsSize = cpSettings.size(); i < cpSettingsSize; i++) {
      UserCPsettings ucps = cpSettings.get(i);
      XmlConfigSection s = new XmlConfigSection(user.getLogin()+"_"+ucps.getHashId());
      section.addSection(s);
      s.setString("protocol",ucps.getProtocol().toString());
      if(ucps.getProtocol() != UserCPsettings.Protocol.file) {
        if(ucps.getProtocol() != UserCPsettings.Protocol.localFtp) {       //todo refactoring (maybe strategy)
          s.setString("host",ucps.getHost());
          if(ucps.getPort()!=null && ucps.getPort()!=0){
            s.setInt("port",ucps.getPort());
          }
        }else {
          s.setInt("directoryMaxSize", ucps.getDirectoryMaxSize());
        }
        s.setString("login",ucps.getLogin());
        s.setString("password",ucps.getPassword());
      }
      if(ucps.isCreateReports()) {
        s.setBool("createReports",true);
      }
      s.setString("workType", ucps.getWorkType() == null ? "" : ucps.getWorkType().toString());
      s.setString("periodInMin", ucps.getPeriodInMin() == 0 ? "" : Long.toString(ucps.getPeriodInMin()));
      s.setString("activePeriodStart", ucps.getActivePeriodStart() == null ? "" : ucps.getActivePeriodStart().toString());
      s.setString("activePeriodEnd", ucps.getActivePeriodEnd() == null ? "" : ucps.getActivePeriodEnd().toString());
      if(ucps.getProtocol() != UserCPsettings.Protocol.localFtp) {               //todo refactoring (maybe strategy)
        s.setString("directory",ucps.getDirectory());
      }
      s.setString("encoding",ucps.getEncoding());
      s.setString("sourceAddress",ucps.getSourceAddress().getSimpleAddress());
      s.setString("name", ucps.getName());
    }
    return section;
  }

  private XmlConfigSection createDeliveryDaysSection(User user) {
    XmlConfigSection deliveryDaysSection = new XmlConfigSection("DELIVERY_DAYS");
    List<Integer> days = user.getDeliveryDays();
    for (Integer day : days) {
      deliveryDaysSection.setBool(day.toString(), true);
    }
    return deliveryDaysSection;
  }

  private XmlConfigSection createUserRolesSection(User user) {
    XmlConfigSection rolesSection = new XmlConfigSection("ROLES");
    Set<String> roles = user.getRoles();
    for (String role : roles) {
      rolesSection.setBool(role, true);
    }
    return rolesSection;
  }

}