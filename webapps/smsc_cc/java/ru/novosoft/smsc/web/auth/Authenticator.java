package ru.novosoft.smsc.web.auth;

/**
 * author: alkhal
 */
public interface Authenticator {

  public SmscPrincipal authenticate(String login, String password);
  
}
