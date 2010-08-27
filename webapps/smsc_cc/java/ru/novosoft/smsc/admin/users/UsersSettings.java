package ru.novosoft.smsc.admin.users;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;

/**
 * author: alkhal
 */
public class UsersSettings {

  private static final ValidationHelper vh = new ValidationHelper(UsersSettings.class);

  private Map<String, User> users = new LinkedHashMap<String, User>();

  UsersSettings() {
  }

  UsersSettings(UsersSettings c) {
    Map<String, User> r = new LinkedHashMap<String, User>(c.users.size());
    for (User u : c.users.values()) {
      r.put(u.getLogin(), new User(u));
    }
    this.users = r;
  }

  public Collection<User> getUsers() {
    return users.values();
  }

  public void setUsers(Collection<User> users) throws AdminException {
    vh.checkNoNulls("users", users);
    Map<String, User> r = new LinkedHashMap<String, User>(users.size());
    for (User r1 : users) {
      for (User r2 : users) {
        if (r1 != r2) {
          vh.checkNotEquals("login", r1.getLogin(), r2.getLogin());
        }
      }
      r.put(r1.getLogin(), r1);
    }
    this.users = r;
  }

  public User getUser(String login) {
    return users.get(login);   
  }

  public UsersSettings cloneSettings() {
    return new UsersSettings(this);
  }
}
