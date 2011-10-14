package ru.novosoft.smsc.web.controllers.sms_view;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.operative_store.DataCoding;
import ru.novosoft.smsc.util.Address;

import java.util.*;

/**
 * @author Artem Snopkov
 */
public abstract class Sms {

  private final ResourceBundle bundle;

  protected Sms(ResourceBundle bundle) {
    this.bundle = bundle;
  }

  public abstract long getId() throws AdminException;

  public abstract Date getSubmitTime() throws AdminException;

  public abstract Date getValidTime() throws AdminException;

  public abstract Integer getAttempts() throws AdminException;

  public abstract Integer getLastResult() throws AdminException;

  public abstract Date getLastTryTime() throws AdminException;

  public abstract Date getNextTryTime() throws AdminException;

  public abstract Address getOriginatingAddress() throws AdminException;

  public abstract Address getDestinationAddress() throws AdminException;

  public abstract Address getDealiasedDestinationAddress() throws AdminException;

  public abstract Integer getMessageReference() throws AdminException;

  public abstract String getServiceType() throws AdminException;

  public abstract Integer getDeliveryReport() throws AdminException;

  public abstract Integer getBillingRecord() throws AdminException;

  public abstract RoutingInfo getOriginatingDescriptor() throws AdminException;

  public abstract RoutingInfo getDestinationDescriptor() throws AdminException;

  public abstract String getRouteId() throws AdminException;

  public abstract Integer getServiceId() throws AdminException;

  public abstract Integer getPriority() throws AdminException;

  public abstract String getSrcSmeId() throws AdminException;

  public abstract String getDstSmeId() throws AdminException;

  public abstract byte getArc() throws AdminException;

  public abstract byte[] getBody() throws AdminException;

  public abstract String getOriginalText() throws AdminException;

  public abstract String getText() throws AdminException;

  public abstract boolean isTextEncoded() throws AdminException;

  public abstract boolean isShowText() throws AdminException;

  public abstract SmsStatus getStatus() throws AdminException;

  protected abstract Map<Integer, Object> loadBodyParameters() throws AdminException;

  private static final String lineSep = System.getProperty("line.separator");
  private static final String doubleSpac = "  ";
  private static final String nullX = " 0x";
  private static final char comma = ',';


  public List<BodyParam> getBodyParameters() throws AdminException {

    List<BodyParam> list = new LinkedList<BodyParam>();
    for(Map.Entry<Integer, Object> e : loadBodyParameters().entrySet())  {

      Integer key = e.getKey();
      Object value = e.getValue();

      String keyMsg;
      try{
        keyMsg = bundle.getString("sms.body.tag." + key);
      }catch (MissingResourceException ex) {
        keyMsg = key.toString();
      }
      if (!isShowText() && keyMsg.equals("SMSC_RAW_PAYLOAD")) continue;
      if (!isShowText() && keyMsg.equals("SMSC_RAW_SHORTMESSAGE")) continue;
      String valMsg;
      if (value instanceof String) {
        valMsg = (String) value;
      } else if (value instanceof Integer) {
        valMsg = value.toString() + nullX + Integer.toHexString((Integer) value);
      } else if (value instanceof byte[]) {
        StringBuilder sb = new StringBuilder();
        byte[] val = (byte[]) value;
        for (int i = 0; i < val.length; i++) {
          if (i > 0) {
            if (i % 16 == 0) sb.append(lineSep);
            else if (i % 8 == 0) sb.append(doubleSpac);
            else
              sb.append(' ');
          }
          int k = (((int) val[i]) >> 4) & 0xf;
          sb.append((char) (k < 10 ? k + 48 : k + 55));
          k = ((int) val[i]) & 0xf;
          sb.append((char) (k < 10 ? k + 48 : k + 55));
        }
        valMsg = sb.toString();
      }else if(value instanceof DataCoding[]) {
        DataCoding[] cs = (DataCoding[])value;
        StringBuilder sb = new StringBuilder();
        boolean first = true;
        for(DataCoding c : cs) {
          if(!first) {
            sb.append(comma);
          }
          first = false;
          sb.append(c);
        }
        valMsg = sb.toString();
      }else if(value instanceof DataCoding) {
        valMsg = value.toString();
      }else {
        valMsg = bundle.getString("smsview.invalidDataType") + ": " + value.getClass().getName();
      }
      list.add(new BodyParam(keyMsg, valMsg));
    }
    return list;
  }


  public static class BodyParam {
    private final String key;
    private final String value;

    public BodyParam(String key, String value) {
      this.key = key;
      this.value = value;
    }

    public String getKey() {
      return key;
    }

    public String getValue() {
      return value;
    }
  }
}
