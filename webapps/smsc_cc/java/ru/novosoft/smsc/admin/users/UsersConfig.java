package ru.novosoft.smsc.admin.users;

import ru.novosoft.smsc.admin.config.ManagedConfigFile;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collection;

/**
 * author: alkhal
 */
public class UsersConfig implements ManagedConfigFile<UsersSettings>{

  public void save(InputStream is, OutputStream os, UsersSettings settings) throws Exception {
    UserParser.saveUsers(os, settings.getUsers());
  }

  public UsersSettings load(InputStream is) throws Exception {
    UsersSettings us = new UsersSettings();
    Collection<User> users = UserParser.loadUsers(is);
    us.setUsers(users);
    return us;
  }

}
