package mobi.eyeline.informer.web.auth;

import java.security.Principal;

/**
 * author: alkhal
 */
public class Role implements Principal {

  private final String name;


  public Role(String name) {
    this.name = name;
  }


  public String getName() {
    return name;
  }

}

