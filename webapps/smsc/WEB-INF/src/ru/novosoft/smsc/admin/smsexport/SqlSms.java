package ru.novosoft.smsc.admin.smsexport;

import java.sql.*;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 11.05.2005
 * Time: 15:22:35
 * To change this template use File | Settings | File Templates.
 */

public class SqlSms implements SQLData
{
  private String sql_type;
  public long id;
  public int st = 0;
  public Date submitTime = new Date(System.currentTimeMillis());
  public Date validTime;
  public int attempts;
  public int lastResult = 0;
  public Date lastTryTime;
  public Date nextTryTime;
  public String OA; // originatingAddress = "originatingAddress";
  public String DA; //destinationAddress = "destinationAddress";
  public String DDA; //dealiasedDestinationAddress = "dealiasedDestinationAddress";
  public int MR; //messageReference;
  public String SVC_TYPE; //serviceType;
 // public boolean needArchivate;
  public short DR;//deliveryReport;
  public short BR;//billingRecord;
  public String SRC_MSC;
  public String SRC_IMSI;
  public long SRC_SME_N ;
  public String DST_MSC ;
  public String DST_IMSI;
  public long DST_SME_N ;
  //public SmsDescriptor originatingDescriptor;
  //public SmsDescriptor destinationDescriptor;
  public String routeId;
  public long SVC_ID; //serviceId;
  public long PRTY; //priority;
  public String srcSmeId;
  public String dstSmeId;
  public short concatMsgRef;
  public short concatSeqNum;
  public int bodyLen;
  public byte arc=-1;

   public SqlSms()
  {
  }

  public SqlSms(String sql_type, long id, int st ,Date submitTime ,Date validTime ,int attempts ,int lastResult,
         Date lastTryTime, Date nextTryTime , String OA ,String DA , String DDA , int MR ,String SVC_TYPE , short DR,
         short BR ,String SRC_MSC ,String SRC_IMSI ,long SRC_SME_N  ,String DST_MSC ,String DST_IMSI,long DST_SME_N ,
         String routeId,long SVC_ID ,long PRTY ,String srcSmeId ,String dstSmeId ,short concatMsgRef,short concatSeqNum,
         int bodyLen)
    {
      this.sql_type = sql_type;
      this.id = id;
      this.st = st;
      this.submitTime = submitTime;
      this.validTime = validTime;
      this.attempts = attempts;
      this.lastResult = lastResult;
      this.lastTryTime = lastTryTime;
      this.nextTryTime = nextTryTime;
      this.OA = OA;
      this.DA = DA;
      this.DDA = DDA;
      this.MR = MR;
      this.SVC_TYPE = SVC_TYPE;
      this.DR = DR;
      this.BR = BR;
      this.SRC_MSC = SRC_MSC;
      this.SRC_IMSI = SRC_IMSI;
      this.SRC_SME_N = SRC_SME_N;
      this.DST_MSC =  DST_MSC;
      this.DST_IMSI = DST_IMSI;
      this.DST_SME_N = DST_SME_N;
      this.routeId = routeId;
      this.SVC_ID = SVC_ID;
      this.PRTY = PRTY;
      this.srcSmeId = srcSmeId;
      this.dstSmeId = dstSmeId;
      this.concatMsgRef = concatMsgRef;
      this.concatSeqNum = concatSeqNum;
      this.bodyLen = bodyLen;
    }

  public int getStatusInt()
  {
    return st;
  }
  ////// implements SQLData //////

 public String getSQLTypeName() throws SQLException
 {
   return sql_type;
 }

 public void readSQL(SQLInput stream, String typeName)
   throws SQLException
 {
   sql_type = typeName;

   id = stream.readLong();
   st = stream.readInt();
   submitTime =stream.readDate();
   validTime = stream.readDate();
   attempts = stream.readInt();
   lastResult = stream.readInt();
   lastTryTime = stream.readDate();
   nextTryTime = stream.readDate();
   OA = stream.readString();
   DA = stream.readString();
   DDA =stream.readString();
   MR = stream.readInt();
   SVC_TYPE = stream.readString();
   DR =stream.readShort();
   BR = stream.readShort();
   SRC_MSC = stream.readString();
   SRC_IMSI = stream.readString();
   SRC_SME_N = stream.readLong();
   DST_MSC =  stream.readString();
   DST_IMSI = stream.readString();
   DST_SME_N = stream.readLong();
   routeId = stream.readString();
   SVC_ID = stream.readLong();
   PRTY = stream.readLong();
   srcSmeId = stream.readString();
   dstSmeId = stream.readString();
   concatMsgRef = stream.readShort();
   concatSeqNum = stream.readShort();
   bodyLen = stream.readInt();
   arc=stream.readByte();
 }

 public void writeSQL(SQLOutput stream)
   throws SQLException
 {
   stream.writeLong(id);
   stream.writeInt(st);
   stream.writeDate( submitTime);
   stream.writeDate(validTime);
   stream.writeInt(attempts);
   stream.writeInt(lastResult);
   stream.writeDate(lastTryTime);
   stream.writeDate(nextTryTime);
   stream.writeString(OA );
   stream.writeString(DA);
   stream.writeString(DDA );
   stream.writeInt(MR);
   stream.writeString(SVC_TYPE);
   stream.writeShort(DR);
   stream.writeShort(BR);
   stream.writeString(SRC_MSC);
   stream.writeString(SRC_IMSI);
   stream.writeLong(SRC_SME_N);
   stream.writeString(DST_MSC);
   stream.writeString(DST_IMSI);
   stream.writeLong(DST_SME_N);
   stream.writeString(routeId);
   stream.writeLong(SVC_ID);
   stream.writeLong(PRTY);
   stream.writeString(srcSmeId);
   stream.writeString(dstSmeId);
   stream.writeShort(concatMsgRef);
   stream.writeShort(concatSeqNum);
   stream.writeInt(bodyLen);
   stream.writeByte(arc);
 }

  public byte getArc()
  {
    return arc;
  }

  public void setArc(byte arc)
  {
    this.arc = arc;
  }
}
