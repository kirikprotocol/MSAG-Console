package ru.novosoft.smsc.admin.region;

import java.util.Set;
import java.util.HashSet;

/**
 * User: artem
 * Date: Aug 1, 2007
 */

public class Region {
  private String name;
  private int bandWidth;
  private int id = -1;
  private String email;
  private final Set subjects = new HashSet();

  public Region(String name) {
    this.name = name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getName() {
    return name;
  }

  public void clearSubjects() {
    subjects.clear();
  }

  public void addSubject(String subjectName) {
    subjects.add(subjectName);
  }

  public Set getSubjects() {
    return subjects;
  }

  public int getBandWidth() {
    return bandWidth;
  }

  public void setBandWidth(int bandWidth) {
    this.bandWidth = bandWidth;
  }

  public String getEmail() {
    return email;
  }

  public void setEmail(String email) {
    this.email = email;
  }

  public int getId() {
    return id;
  }

  void setId(int id) {
    this.id = id;
  }

  boolean isExists() {
    return id != -1;
  }
}
