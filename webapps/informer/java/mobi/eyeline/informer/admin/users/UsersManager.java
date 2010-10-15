package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.util.config.BaseManager;
import mobi.eyeline.informer.admin.util.config.ConfigFileManager;
import mobi.eyeline.informer.admin.util.config.SettingsReader;
import mobi.eyeline.informer.admin.util.config.SettingsWriter;

import java.io.File;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Управление настройками пользователей
 * @author Aleksandr Khalitov
 */
public class UsersManager extends BaseManager<UsersSettings> {

  public UsersManager(Infosme infosme, File config, File backup, FileSystem fileSystem) throws InitException {
    super(infosme,  config, backup, fileSystem,new UsersConfig());
  }

  public void addUser(final User user) throws AdminException{
    updateSettings(new SettingsWriter<UsersSettings>() {
      public void changeSettings(UsersSettings settings) throws AdminException {
        settings.addUser(user);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.addUser(user.getLogin());
      }
    });
  }

  public void updateUser(final User user) throws AdminException{
    updateSettings(new SettingsWriter<UsersSettings>() {
      public void changeSettings(UsersSettings settings) throws AdminException {
        settings.updateUser(user);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.updateUser(user.getLogin());
      }
    });
  }

  public void removeUser(final String login) throws AdminException{
    updateSettings(new SettingsWriter<UsersSettings>() {
      public void changeSettings(UsersSettings settings) throws AdminException {
        settings.removeUser(login);
      }
      public void infosmeCommand(Infosme infosme) throws AdminException {
        infosme.removeUser(login);
      }
    });
  }

  public List<User> getUsers()  {
    return readSettings(new SettingsReader<UsersSettings, List<User>>(){
      public List<User> executeRead(UsersSettings settings)  {
        List<User> result = new LinkedList<User>();
        for(User u : settings.getUsers()) {
          result.add(new User(u));
        }
        return result;
      }
    });
  }

  public User getUser(final String login)  {
    return readSettings(new SettingsReader<UsersSettings, User>(){
      public User executeRead(UsersSettings settings)  {
        User u = settings.getUser(login);
        return u == null ? null : new User(u);        
      }
    });
  }

}
