package ru.novosoft.smsc.admin.smsview.archive;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.smsview.SmsQuery;

import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.01.2004
 * Time: 20:12:28
 * To change this template use Options | File Templates.
 */
public class QueryMessage extends Message
{
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

  private SmsQuery query = null;

  public QueryMessage(SmsQuery query)
  {
    super(Message.SMSC_BYTE_QUERY_TYPE);
    this.query = query;
  }

  public QueryMessage(byte type, SmsQuery query)
  {
    super(type);
    this.query = query;
  }

  private class Param
  {
    public byte type;
    public long longValue;
    public String strValue;

    public Param(byte type, long longValue)
    {
      this.type = type;
      this.longValue = longValue;
    }

    public Param(byte type, String strValue)
    {
      this.type = type;
      this.strValue = strValue;
    }
  }

  private boolean isMeaning(String str)
  {
    return (str != null && str.length() != 0 && !str.trim().equalsIgnoreCase("*"));
  }

  private boolean isMeaning(int value)
  {
    return (value != SmsQuery.SMS_UNDEFINED_VALUE);
  }

  private List formatParameters() throws IOException
  {
    List list = new ArrayList();

    if (isMeaning(query.getSmsId())) {
      long idValue = 0;
      try {
        idValue = Long.valueOf(query.getSmsId().trim()).longValue();
      } catch (NumberFormatException ex) {
        throw new IOException("Invalid numeric format for sms id");
      }
      list.add(new Param(T_SMS_ID, idValue));
    }
    if (query.getFilterFromDate())
      list.add(new Param(T_FROM_DATE, Message.convertStringToDate(query.getFromDate())));
    if (query.getFilterTillDate())
      list.add(new Param(T_TILL_DATE, Message.convertStringToDate(query.getTillDate())));

    if (isMeaning(query.getAbonentAddress())) {
      try {
        Mask mask = new Mask(query.getAbonentAddress().trim());
        list.add(new Param(T_ABN_ADDRESS, mask.getNormalizedMask()));
      } catch (AdminException e) {
        throw new IOException("Invalid abonent address mask: " + e.getMessage());
      }
    }
    else {
      if (isMeaning(query.getFromAddress())) {
        try {
          Mask mask = new Mask(query.getFromAddress().trim());
          list.add(new Param(T_SRC_ADDRESS, mask.getNormalizedMask()));
        } catch (AdminException e) {
          throw new IOException("Invalid source address mask: " + e.getMessage());
        }
      }
      if (isMeaning(query.getToAddress())) {
        try {
          Mask mask = new Mask(query.getToAddress().trim());
          list.add(new Param(T_DST_ADDRESS, mask.getNormalizedMask()));
        } catch (AdminException e) {
          throw new IOException("Invalid destination address mask: " + e.getMessage());
        }
      }
    }

    if (isMeaning(query.getSmeId())) {
      list.add(new Param(T_SME_ID, query.getSmeId().trim()));
    }
    else {
      if (isMeaning(query.getSrcSmeId()))
        list.add(new Param(T_SRC_SME_ID, query.getSrcSmeId().trim()));
      if (isMeaning(query.getDstSmeId()))
        list.add(new Param(T_DST_SME_ID, query.getDstSmeId().trim()));
    }

    if (isMeaning(query.getRouteId()))
      list.add(new Param(T_ROUTE_ID, query.getRouteId().trim()));

    if (isMeaning(query.getStatus()))
      list.add(new Param(T_STAUS, query.getStatus()));
    if (isMeaning(query.getLastResult()))
      list.add(new Param(T_LAST_RESULT, query.getLastResult()));

    return list;
  }

  public void send(OutputStream os) throws IOException
  {
    List params = formatParameters();
    super.send(os);
    Message.writeUInt8(os, params.size());
    for (int i = 0; i < params.size(); i++) {
      Param param = (Param) params.get(i);
      switch (param.type) {
        case T_SMS_ID:
          Message.writeUInt8(os, param.type);
          Message.writeInt64(os, param.longValue);
          break;
        case T_STAUS:
        case T_LAST_RESULT:
          Message.writeUInt8(os, param.type);
          Message.writeUInt32(os, (int) param.longValue);
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
          Message.writeUInt8(os, param.type);
          Message.writeString32(os, param.strValue);
          break;
        default:
          throw new IOException("Undefined parameter type: " + param.type);
      }
    }
  }
}
