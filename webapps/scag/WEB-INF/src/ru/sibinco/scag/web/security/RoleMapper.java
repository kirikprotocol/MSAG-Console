package ru.sibinco.scag.web.security;

import java.util.Map;
import java.util.Set;

public interface RoleMapper {

  public Set<String> getUserRoles(String username);

  public Set<String> getRolesAllowedForURI(String uri);

  public Map<String, Set<String>> getURIsForRoles();

}
