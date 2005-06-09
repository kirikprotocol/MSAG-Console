/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.auth;

/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 17:32:26
 */

import org.apache.catalina.realm.RealmBase;
import ru.sibinco.scag.Constants;

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
    return AuthenticatorProxy.getInstance().authenticate(getName(), login, password);
  }

  public boolean hasRole(Principal principal, String role)
  {
    return AuthenticatorProxy.getInstance().hasRole(getName(), principal, role);
  }
}