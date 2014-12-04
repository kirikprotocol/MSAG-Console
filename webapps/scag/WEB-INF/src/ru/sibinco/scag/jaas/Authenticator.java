package ru.sibinco.scag.jaas;

import ru.sibinco.scag.web.security.UserLoginData;

public interface Authenticator {

  public boolean authenticate(String login, String password);

}
