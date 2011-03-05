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

  public static enum Type {
    deliveries, messages
  }

  protected int id;

  protected String name;

  private Type type;

  protected int progress;

  protected String creater;

  protected String error;
  protected String[] errorArgs;

  public static enum Status {
    IN_PROCESS, CANCELED, FINISHED, ERROR
  }

  protected Date from;

  protected Date till;

  protected Status status = Status.IN_PROCESS;

  protected Request(Type type) {
    this.type = type;
  }

  public String getCreater() {
    return creater;
  }

  void setCreater(String creater) {
    this.creater = creater;
  }

  public Date getFrom() {
    return from;
  }

  void setFrom(Date from) {
    this.from = from;
  }

  public Date getTill() {
    return till;
  }

  void setTill(Date till) {
    this.till = till;
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

  public String getError() {
    return error;
  }

  void setError(String error) {
    this.error = error;
  }

  public String[] getErrorArgs() {
    return errorArgs;
  }

  void setErrorArgs(String[] errorArgs) {
    this.errorArgs = errorArgs;
  }

  protected abstract void execute(RequestExecutor executor) throws AdminException;

  protected void save(XmlConfig c) {

    final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    c.setString("name", name);

    c.setInt("id", id);

    if(from != null ) {
      c.setString("startDate", sdf.format(from));
    }

    if(till != null) {
      c.setString("endDate", sdf.format(till));
    }

    if(status != null) {
      c.setString("status", status.toString());
    }

    c.setString("creater", creater);

    if(error != null) {
      c.setString("error", error);
    }
    if(errorArgs != null) {
      c.setStringArray("errorArgs", errorArgs, "|");
    }
  }

  protected void load(XmlConfig c) throws AdminException {

    final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    try{
      String date = c.getString("startDate", null);
      from = date == null ? null : sdf.parse(date);

      date = c.getString("endDate", null);
      till = date == null ? null : sdf.parse(date);

      creater = c.getString("creater");

      name = c.getString("name");

      id = c.getInt("id");

      status = DeliveriesRequest.Status.valueOf(c.getString("status"));

      error = c.getString("error", null);
      if(c.containsParam("errorArgs")) {
        errorArgs = c.getStringArray("errorArgs", "|");
      }

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
        ", from=" + from +
        ", till=" + till +
        ", status=" + status +
        ", error=" + error
        +
        '}';
  }
}
