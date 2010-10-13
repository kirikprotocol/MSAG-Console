package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
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
    String lang = section.getString("locale");
    if(lang!=null) {
        u.setLocale(new Locale(lang));
    }
    u.setRoles(loadUserRoles(section));

    return u;
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
    if(user.getLocale()!=null) {
      userSection.setString("locale",user.getLocale().getLanguage());
    }
    userSection.addSection(createUserRolesSection(user));
    return userSection;
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
