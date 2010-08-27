/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.novosoft.smsc.web.auth;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.users.UsersManager;
import ru.novosoft.smsc.admin.users.UsersSettings;

public class AuthenticatorImpl implements Authenticator {

  private static final Logger logger = Logger.getLogger(AuthenticatorImpl.class);

  private UsersManager usersManager;

  public AuthenticatorImpl(UsersManager usersManager) {
    this.usersManager = usersManager;
  }

  public synchronized SmscPrincipal authenticate(String login, String password) {
    try{
      UsersSettings us = usersManager.getUsersSettings();
      User u = us.getUser(login);
      if(u == null) {
        logger.warn("AuthenticatorImpl.authenticate('" + login + "', '" + password + "') FAILED - User not found.");
        return null;
      }
      if(!password.equals(u.getPassword())) {
        logger.warn("AuthenticatorImpl.authenticate('" + login + "', '" + password + "') FAILED - Incorrect password.");
        return null;
      }
      return new SmscPrincipal(login, password, u.getRoles());

    }catch (Exception e){
      logger.error(e,e);
      e.printStackTrace();
      return null;
    }
  }

}