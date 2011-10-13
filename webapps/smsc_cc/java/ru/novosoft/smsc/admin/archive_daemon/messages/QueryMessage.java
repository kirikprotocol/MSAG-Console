package ru.novosoft.smsc.admin.archive_daemon.messages;

import ru.novosoft.smsc.admin.archive_daemon.ArchiveMessageFilter;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.IOUtils;

import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

/**
 * Служебный класс для работы с ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */
public class QueryMessage extends Message {
  public final static byte T_SMS_ID = 10;
  public final static byte T_FROM_DATE = 20;
  public final static byte T_TILL_DATE = 30;
  public final static byte T_SRC_ADDRESS = 40;
  public final static byte T_DST_ADDRESS = 50;
  public final static byte T_SRC_SME_ID = 60;
  public final static byte T_DST_SME_ID = 70;
  public final static byte T_ROUTE_ID = 80;
  public final static byte T_ABN_ADDRESS = 90;
  public final static byte T_SME_ID = 100;
  public final static byte T_STAUS = 110;
  public final static byte T_LAST_RESULT = 120;

  private ArchiveMessageFilter query = null;

  public QueryMessage(ArchiveMessageFilter query) {
    super(SMSC_BYTE_QUERY_TYPE);
    this.query = query;
  }

  public QueryMessage(byte type, ArchiveMessageFilter query) {
    super(type);
    this.query = query;
  }

  private static class Param {
    private byte type;
    private long longValue;
    private String strValue;

    Param(byte type, long longValue) {
      this.type = type;
      this.longValue = longValue;
    }

    private Param(byte type, String strValue) {
      this.type = type;
      this.strValue = strValue;
    }
  }


  private List<Param> formatParameters() throws IOException {
    List<Param> list = new ArrayList<Param>();

    if (query.getSmsId() != null) {
      long idValue;
      try {
        idValue = query.getSmsId();
      } catch (NumberFormatException ex) {
        throw new IOException("Invalid numeric format for sms id");
      }
      list.add(new Param(T_SMS_ID, idValue));
    }
    if (query.getFromDate() != null)
      list.add(new Param(T_FROM_DATE, convertStringToDate(query.getFromDate())));
    if (query.getTillDate() != null)
      list.add(new Param(T_TILL_DATE, convertStringToDate(query.getTillDate())));

    if (query.getAbonentAddress() != null) {

      Address address = new Address(query.getAbonentAddress());
      list.add(new Param(T_ABN_ADDRESS, address.getNormalizedAddress()));

    } else {
      if (query.getFromAddress() != null) {

        Address address = query.getFromAddress();
        list.add(new Param(T_SRC_ADDRESS, address.getNormalizedAddress()));

      }
      if (query.getToAddress() != null) {
        Address address = query.getToAddress();
        list.add(new Param(T_DST_ADDRESS, address.getNormalizedAddress()));
      }
    }

    if (query.getSmeId() != null) {
      list.add(new Param(T_SME_ID, query.getSmeId().trim()));
    } else {
      if (query.getSrcSmeId() != null)
        list.add(new Param(T_SRC_SME_ID, query.getSrcSmeId().trim()));
      if (query.getDstSmeId() != null)
        list.add(new Param(T_DST_SME_ID, query.getDstSmeId().trim()));
    }

    if (query.getRouteId() != null)
      list.add(new Param(T_ROUTE_ID, query.getRouteId().trim()));

    if (query.getStatus() != null)
      list.add(new Param(T_STAUS, query.getStatus().toString()));
    if (query.getLastResult() != null)
      list.add(new Param(T_LAST_RESULT, query.getLastResult()));

    return list;
  }

  public void send(OutputStream os) throws IOException {
    List<Param> params = formatParameters();
    super.send(os);
    IOUtils.writeUInt8(os, params.size());
    for (Param param : params) {
      switch (param.type) {
        case T_SMS_ID:
          IOUtils.writeUInt8(os, param.type);
          IOUtils.writeInt64(os, param.longValue);
          break;
        case T_STAUS:
        case T_LAST_RESULT:
          IOUtils.writeUInt8(os, param.type);
          IOUtils.writeUInt32(os, (int) param.longValue);
          break;
        case T_FROM_DATE:
        case T_TILL_DATE:
        case T_ROUTE_ID:
        case T_SME_ID:
        case T_SRC_SME_ID:
        case T_DST_SME_ID:
        case T_SRC_ADDRESS:
        case T_DST_ADDRESS:
        case T_ABN_ADDRESS:
          IOUtils.writeUInt8(os, param.type);
          IOUtils.writeString32(os, param.strValue);
          break;
        default:
          throw new IOException("Undefined parameter type: " + param.type);
      }
    }
  }
}
