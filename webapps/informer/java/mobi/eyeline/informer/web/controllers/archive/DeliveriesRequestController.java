package mobi.eyeline.informer.web.controllers.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.archive.DeliveriesRequest;
import mobi.eyeline.informer.admin.archive.DeliveriesRequestPrototype;
import mobi.eyeline.informer.admin.archive.Request;
import mobi.eyeline.informer.admin.delivery.DeliveryException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.text.MessageFormat;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveriesRequestController extends InformerController{

  private Integer reqId;

  private String name;

  private Date from;

  private Date till;

  private String owner;

  private String deliveryId;

  private String deliveryName;

  private boolean requestFinished;

  private String requestError;

  public DeliveriesRequestController() {
    String id = getRequestParameter("reqId");
    if(id != null && id.length()>0) {
      reqId = Integer.parseInt(id);
      tryLoad();
    }
  }

  private void tryLoad() {
    DeliveriesRequest request = null;
    try {
      request = (DeliveriesRequest)getConfig().getRequest(reqId);
    } catch (AdminException e) {
      addError(e);
    }
    if(request == null) {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "archive.request.not.found", reqId);
    }else {
      name = request.getName();
      from = request.getFrom();
      till = request.getTill();
      owner = request.getOwner();
      deliveryId = request.getDeliveryId() == null ? null : Integer.toString(request.getDeliveryId());
      deliveryName = request.getDeliveryName();
      requestFinished = request.getStatus() == Request.Status.FINISHED;

      if(request.getStatus() == Request.Status.ERROR) {
        ResourceBundle bundle = ResourceBundle.getBundle(DeliveryException.class.getName(), getLocale());
        if(request.getError() != null) {
          String pattern = bundle.getString(request.getError());
          requestError = request.getErrorArgs() == null ? pattern : MessageFormat.format(pattern, request.getErrorArgs());
        }else {
          requestError = bundle.getString("internal_error");
        }
      }
    }
  }


  public List<SelectItem> getUniqueUsers() {
    List<SelectItem> ss = new LinkedList<SelectItem>();
    ss.add(new SelectItem("",""));
    for(User u : getConfig().getUsers()) {
      ss.add(new SelectItem(u.getLogin(), u.getLogin()));
    }
    return ss;
  }

  public String save() {
    try {
      if(reqId != null) {
        DeliveriesRequest request = (DeliveriesRequest)getConfig().getRequest(reqId);
        if(request == null) {
          addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "archive.request.not.found", reqId);
          return null;
        }
        request.setName(name);
        getConfig().modifyRequest(request);
      }else {
        DeliveriesRequestPrototype prototype = new DeliveriesRequestPrototype();
        prototype.setFrom(from);
        prototype.setName(name);
        prototype.setTill(till);
        if(deliveryId != null && deliveryId.length()>0) {
          prototype.setDeliveryId(Integer.parseInt(deliveryId));
        }
        if(deliveryName != null && deliveryName.length()>0) {
          prototype.setDeliveryName(deliveryName);
        }
        if(isUserInAdminRole()) {
          if(owner != null && owner.length()>0) {
            prototype.setOwner(owner);
          }
        }else {
          prototype.setOwner(getUserName());
        }
        getConfig().createRequest(getUserName(), prototype);
      }
    } catch (AdminException e) {
      addError(e);
      return null;
    }
    return "ARCHIVE_LIST";
  }

  public String getRequestError() {
    return requestError;
  }

  public boolean isNew() {
    return reqId == null;
  }


  public String getDeliveryName() {
    return deliveryName;
  }

  public void setDeliveryName(String deliveryName) {
    this.deliveryName = deliveryName;
  }

  public Integer getReqId() {
    return reqId;
  }

  public void setReqId(Integer reqId) {
    this.reqId = reqId;
  }


  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public Date getFrom() {
    return from;
  }

  public void setFrom(Date from) {
    this.from = from;
  }

  public Date getTill() {
    return till;
  }

  public void setTill(Date till) {
    this.till = till;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) {
    this.owner = owner;
  }

  public String getDeliveryId() {
    return deliveryId;
  }

  public void setDeliveryId(String deliveryId) {
    this.deliveryId = deliveryId;
  }

  public boolean isRequestFinished() {
    return requestFinished;
  }
}
