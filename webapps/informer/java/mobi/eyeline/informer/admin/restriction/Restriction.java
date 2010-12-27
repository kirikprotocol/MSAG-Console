package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

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

  private static final ValidationHelper vh = new ValidationHelper(Restriction.class);

  public Restriction() {
  }


  public Restriction(Restriction other) {
    this.id = other.id;
    this.name = other.name;
    this.startDate = other.getStartDate();
    this.endDate = other.getEndDate();
    this.userIds = other.userIds == null ? null : new ArrayList<String>(other.getUserIds());
    this.allUsers = other.allUsers;
  }

  void validate() throws AdminException {
    vh.checkNotEmpty("name", name);
    vh.checkNotNull("startDate", startDate);
    vh.checkNotNull("endDate", endDate);
    vh.checkGreaterThan("endDate", endDate, startDate);
    if (!allUsers) {
      vh.checkSizeGreaterThan("users", userIds, 0);
    }
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

  public void setStartDate(Date startDate) {

    this.startDate = startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) {

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

  public void setName(String name) {
    this.name = name;
  }

  @Override
  public String toString() {
    return "Restriction{" +
        "id=" + id +
        ", name='" + name + '\'' +
        ", startDate=" + startDate +
        ", endDate=" + endDate +
        ", allUsers=" + allUsers +
        ", userIds=" + userIds +
        '}';
  }
}
