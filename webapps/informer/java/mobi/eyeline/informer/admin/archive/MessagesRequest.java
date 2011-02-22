package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.config.XmlConfig;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class MessagesRequest extends Request{

  private static final ValidationHelper vh = new ValidationHelper(MessagesRequest.class);

  private Date from;

  private Date till;

  private Address address;

  MessagesRequest() {
    super(Type.messages);
  }

  MessagesRequest(MessagesRequest r) {
    this();
    from = r.from == null ? null : new Date(r.from.getTime());
    till = r.till == null ? null : new Date(r.till.getTime());
    address = r.address;
    name = r.name;
    creater = r.creater;
    id = r.id;
    startDate = r.startDate == null ? null : new Date(r.startDate.getTime());
    endDate = r.endDate == null ? null : new Date(r.endDate.getTime());
    progress = r.progress;
    status = r.status;
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

  public Address getAddress() {
    return address;
  }

  void setAddress(Address address) {
    this.address = address;
  }

  public void copyFrom(MessagesRequestPrototype pr) {
    this.address = pr.getAddress() == null ? null : new Address(pr.getAddress());
    this.from = pr.getFrom() == null ? null : new Date(pr.getFrom().getTime());
    this.till = pr.getTill() == null ? null : new Date(pr.getTill().getTime());
    this.name = pr.getName();
  }

  @Override
  protected void execute(RequestExecutor executor) throws AdminException {
    executor.execute(this);
  }

  @Override
  protected void save(XmlConfig c) {
    super.save(c);

    final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    if(address != null) {
      c.setString("address", address.getSimpleAddress());
    }

    if(from != null) {
      c.setString("from", sdf.format(from));
    }

    if(till != null) {
      c.setString("till", sdf.format(till));
    }
  }

  @Override
  protected void load(XmlConfig c) throws AdminException{
    super.load(c);

    final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    try{
      String address = c.getString("address");
      if(address != null) {
        this.address = new Address(address);
      }
      String date = c.getString("from", null);
      from = date == null ? null : sdf.parse(date);

      date = c.getString("till", null);
      till = date == null ? null : sdf.parse(date);

    }catch (Exception e){
      logger.error(e,e);
      throw new ArchiveException("internal_error");
    }
  }

  @Override
  protected Request copy() {
    return new MessagesRequest(this);
  }

  public void validate() throws AdminException {
    vh.checkNotEmpty("name", name);
    vh.checkNotNull("from", from);
    vh.checkNotNull("address", address);
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (!(o instanceof MessagesRequest)) return false;

    MessagesRequest that = (MessagesRequest) o;

    if (address != null ? !address.equals(that.address) : that.address != null) return false;
    if(endDate != null) {
      if(that.endDate == null || endDate.getTime()/1000 != that.endDate.getTime()/1000) {
        return false;
      }
    }else {
      if(that.endDate != null) {
        return false;
      }
    }
    if(from != null) {
      if(that.from == null || from.getTime()/1000 != that.from.getTime()/1000) {
        return false;
      }
    }else {
      if(that.from != null) {
        return false;
      }
    }

    return true;
  }

  @Override
  public int hashCode() {
    int result = vh != null ? vh.hashCode() : 0;
    result = 31 * result + (from != null ? from.hashCode() : 0);
    result = 31 * result + (till != null ? till.hashCode() : 0);
    result = 31 * result + (address != null ? address.hashCode() : 0);
    return result;
  }

  @Override
  public String toString() {
    return "MessagesRequest{" +
        "from=" + from +
        ", till=" + till +
        ", address=" + address +", " + super.toString()+
        "} " ;
  }
}
