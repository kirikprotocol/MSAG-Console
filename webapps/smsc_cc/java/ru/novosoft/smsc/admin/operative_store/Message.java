package ru.novosoft.smsc.admin.operative_store;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;
import java.util.Date;

/**
 * Структура, описывающая сообщение, хранящееся в оперативном сторадже
 * @author Artem Snopkov
 */
public interface Message extends Serializable {

  public long getId() throws AdminException;

  public Date getSubmitTime() throws AdminException;

  public Date getValidTime() throws AdminException;

  public Integer getAttempts() throws AdminException;

  public Integer getLastResult() throws AdminException;

  public Date getLastTryTime() throws AdminException;

  public Date getNextTryTime() throws AdminException;

  public Address getOriginatingAddress() throws AdminException;

  public Address getDestinationAddress() throws AdminException;

  public Address getDealiasedDestinationAddress() throws AdminException;

  public Integer getMessageReference() throws AdminException;

  public String getServiceType() throws AdminException;

  public Integer getDeliveryReport() throws AdminException;

  public Integer getBillingRecord() throws AdminException;

  public RoutingInfo getOriginatingDescriptor() throws AdminException;

  public RoutingInfo getDestinationDescriptor() throws AdminException;

  public String getRouteId() throws AdminException;

  public Integer getServiceId() throws AdminException;

  public Integer getPriority() throws AdminException;

  public String getSrcSmeId() throws AdminException;

  public String getDstSmeId() throws AdminException;

  public byte getArc() throws AdminException;

  public byte[] getBody() throws AdminException;

  public String getOriginalText() throws AdminException;

  public String getText() throws AdminException;

  public boolean isTextEncoded() throws AdminException;

  public BodyParameters getBodyParameters() throws AdminException;
}
