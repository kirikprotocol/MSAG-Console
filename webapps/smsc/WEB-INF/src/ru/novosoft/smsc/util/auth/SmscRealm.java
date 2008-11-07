package ru.novosoft.smsc.util.auth;

/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 17:32:26
 */

import org.apache.catalina.realm.RealmBase;
import ru.novosoft.smsc.admin.Constants;

import java.security.Principal;

public class SmscRealm extends RealmBase
{
  protected String getName()
  {
    return Constants.TomcatRealmName;
  }

  protected String getPassword(String s)
  {
    return null;
  }

  protected Principal getPrincipal(String s)
  {
    return null;
  }

  public Principal authenticate(String login, String password)
  {
      System.out.println("SmscRealm.authenticate");
    return XmlAuthenticator.getInstance().authenticate(login, password);
  }

  public boolean hasRole(Principal principal, String role)
  {
      System.out.println("SmscRealm.hasRole");
      return true;
    //return AuthenticatorProxy.getInstance().hasRole(getTitle(), principal, role);
  }
}
