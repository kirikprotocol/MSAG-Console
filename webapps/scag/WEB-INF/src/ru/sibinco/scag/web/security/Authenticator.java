package ru.sibinco.scag.web.security;

public interface Authenticator {

  public boolean authenticate(String login, String password);

}
