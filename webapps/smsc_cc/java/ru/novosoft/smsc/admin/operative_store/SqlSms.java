package ru.novosoft.smsc.admin.operative_store;

import java.sql.*;

/**
 * author: Aleksandr Khalitov
 */
class SqlSms implements SQLData {
  private String sql_type;
  private long id;
  private int st = 0;
  private Date submitTime = new Date(System.currentTimeMillis());
  private Date validTime;
  private int attempts;
  private int lastResult = 0;
  private Date lastTryTime;
  private Date nextTryTime;
  private String OA; // originatingAddress = "originatingAddress";
  private String DA; //destinationAddress = "destinationAddress";
  private String DDA; //dealiasedDestinationAddress = "dealiasedDestinationAddress";
  private int MR; //messageReference;
  private String SVC_TYPE; //serviceType;
  // public boolean needArchivate;
  private short DR;//deliveryReport;
  private short BR;//billingRecord;
  private String SRC_MSC;
  private String SRC_IMSI;
  private long SRC_SME_N;
  private String DST_MSC;
  private String DST_IMSI;
  private long DST_SME_N;
  //public SmsDescriptor originatingDescriptor;
  //public SmsDescriptor destinationDescriptor;
  private String routeId;
  private long SVC_ID; //serviceId;
  private long PRTY; //priority;
  private String srcSmeId;
  private String dstSmeId;
  private short concatMsgRef;
  private short concatSeqNum;
  private int bodyLen;
  private byte arc = -1;

  public SqlSms() {
  }

  SqlSms(String sql_type, long id, int st, Date submitTime, Date validTime, int attempts, int lastResult,
         Date lastTryTime, Date nextTryTime, String OA, String DA, String DDA, int MR, String SVC_TYPE, short DR,
         short BR, String SRC_MSC, String SRC_IMSI, long SRC_SME_N, String DST_MSC, String DST_IMSI, long DST_SME_N,
         String routeId, long SVC_ID, long PRTY, String srcSmeId, String dstSmeId, short concatMsgRef, short concatSeqNum, byte arc,
         int bodyLen) {
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
    this.DST_MSC = DST_MSC;
    this.DST_IMSI = DST_IMSI;
    this.DST_SME_N = DST_SME_N;
    this.routeId = routeId;
    this.SVC_ID = SVC_ID;
    this.PRTY = PRTY;
    this.srcSmeId = srcSmeId;
    this.dstSmeId = dstSmeId;
    this.concatMsgRef = concatMsgRef;
    this.concatSeqNum = concatSeqNum;
    this.arc = arc;
    this.bodyLen = bodyLen;
  }

  public int getStatusInt() {
    return st;
  }
  ////// implements SQLData //////

  public String getSQLTypeName() throws SQLException {
    return sql_type;
  }

  public void readSQL(SQLInput stream, String typeName)
      throws SQLException {
    sql_type = typeName;

    id = stream.readLong();
    st = stream.readInt();
    submitTime = stream.readDate();
    validTime = stream.readDate();
    attempts = stream.readInt();
    lastResult = stream.readInt();
    lastTryTime = stream.readDate();
    nextTryTime = stream.readDate();
    OA = stream.readString();
    DA = stream.readString();
    DDA = stream.readString();
    MR = stream.readInt();
    SVC_TYPE = stream.readString();
    DR = stream.readShort();
    BR = stream.readShort();
    SRC_MSC = stream.readString();
    SRC_IMSI = stream.readString();
    SRC_SME_N = stream.readLong();
    DST_MSC = stream.readString();
    DST_IMSI = stream.readString();
    DST_SME_N = stream.readLong();
    routeId = stream.readString();
    SVC_ID = stream.readLong();
    PRTY = stream.readLong();
    srcSmeId = stream.readString();
    dstSmeId = stream.readString();
    concatMsgRef = stream.readShort();
    concatSeqNum = stream.readShort();
    arc = stream.readByte();
    bodyLen = stream.readInt();
  }

  public void writeSQL(SQLOutput stream)
      throws SQLException {
    stream.writeLong(id);
    stream.writeInt(st);
    stream.writeDate(submitTime);
    stream.writeDate(validTime);
    stream.writeInt(attempts);
    stream.writeInt(lastResult);
    stream.writeDate(lastTryTime);
    stream.writeDate(nextTryTime);
    stream.writeString(OA);
    stream.writeString(DA);
    stream.writeString(DDA);
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
    stream.writeByte(arc);
    stream.writeInt(bodyLen);
  }

  public long getId() {
    return id;
  }

