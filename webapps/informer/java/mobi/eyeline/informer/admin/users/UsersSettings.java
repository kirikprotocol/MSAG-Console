package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Настройки пользователей
 *
 * @author Aleksandr Khalitov
 */
class UsersSettings {

  private static final ValidationHelper vh = new ValidationHelper(UsersSettings.class);

  private Map<String, User> users = new LinkedHashMap<String, User>();

  UsersSettings() {
  }

  private UsersSettings(UsersSettings c) {
    Map<String, User> r = new LinkedHashMap<String, User>(c.users.size());
    for (User u : c.users.values()) {
      r.put(u.getLogin(), new User(u));
    }
    this.users = r;
  }

  public Collection<User> getUsers() {
    return users.values();
  }

  public Map<String, User> getUsersMap() {
    return users;
  }

  void setUsers(Collection<User> users) throws AdminException {
    vh.checkNoNulls("users", users);
    Map<String, User> r = new LinkedHashMap<String, User>(users.size());
    for (User r1 : users) {
      r1.validate();
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

  public void updateUser(User user) throws AdminException {
    user.validate();
    User old = users.remove(user.getLogin());
    if (old == null) {
      throw new UserException("user_not_exist", user.getLogin());
    }
    users.put(user.getLogin(), user);
  }

  public void removeUser(String login) throws UserException {
    User old = users.remove(login);
    if (old == null) {
      throw new UserException("user_not_exist", login);
    }
  }

  public void addUser(User user) throws AdminException{
    user.validate();
    if (users.containsKey(user.getLogin()))
      throw new UserException("user_already_exist", user.getLogin());
    users.put(user.getLogin(), user);
  }
}
