package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 03.11.2010
 * Time: 14:16:53
 */
public class Restriction {
  private Integer id;
  private String name;
  private Date startDate;
  private Date endDate;
  private boolean allUsers;
  private List<String> userIds;


  public Restriction() {    
  }


  public Restriction(Restriction other) {
    this.id=other.id;
    this.name=other.name;
    this.startDate = other.getStartDate();
    this.endDate = other.getEndDate();
    this.userIds = other.userIds == null ? null : new ArrayList<String>(other.getUserIds());
    this.allUsers = other.allUsers;
  }

  public Integer getId() {
    return id;
  }

  public void setId(Integer id) {
    this.id = id;
  }

  public Date getStartDate() {
    return startDate;
  }

  public void setStartDate(Date startDate) throws AdminException {

    this.startDate = startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) throws AdminException {

    this.endDate = endDate;
  }

  public boolean isAllUsers() {
    return allUsers;
  }

  public void setAllUsers(boolean allUsers) {
    this.allUsers = allUsers;
  }

  public List<String> getUserIds() {
    return userIds;
  }

  public void setUserIds(List<String> userIds) {
    this.userIds = userIds;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) throws AdminException {
    this.name = name;
  }
}
