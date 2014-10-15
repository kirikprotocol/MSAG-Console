package ru.sibinco.scag.jaas;

import java.util.Set;

public class MSAGPrincipal {
  private String name;
  private final String password;
  private final Set<String> roles;

  public MSAGPrincipal(String name, String password, Set<String> roles) {
    this.name = name;
    this.password = password;
    this.roles = roles;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getPassword() {
    return password;
  }

  public Set<String> getRoles() {
    return roles;
  }

  @Override
  public String toString() {
    return "MSAGPrincipal" + "{name='" + name + '\'' + ", password='" + password + '\'' + ", roles=" + roles + '}';
  }
}