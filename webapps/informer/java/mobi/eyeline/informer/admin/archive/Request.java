package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.config.XmlConfig;
import org.apache.log4j.Logger;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public abstract class Request {

  protected static final Logger logger = Logger.getLogger(Request.class);

  public static enum Type {deliveries, messages}

  protected int id;

  protected String name;

  private Type type;

  protected int progress;

  protected String creater;

  public static enum Status {NEW, IN_PROCCESS, CANCELED, FINISHED}

  protected Date startDate = new Date();

  protected Date endDate;

  protected Status status = Status.NEW;

  protected Request(Type type) {
    this.type = type;
  }

  public String getCreater() {
    return creater;
  }

  void setCreater(String creater) {
    this.creater = creater;
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

  public Status getStatus() {
    return status;
  }

  void setStatus(Status status) {
    this.status = status;
  }

  public int getProgress() {
    return progress;
  }

  void setProgress(int progress) {
    this.progress = progress;
  }

  public int getId() {
    return id;
  }

  void setId(int id) {
    this.id = id;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public Type getType() {
    return type;
  }

  protected abstract void execute(RequestExecutor executor) throws AdminException;

  protected void save(XmlConfig c) {

    final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    c.setString("name", name);

    c.setInt("id", id);

    if(startDate != null ) {
      c.setString("startDate", sdf.format(startDate));
    }

    if(endDate != null) {
      c.setString("endDate", sdf.format(endDate));
    }

    if(status != null) {
      c.setString("status", status.toString());
    }

    c.setString("creater", creater);
  }

  protected void load(XmlConfig c) throws AdminException {

    final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    try{
      String date = c.getString("startDate", null);
      startDate = date == null ? null : sdf.parse(date);

      date = c.getString("endDate", null);
      endDate = date == null ? null : sdf.parse(date);

      creater = c.getString("creater");

      name = c.getString("name");

      id = c.getInt("id");

      status = DeliveriesRequest.Status.valueOf(c.getString("status"));

    }catch (Exception e){
      logger.error(e,e);
      throw new ArchiveException("internal_error");
    }
  }

  protected abstract Request copy();

  @Override
  public String toString() {
    return "Request{" +
        "id=" + id +
        ", name='" + name + '\'' +
        ", type=" + type +
        ", progress=" + progress +
        ", creater='" + creater + '\'' +
        ", startDate=" + startDate +
        ", endDate=" + endDate +
        ", status=" + status +
        '}';
  }
}
