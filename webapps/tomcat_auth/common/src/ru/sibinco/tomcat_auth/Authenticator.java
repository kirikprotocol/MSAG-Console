/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 22:26:51
 */
package ru.sibinco.tomcat_auth;

import java.security.Principal;


public interface Authenticator
{
  public Principal authenticate(final String login, final String password);

  public boolean hasRole(final Principal principal, final String role);
}
