package mobi.eyeline.informer.admin.archive;


import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class ArchiveDelivery {

  private Integer id; //todo Почему Integer а не int ?

  private String name;

  private Date startDate;
  private Date endDate;

  private String owner;

  ArchiveDelivery() {
  }

  public Integer getId() {
    return id;
  }

  void setId(Integer id) {
    this.id = id;
  }

  public String getName() {
    return name;
  }

  void setName(String name) {
    this.name = name;
  }

  public Date getStartDate() {
    return startDate;
  }

  void setStartDate(Date startDate) {
    this.startDate = startDate;
  }

  public Date getEndDate() {
    return endDate;
  }

  void setEndDate(Date endDate) {
    this.endDate = endDate;
  }

  public String getOwner() {
    return owner;
  }

  void setOwner(String owner) {
    this.owner = owner;
  }
}
