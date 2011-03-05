package mobi.eyeline.informer.web.controllers.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.archive.MessagesRequest;
import mobi.eyeline.informer.admin.archive.MessagesRequestPrototype;
import mobi.eyeline.informer.admin.archive.Request;
import mobi.eyeline.informer.admin.delivery.DeliveryException;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import java.text.MessageFormat;
import java.util.Date;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class MessagesRequestController extends InformerController{

  private Integer reqId;

  private String name;

  private Date from;

  private Date till;

  private Address address;

  private boolean requestFinished;

  private String requestError;

  public MessagesRequestController() {
    String id = getRequestParameter("reqId");
    if(id != null && id.length()>0) {
      reqId = Integer.parseInt(id);
      tryLoad();
    }
  }

  private void tryLoad() {
    MessagesRequest request = null;
    try {
      request = (MessagesRequest)getConfig().getRequest(reqId);
    } catch (AdminException e) {
      addError(e);
    }
    if(request == null) {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "archive.request.not.found", reqId);
    }else {
      name = request.getName();
      from = request.getFrom();
      till = request.getTill();
      address = request.getAddress();
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

  public String save() {
    try {
      if(reqId != null) {
        MessagesRequest request = (MessagesRequest)getConfig().getRequest(reqId);
        if(request == null) {
          addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "archive.request.not.found", reqId);
          return null;
        }
        request.setName(name);
        getConfig().modifyRequest(request);
      }else {
        MessagesRequestPrototype prototype = new MessagesRequestPrototype();
        prototype.setFrom(from);
        prototype.setName(name);
        prototype.setTill(till);
        prototype.setAddress(address);
        if(!isUserInAdminRole()) {
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

  public Address getAddress() {
    return address;
  }

  public void setAddress(Address address) {
    this.address = address;
  }

  public boolean isRequestFinished() {
    return requestFinished;
  }
}
