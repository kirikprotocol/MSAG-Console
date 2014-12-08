package ru.sibinco.scag.web.security;

import java.util.HashSet;
import java.util.Set;

public class UserLoginData {

  private String name;

  private String password;

  private Set<String> roles = new HashSet<String>();

  public UserLoginData() {}

  public UserLoginData(String name, Set<String> roles){
    this.name = name;
    this.roles = roles;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }


  public Set<String> getRoles() {
    return roles;
  }

  public void setRoles(Set<String> roles) {
    this.roles = roles;
  }

  public boolean isURIPermitted(String uri){
    return true;
  }

  public String toString(){
    return "UserLoginData{"
        +"name='"+name+"', "
        +"roles="+roles
        +"}";


  }

  public boolean isUserInRole(String role){
    return roles.contains(role);
  }
}
