/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package mobi.eyeline.informer.web.auth.impl;

import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.auth.Authenticator;
import mobi.eyeline.informer.web.auth.InformerPrincipal;
import org.apache.log4j.Logger;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class AuthenticatorImpl implements Authenticator {

  private static final Logger logger = Logger.getLogger(AuthenticatorImpl.class);

  private final Users users;

  private final Lock mapLock = new ReentrantLock();

  private Map<String, Error> errors = new LinkedHashMap<String, Error>(100) {
    private static final int MAX_ENTRIES = 100;
    @Override
    protected boolean removeEldestEntry(Map.Entry<String, Error> eldest) {
      return size() > MAX_ENTRIES;
    }
  };

  public AuthenticatorImpl(Users users) {
    this.users = users;
  }

  public synchronized InformerPrincipal authenticate(String login, String password) {
    try {
      User u = users.getUser(login);
      try{
        mapLock.lock();
        if (u == null) {
          errors.put(login, Error.NOT_FOUND);
          logger.warn("AuthenticatorImpl.authenticate('" + login + "', '" + password + "') FAILED - User not found.");
          return null;
        }
        if (User.Status.ENABLED != u.getStatus()) {
          errors.put(login, Error.USER_BLOCKED);
          logger.warn("AuthenticatorImpl.authenticate('" + login + "', '" + password + "') FAILED - User not ENABLED.");
          return null;
        }
        if (!password.equals(u.getPassword())) {
          errors.put(login, Error.WRONG_PASSWORD);
          logger.warn("AuthenticatorImpl.authenticate('" + login + "', '" + password + "') FAILED - Incorrect password.");
          return null;
        }
        errors.remove(login);
      }finally {
        mapLock.unlock();
      }
      return new InformerPrincipal(login, password, u.getRoles());

    } catch (Exception e) {
      logger.error(e, e);
      e.printStackTrace();
      return null;
    }
  }

  @Override
  public Error getError(String login) {
    try{
      mapLock.lock();
      return errors.get(login);
    }finally {
      mapLock.unlock();
    }
  }

}