package ru.sibinco.scag.web.security;

import java.util.Set;

public interface RoleMapper {

  public Set<String> getUserRoles(String username);

  public Set<String> getRolesAllowedForURI(String uri);

}
