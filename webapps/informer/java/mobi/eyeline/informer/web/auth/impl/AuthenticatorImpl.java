/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package mobi.eyeline.informer.web.auth.impl;

import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.auth.Authenticator;
import mobi.eyeline.informer.web.auth.InformerPrincipal;
import org.apache.log4j.Logger;

public class AuthenticatorImpl implements Authenticator {

  private static final Logger logger = Logger.getLogger(AuthenticatorImpl.class);

  private final Users users;

  public AuthenticatorImpl(Users users) {
    this.users = users;
  }

  public synchronized InformerPrincipal authenticate(String login, String password) {
    try{
      User u = users.getUser(login);
      if(u == null) {
        logger.warn("AuthenticatorImpl.authenticate('" + login + "', '" + password + "') FAILED - User not found.");
        return null;
      }
      if(!password.equals(u.getPassword())) {
        logger.warn("AuthenticatorImpl.authenticate('" + login + "', '" + password + "') FAILED - Incorrect password.");
        return null;
      }
      return new InformerPrincipal(login, password, u.getRoles());

    }catch (Exception e){
      logger.error(e,e);
      e.printStackTrace();
      return null;
    }
  }

}