  public void setId(long id) {
    this.id = id;
  }

  public int getSt() {
    return st;
  }

  public void setSt(int st) {
    this.st = st;
  }

  public Date getSubmitTime() {
    return submitTime;
  }

  public void setSubmitTime(Date submitTime) {
    this.submitTime = submitTime;
  }

  public Date getValidTime() {
    return validTime;
  }

  public void setValidTime(Date validTime) {
    this.validTime = validTime;
  }

  public int getAttempts() {
    return attempts;
  }

  public void setAttempts(int attempts) {
    this.attempts = attempts;
  }

  public int getLastResult() {
    return lastResult;
  }

  public void setLastResult(int lastResult) {
    this.lastResult = lastResult;
  }

  public Date getLastTryTime() {
    return lastTryTime;
  }

  public void setLastTryTime(Date lastTryTime) {
    this.lastTryTime = lastTryTime;
  }

  public Date getNextTryTime() {
    return nextTryTime;
  }

  public void setNextTryTime(Date nextTryTime) {
    this.nextTryTime = nextTryTime;
  }

  public String getOA() {
    return OA;
  }

  public void setOA(String OA) {
    this.OA = OA;
  }

  public String getDA() {
    return DA;
  }

  public void setDA(String DA) {
    this.DA = DA;
  }

  public String getDDA() {
    return DDA;
  }

  public void setDDA(String DDA) {
    this.DDA = DDA;
  }

  public int getMR() {
    return MR;
  }

  public void setMR(int MR) {
    this.MR = MR;
  }

  public String getSVC_TYPE() {
    return SVC_TYPE;
  }

  public void setSVC_TYPE(String SVC_TYPE) {
    this.SVC_TYPE = SVC_TYPE;
  }

  public short getDR() {
    return DR;
  }

  public void setDR(short DR) {
    this.DR = DR;
  }

  public short getBR() {
    return BR;
  }

  public void setBR(short BR) {
    this.BR = BR;
  }

  public String getSRC_MSC() {
    return SRC_MSC;
  }

  public void setSRC_MSC(String SRC_MSC) {
    this.SRC_MSC = SRC_MSC;
  }

  public String getSRC_IMSI() {
    return SRC_IMSI;
  }

  public void setSRC_IMSI(String SRC_IMSI) {
    this.SRC_IMSI = SRC_IMSI;
  }

  public long getSRC_SME_N() {
    return SRC_SME_N;
  }

  public void setSRC_SME_N(long SRC_SME_N) {
    this.SRC_SME_N = SRC_SME_N;
  }

  public String getDST_MSC() {
    return DST_MSC;
  }

  public void setDST_MSC(String DST_MSC) {
    this.DST_MSC = DST_MSC;
  }

  public String getDST_IMSI() {
    return DST_IMSI;
  }

  public void setDST_IMSI(String DST_IMSI) {
    this.DST_IMSI = DST_IMSI;
  }

  public long getDST_SME_N() {
    return DST_SME_N;
  }

  public void setDST_SME_N(long DST_SME_N) {
    this.DST_SME_N = DST_SME_N;
  }

  public String getRouteId() {
    return routeId;
  }

  public void setRouteId(String routeId) {
    this.routeId = routeId;
  }

  public long getSVC_ID() {
    return SVC_ID;
  }

  public void setSVC_ID(long SVC_ID) {
    this.SVC_ID = SVC_ID;
  }

  public long getPRTY() {
    return PRTY;
  }

  public void setPRTY(long PRTY) {
    this.PRTY = PRTY;
  }

  public String getSrcSmeId() {
    return srcSmeId;
  }

  public void setSrcSmeId(String srcSmeId) {
    this.srcSmeId = srcSmeId;
  }

  public String getDstSmeId() {
    return dstSmeId;
  }

  public void setDstSmeId(String dstSmeId) {
    this.dstSmeId = dstSmeId;
  }

  public short getConcatMsgRef() {
    return concatMsgRef;
  }

  public void setConcatMsgRef(short concatMsgRef) {
    this.concatMsgRef = concatMsgRef;
  }

  public short getConcatSeqNum() {
    return concatSeqNum;
  }

  public void setConcatSeqNum(short concatSeqNum) {
    this.concatSeqNum = concatSeqNum;
  }

  public int getBodyLen() {
    return bodyLen;
  }

  public void setBodyLen(int bodyLen) {
    this.bodyLen = bodyLen;
  }

  public byte getArc() {
    return arc;
  }

  public void setArc(byte arc) {
    this.arc = arc;
  }
}
