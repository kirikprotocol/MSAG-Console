package ru.sibinco.scag.web;

import org.apache.log4j.Logger;
import ru.sibinco.scag.web.security.Authenticator;
import ru.sibinco.scag.web.security.RoleMapper;

public class WebContext {

  private static final Logger log = Logger.getLogger(WebContext.class);

  private static Authenticator auth;

  private static RoleMapper rm;

  public static void init(Authenticator authenticator, RoleMapper roleMapper){
    auth = authenticator;
    rm = roleMapper;
    if (log.isInfoEnabled()) log.info("Web context initialized.");
  }

  public static Authenticator getAuthenticator() {
    return auth;
  }

  public static void setAuthenticator(Authenticator authenticator){
    auth = authenticator;
  }

  public static RoleMapper getRoleMapper(){
    return rm;
  }

  public static void setRoleMapper(RoleMapper roleMapper){
    rm = roleMapper;
  }

}
