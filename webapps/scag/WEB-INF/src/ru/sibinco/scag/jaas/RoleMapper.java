package ru.sibinco.scag.jaas;

import java.util.Set;

public interface RoleMapper {

  public Set<String> getRoles(String userName);

}
