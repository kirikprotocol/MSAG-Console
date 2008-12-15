package ru.sibinco.smsx.engine.service.group.datasource.impl.principal;

import ru.sibinco.smsx.engine.service.group.datasource.Principal;

/**
 * User: artem
 * Date: 18.11.2008
 */
class PrincipalWrapper {
  public final Principal principal;
  public final long fileOffset;

  public PrincipalWrapper(Principal principal, long fileOffset) {
    this.principal = principal;
    this.fileOffset = fileOffset;
  }
}
