package ru.novosoft.smsc.util.auth;

/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 22:26:51
 */

import java.security.Principal;

public interface Authenticator
{
  public Principal authenticate(String login, String password);

  public boolean hasRole(Principal principal, String role);
}
