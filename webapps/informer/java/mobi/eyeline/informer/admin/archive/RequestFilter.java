package mobi.eyeline.informer.admin.archive;

/**
 * author: Aleksandr Khalitov
 */
public class RequestFilter {

  private Integer id;

  private Request.Type type;

  private String namePrefix;

  private String creator;

  public Integer getId() {
    return id;
  }

  public void setId(Integer id) {
    this.id = id;
  }

  public Request.Type getType() {
    return type;
  }

  public void setType(Request.Type type) {
    this.type = type;
  }

  public String getNamePrefix() {
    return namePrefix;
  }

  public void setNamePrefix(String namePrefix) {
    this.namePrefix = namePrefix;
  }

  public String getCreator() {
    return creator;
  }

  public void setCreator(String creator) {
    this.creator = creator;
  }
}
