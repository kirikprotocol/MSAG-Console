/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 22:25:58
 */
package ru.sibinco.tomcat_auth;

import java.security.Principal;
import java.util.HashMap;
import java.util.Map;


public class AuthenticatorProxy
{
  private static AuthenticatorProxy instance = null;

  public static synchronized AuthenticatorProxy getInstance()
  {
    if (instance == null)
      instance = new AuthenticatorProxy();

    return instance;
  }


  private Map authenticators = new HashMap();

  public void registerAuthenticator(final String realmName, final Authenticator authenticator)
  {
    synchronized (authenticators) {
      System.out.println(this.getClass().getName()+ ".registerAuthenticator(" + realmName + ", " + authenticator + ")");
      authenticators.put(realmName, authenticator);
    }
  }

  public Principal authenticate(final String realmName, final String login, final String password)
  {
    final Authenticator auth;
    synchronized (authenticators) {
      auth = (Authenticator) authenticators.get(realmName);
      System.out.println(this.getClass().getName()+ ".authenticate(\"" + realmName + "\", \"" + login + "\", \"" + password + "\"): " + auth);
    }
    if (auth == null)
      return null;
    else
      return auth.authenticate(login, password);
  }

  public boolean hasRole(final String realmName, final Principal principal, final String role)
  {
    final Authenticator auth;
    synchronized (authenticators) {
      auth = (Authenticator) authenticators.get(realmName);
      System.out.println(this.getClass().getName()+ ".hasRole: " + auth);
    }
    if (auth == null)
      return false;
    else
      return auth.hasRole(principal, role);
  }
}
