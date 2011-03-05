package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.config.XmlConfig;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class MessagesRequest extends Request{

  private static final ValidationHelper vh = new ValidationHelper(MessagesRequest.class);

  private Address address;
  
  private String owner;

  MessagesRequest() {
    super(Type.messages);
  }

  MessagesRequest(MessagesRequest r) {
    this();
    owner = r.owner;
    from = r.from == null ? null : new Date(r.from.getTime());
    till = r.till == null ? null : new Date(r.till.getTime());
    address = r.address;
    name = r.name;
    creater = r.creater;
    id = r.id;
    progress = r.progress;
    status = r.status;
    error = r.error;
    if(r.errorArgs != null) {
      errorArgs = new String[r.errorArgs.length];
      System.arraycopy(r.errorArgs, 0, errorArgs, 0, errorArgs.length);
    }
  }

  public Address getAddress() {
    return address;
  }

  void setAddress(Address address) {
    this.address = address;
  }

  public String getOwner() {
    return owner;
  }

  void setOwner(String owner) {
    this.owner = owner;
  }

  public void copyFrom(MessagesRequestPrototype pr) {
    this.address = pr.getAddress() == null ? null : new Address(pr.getAddress());
    this.from = pr.getFrom() == null ? null : new Date(pr.getFrom().getTime());
    this.till = pr.getTill() == null ? null : new Date(pr.getTill().getTime());
    this.name = pr.getName();
    this.owner = pr.getOwner();
  }

  @Override
  protected void execute(RequestExecutor executor) throws AdminException {
    executor.execute(this);
  }

  @Override
  protected void save(XmlConfig c) {
    super.save(c);

    if(address != null) {
      c.setString("address", address.getSimpleAddress());
    }
    if(owner != null) {
      c.setString("owner", owner);
    }
  }

  @Override
  protected void load(XmlConfig c) throws AdminException{
    super.load(c);

    try{
      String address = c.getString("address");
      if(address != null) {
        this.address = new Address(address);
      }
      this.owner = c.getString("owner", null);

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
    if(from != null) {
      if(that.from == null || from.getTime()/1000 != that.from.getTime()/1000) {
        return false;
      }
    }else {
      if(that.from != null) {
        return false;
      }
    }
    if(till != null) {
      if(that.till == null || till.getTime()/1000 != that.till.getTime()/1000) {
        return false;
      }
    }else {
      if(that.till != null) {
        return false;
      }
    }

    if (progress != that.progress) return false;
    if (creater != null ? !creater.equals(that.creater) : that.creater != null) return false;
    if (id != that.id) return false;
    if (name != null ? !name.equals(that.name) : that.name != null) return false;
    if (owner != null ? !owner.equals(that.owner) : that.owner != null) return false;
    if (status != that.status) return false;

    return true;
  }

  @Override
  public String toString() {
    return "MessagesRequest{" +
        "from=" + from +
        ", till=" + till +
        ", owner=" + owner +
        ", address=" + address +", " + super.toString()+
        "} " ;
  }
}
