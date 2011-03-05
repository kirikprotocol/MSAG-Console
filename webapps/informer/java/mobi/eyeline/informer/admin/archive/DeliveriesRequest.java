package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;
import mobi.eyeline.informer.util.config.XmlConfig;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveriesRequest extends Request{

  private static final ValidationHelper vh = new ValidationHelper(DeliveriesRequest.class);

  private String deliveryName;

  private Integer deliveryId;

  private String owner;

  DeliveriesRequest() {
    super(Type.deliveries);
  }

  DeliveriesRequest(DeliveriesRequest r) {
    this();
    deliveryId = r.getDeliveryId();
    from = r.from == null ? null : new Date(r.from.getTime());
    till = r.till == null ? null : new Date(r.till.getTime());
    owner = r.owner;
    name = r.name;
    deliveryName = r.deliveryName;
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

  void setDeliveryName(String deliveryName) {
    this.deliveryName = deliveryName;
  }

  void setDeliveryId(Integer deliveryId) {
    this.deliveryId = deliveryId;
  }

  void setOwner(String owner) {
    this.owner = owner;
  }

  public String getDeliveryName() {
    return deliveryName;
  }

  public Integer getDeliveryId() {
    return deliveryId;
  }

  public String getOwner() {
    return owner;
  }

 void copyFrom(DeliveriesRequestPrototype pr) throws AdminException {
    deliveryId = pr.getDeliveryId();
    from = pr.getFrom() == null ? null : new Date(pr.getFrom().getTime());
    till = pr.getTill() == null ? null : new Date(pr.getTill().getTime());
    owner = pr.getOwner();
    name = pr.getName();
    deliveryName = pr.getDeliveryName();
  }

  void validate() throws AdminException {
    vh.checkNotEmpty("name", name);
    vh.checkNotNull("from", from);
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    DeliveriesRequest that = (DeliveriesRequest) o;

    if (progress != that.progress) return false;
    if (creater != null ? !creater.equals(that.creater) : that.creater != null) return false;
    if (deliveryId != null ? !deliveryId.equals(that.deliveryId) : that.deliveryId != null) return false;
    if (deliveryName != null ? !deliveryName.equals(that.deliveryName) : that.deliveryName != null) return false;
    if(till != null) {
      if(that.till == null || till.getTime()/1000 != that.till.getTime()/1000) {
        return false;
      }
    }else {
      if(that.till != null) {
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
    if (id != that.id) return false;
    if (name != null ? !name.equals(that.name) : that.name != null) return false;
    if (owner != null ? !owner.equals(that.owner) : that.owner != null) return false;
    if (status != that.status) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = creater != null ? creater.hashCode() : 0;
    result = 31 * result + (name != null ? name.hashCode() : 0);
    result = 31 * result + (from != null ? from.hashCode() : 0);
    result = 31 * result + (till != null ? till.hashCode() : 0);
    result = 31 * result + (deliveryName != null ? deliveryName.hashCode() : 0);
    result = 31 * result + (deliveryId != null ? deliveryId.hashCode() : 0);
    result = 31 * result + (owner != null ? owner.hashCode() : 0);
    result = 31 * result + id;
    result = 31 * result + progress;
    result = 31 * result + (status != null ? status.hashCode() : 0);
    return result;
  }

  @Override
  protected void execute(RequestExecutor executor) throws AdminException {
    executor.execute(this);
  }

  @Override
  protected void save(XmlConfig c) {

    super.save(c);

    if(deliveryId != null) {
      c.setInt("deliveryId", deliveryId);
    }

    if(deliveryName != null) {
      c.setString("deliveryName", deliveryName);
    }

    if(owner != null) {
      c.setString("owner", owner);
    }
  }

  @Override
  protected void load(XmlConfig c) throws AdminException {

    super.load(c);

    try{

      int dId = c.getInt("deliveryId", -1);
      deliveryId = dId == -1 ? null : dId;
      deliveryName = c.getString("deliveryName", null);
      owner = c.getString("owner", null);

    }catch (Exception e){
      logger.error(e,e);
      throw new ArchiveException("internal_error");
    }
  }

  @Override
  public String toString() {
    return "DeliveriesRequest{" +
        "from=" + from +
        ", till=" + till +
        ", deliveryName='" + deliveryName + '\'' +
        ", deliveryId=" + deliveryId +
        ", owner='" + owner + "\', "+ super.toString() +
        "} ";
  }

  @Override
  protected Request copy() {
    return new DeliveriesRequest(this);
  }
}
