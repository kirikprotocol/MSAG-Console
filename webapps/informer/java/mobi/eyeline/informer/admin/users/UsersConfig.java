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
class UsersConfig implements ManagedConfigFile<UsersSettings>{



  public UsersSettings load(InputStream is) throws Exception {
    UsersSettings us = new UsersSettings();
    XmlConfig config = new XmlConfig();
    config.load(is);
    XmlConfigSection users = config.getSection("USERS");
    Collection<XmlConfigSection> sections =  users.sections();
    List<User> result = new ArrayList<User>(sections.size());

    for(XmlConfigSection section : sections) {
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
    u.setPhone(section.getString("phone"));
    u.setEmail(section.getString("email"));
    u.setOrganization(section.getString("organization"));
    u.setCreateCDR(section.getBool("createCDR",false));
    String lang = section.getString("locale");
    if(lang!=null) {
        u.setLocale(new Locale(lang));
    }
    String delivStart=section.getString("deliveryStartTime",null);
    if(delivStart!=null) {
        u.setDeliveryStartTime(new Time(delivStart));
    }
    String delivEnd=section.getString("deliveryEndTime",null);
    if(delivEnd!=null) {
        u.setDeliveryEndTime(new Time(delivEnd));
    }
    u.setSmsPerSec(section.getInt("smsPerSec",u.getSmsPerSec()));
    u.setSourceAddr(new Address(section.getString("sourceAddr")));
    u.setValidHours(section.getInt("validHours"));
    u.setRoles(loadUserRoles(section));
    u.setDeliveryDays(loadDeliveryDays(section));
    u.setDeliveryType(User.DeliveryType.valueOf(section.getString("deliveryType")));
    u.setTransactionMode(section.getBool("transactionMode",false));
    u.setPolicyId(section.getString("policyId"));
    u.setRegions(loadUserRegions(section));
    u.setPriority(section.getInt("priority"));
    u.setEmailNotification(section.getBool("emailNotification",false));
    u.setSmsNotification(section.getBool("smsNotification",false));
    u.setCreateArchive(section.getBool("createArchive",false));
    u.setDeliveryLifetime(section.getInt("deliveryLifetime"));

    u.setDirectory(section.getString("directory"));
    u.setDirectoryPoolPeriod(section.getInt("directoryPoolPeriod"));
    u.setCreateReports(section.getBool("createReports",false));
    u.setReportsLifetime(section.getInt("reportsLifetime"));

    return u;
  }

  private Set<String> loadUserRegions(XmlConfigSection section) throws XmlConfigException {
    TreeSet<String> result = new TreeSet<String>();
    if(section.containsSection("REGIONS")) {
      XmlConfigSection regions = section.getSection("REGIONS");
      Collection<XmlConfigParam> params = regions.params();
      for(XmlConfigParam p : params) {
        if(p.getBool()) {
          result.add(p.getName());
        }
      }
    }
    return result;
  }


  private Set<Integer> loadDeliveryDays(XmlConfigSection section) throws XmlConfigException {
    if(section.containsSection("DELIVERY_DAYS")) {
      XmlConfigSection daysSection = section.getSection("DELIVERY_DAYS");      
      Collection<XmlConfigParam> params = daysSection.params();
      TreeSet<Integer> result = new TreeSet<Integer>();
      for(XmlConfigParam p : params) {
        if(p.getBool()) {
          result.add(Integer.valueOf(p.getName()));
        }
      }
      return result;
    }
    return null;
  }

  private Set<String> loadUserRoles(XmlConfigSection section) throws XmlConfigException {
    XmlConfigSection roles = section.getSection("ROLES");
    Collection<XmlConfigParam> params = roles.params();
    TreeSet<String> result = new TreeSet<String>();
    for(XmlConfigParam p : params) {
      if(p.getBool()) {
        result.add(p.getName());
      }
    }
    return result;
  }

  public void save(InputStream oldFile, OutputStream newFile, UsersSettings settings) throws XmlConfigException {

    XmlConfig config = new XmlConfig();
    config.load(oldFile);


    XmlConfigSection  users = new XmlConfigSection("USERS");

    for(User user : settings.getUsers() ) {
      XmlConfigSection section = createUserSection(user);
      users.addSection(section);
    }

    config.addSection(users);
    config.save(newFile);
  }

  private XmlConfigSection createUserSection(User user) {
    XmlConfigSection userSection = new XmlConfigSection(user.getLogin());    
    userSection.setString("password",user.getPassword());
    userSection.setString("status",user.getStatus().toString());
    userSection.setString("firstName",user.getFirstName());
    userSection.setString("lastName",user.getLastName());
    userSection.setString("phone",user.getPhone());
    userSection.setString("email",user.getEmail());
    userSection.setString("organization",user.getOrganization());
    if(user.isCreateCDR()) {
      userSection.setBool("createCDR",true);
    }
    if(user.getLocale()!=null) {
      userSection.setString("locale",user.getLocale().getLanguage());
    }
    userSection.setInt("smsPerSec",user.getSmsPerSec());
    userSection.setString("sourceAddr",user.getSourceAddr().getSimpleAddress());
    userSection.setInt("validHours",user.getValidHours());
    userSection.setString("deliveryType",user.getDeliveryType().toString());
    if(user.isTransactionMode())  {
      userSection.setBool("transactionMode",true);
    }
    userSection.setString("policyId",user.getPolicyId());
    if(user.getDeliveryStartTime()!=null) {
      userSection.setString("deliveryStartTime",user.getDeliveryStartTime().getTimeString());
    }
    if(user.getDeliveryEndTime()!=null) {
      userSection.setString("deliveryEndTime",user.getDeliveryEndTime().getTimeString());
    }

    userSection.addSection(createUserRolesSection(user));

    if(user.getDeliveryDays()!=null && !user.getDeliveryDays().isEmpty()) {
      userSection.addSection(createDeliveryDaysSection(user));
    }
    if(user.getRegions()!=null && !user.getRegions().isEmpty()) {
      userSection.addSection(createRegionsSection(user));
    }

    userSection.setInt("priority",user.getPriority());
    if(user.isEmailNotification()) {
      userSection.setBool("emailNotification",true);
    }
    if(user.isSmsNotification()) {
      userSection.setBool("smsNotification",true);
    }
    if(user.isCreateArchive()) {
      userSection.setBool("createArchive",true);
    }

    userSection.setInt("deliveryLifetime",user.getDeliveryLifetime());
    userSection.setString("directory",user.getDirectory());
    userSection.setInt("directoryPoolPeriod",user.getDirectoryPoolPeriod());
    if(user.isCreateReports()) {
      userSection.setBool("createReports",true);
    }
    userSection.setInt("reportsLifetime",user.getReportsLifetime());

    return userSection;
  }

  private XmlConfigSection createRegionsSection(User user) {
    XmlConfigSection deliveryDaysSection = new XmlConfigSection("REGIONS");
    Set<String> regions = user.getRegions();
    for(String region : regions) {
      deliveryDaysSection.setBool(region,true);
    }
    return deliveryDaysSection;
  }

  private XmlConfigSection createDeliveryDaysSection(User user) {
    XmlConfigSection deliveryDaysSection = new XmlConfigSection("DELIVERY_DAYS");
    Set<Integer> days = user.getDeliveryDays();
    for(Integer day : days) {
      deliveryDaysSection.setBool(day.toString(),true);
    }
    return deliveryDaysSection;
  }

  private XmlConfigSection createUserRolesSection(User user) {
    XmlConfigSection rolesSection = new XmlConfigSection("ROLES");
    Set<String> roles = user.getRoles();
    for(String role : roles) {
      rolesSection.setBool(role,true);
    }
    return rolesSection;
  }

}
