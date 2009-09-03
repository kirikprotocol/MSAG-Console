package ru.novosoft.smsc.admin.region;

import ru.novosoft.smsc.util.Functions;

import java.util.Set;
import java.util.HashSet;
import java.util.TimeZone;
import java.util.Date;

/**
 * User: artem
 * Date: Aug 1, 2007
 */

public class Region {
  private String name;
  private int bandWidth;
  private int id = -1;
  private String email;
  private TimeZone timezone;
  private final Set subjects = new HashSet();
  private String infoSmeSmsc;

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

  public TimeZone getTimezone() {
    return timezone;
  }

  public void setTimezone(TimeZone timezone) {
    this.timezone = timezone;
  }

  /**
   * Converts local (server) time to region specific timezone
   * @param time local (server) time
   * @return region specific time
   */
  public Date getRegionTime(Date time) {
    return time == null ? null : Functions.convertTime(time, TimeZone.getDefault(), timezone);
  }


  /**
   * Converts region specific time to local (server) time
   * @param time region specific time
   * @return local (server) time
   */
  public Date getLocalTime(Date time) {
    return time == null ? null : Functions.convertTime(time, timezone, TimeZone.getDefault());
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

  public String getInfoSmeSmsc() {
    return infoSmeSmsc;
  }

  public void setInfoSmeSmsc(String infoSmeSmsc) {
    this.infoSmeSmsc = infoSmeSmsc;
  }
}
