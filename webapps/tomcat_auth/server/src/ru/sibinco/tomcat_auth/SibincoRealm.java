/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 17:32:26
 */
package ru.sibinco.tomcat_auth;

import org.apache.catalina.realm.RealmBase;

import java.security.Principal;


public class SibincoRealm extends RealmBase
{
  public static final String REALM_NAME = "Sibinco.TomcatAuthRealm";

  protected String getName()
  {
    return REALM_NAME;
  }

  protected String getPassword(final String s)
  {
    return null;
  }

  protected Principal getPrincipal(final String s)
  {
    return null;
  }

  public Principal authenticate(final String login, final String password)
  {
    final Principal principal = AuthenticatorProxy.getInstance().authenticate(getName(), login, password);
    System.out.println(REALM_NAME + ".authenticate(\"" + login + "\", \"" + password + "\"): " + (principal != null ? principal.toString() : "<null>"));
    return principal;
  }

  public boolean hasRole(final Principal principal, final String role)
  {
    final boolean result = AuthenticatorProxy.getInstance().hasRole(getName(), principal, role);
    System.out.println(REALM_NAME + ".hasRole(\"" + principal + "\", \"" + role + "\"): " + result);
    return result;
  }
}
