package ru.novosoft.smsc.admin.operative_store;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;
import java.util.Date;

/**
 * Структура, описывающая сообщение, хранящееся в оперативном сторадже
 *
 * @author Artem Snopkov
 */
public interface Message extends Serializable {

  public long getId();

  public Date getSubmitTime();

  public Date getValidTime();

  public Integer getAttempts();

  public Integer getLastResult();

  public Date getLastTryTime();

  public Date getNextTryTime();

  public Address getOriginatingAddress();

  public Address getDestinationAddress();

  public Address getDealiasedDestinationAddress();

  public Integer getMessageReference();

  public String getServiceType();

  public Integer getDeliveryReport();

  public Integer getBillingRecord();

  public RoutingInfo getOriginatingDescriptor();

  public RoutingInfo getDestinationDescriptor();

  public String getRouteId();

  public Integer getServiceId();

  public Integer getPriority();

  public String getSrcSmeId();

  public String getDstSmeId();

  public byte getArc();

  public byte[] getBody();

  public String getOriginalText() throws AdminException;

  public String getText() throws AdminException;

  public boolean isTextEncoded() throws AdminException;

  public BodyParameters getBodyParameters() throws AdminException;

  public Status getStatus();


  public static enum Status {
    ENROUTE, DELIVERED, EXPIRED, UNDELIVERABLE, DELETED, UNKNOWN;

    public static Status valueOf(int status) {
      Status[] _values = Status.values();
      if (status >= 0 && status < _values.length) {
        return _values[status];
      } else {
        return UNKNOWN;
      }
    }

    public int getCode() {
      Status[] statuses = values();
      for (int i = 0; i < statuses.length; i++) {
        if (statuses[i] == this) {
          return i;
        }
      }
      return -1;
    }
  }
}
