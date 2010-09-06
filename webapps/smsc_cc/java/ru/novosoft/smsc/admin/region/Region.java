package ru.novosoft.smsc.admin.region;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.TimeZone;

/**
 * Структуры, описывающая данные о регионе
 * @author Artem Snopkov
 */
public class Region implements Serializable {

  private static final transient ValidationHelper vh = new ValidationHelper(Region.class);

  private final int id;
  private final String name;
  private int bandwidth;
  private String email;
  private TimeZone timezone;
  private Collection<String> subjects = new ArrayList<String>();

  Region(int id, String name) throws AdminException {
    vh.checkNotEmpty("name", name);
    this.id = id;
    this.name = name;
  }

  Region(Region copy) {
    this.id = copy.id;
    this.name = copy.name;
    this.bandwidth = copy.bandwidth;
    this.email = copy.email;
    this.timezone = copy.timezone;
    this.subjects = new ArrayList<String>(copy.subjects);
  }

  public int getId() {
    return id;
  }

  public String getName() {
    return name;
  }  

  public int getBandwidth() {
    return bandwidth;
  }

  public void setBandwidth(int bandwidth) throws AdminException {
    vh.checkGreaterOrEqualsTo("bandwidth", bandwidth, 0);
    this.bandwidth = bandwidth;
  }

  public String getEmail() {
    return email;
  }

  public void setEmail(String email) {
    this.email = email;
  }

  public TimeZone getTimezone() {
    return timezone;
  }

  public void setTimezone(TimeZone timezone) {
    this.timezone = timezone;
  }

  public Collection<String> getSubjects() {
    return new ArrayList<String>(subjects);
  }

  public void setSubjects(Collection<String> subjects) throws AdminException {
    vh.checkNoNulls("subjects", subjects);
    this.subjects = new ArrayList<String>(subjects);
  }
}
