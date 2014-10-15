package ru.sibinco.scag.web;

import org.apache.log4j.Logger;
import ru.sibinco.scag.jaas.Authenticator;

public class WebContext {

  private static final Logger log = Logger.getLogger(WebContext.class);

  private static Authenticator auth;

  public static void init(Authenticator authenticator){
    auth = authenticator;
    if (log.isInfoEnabled()) log.info("Web context initialized.");
  }

  public static Authenticator getAuthenticator() {
    return auth;
  }

  public static void setAuthenticator(Authenticator authenticator){
    auth = authenticator;
  }

}
