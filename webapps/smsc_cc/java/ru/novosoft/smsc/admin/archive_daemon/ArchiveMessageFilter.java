package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.util.Address;

import java.util.Date;

/**
 * Запрос для извлечения записей ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */
public class ArchiveMessageFilter {

  private int rowsMaximum = 500;

  private Address abonentAddress;

  private Address fromAddress;

  private Address toAddress;

  private String smeId;

  private String srcSmeId;

  private String dstSmeId;

  private String routeId;

  private Long smsId;

  private Date fromDate;

  private Date tillDate;

  private SmsRow.Status status;

  private Integer lastResult;

  public ArchiveMessageFilter() {
  }

  public ArchiveMessageFilter(ArchiveMessageFilter q) {
    abonentAddress = q.getAbonentAddress() == null ? null : new Address(q.getAbonentAddress());
    fromAddress = q.getFromAddress() == null ? null : new Address(q.getFromAddress());
    toAddress = q.getToAddress() == null ? null : new Address(q.getToAddress());
    smeId = q.getSmeId();
    srcSmeId = q.getSrcSmeId();
    dstSmeId = q.getDstSmeId();
    routeId = q.getRouteId();
    smsId = q.getSmsId();
    fromDate = q.getFromDate() == null ? null : new Date(q.getFromDate().getTime());
    tillDate = q.getTillDate() == null ? null : new Date(q.getTillDate().getTime());
    lastResult = q.getLastResult();
    rowsMaximum = q.getRowsMaximum();
    status = q.getStatus();
  }

  public Integer getRowsMaximum() {
    return rowsMaximum;
  }

  public void setRowsMaximum(Integer max) {
    rowsMaximum = max;
  }

  public Address getAbonentAddress() {
    return abonentAddress;
  }

  public void setAbonentAddress(Address address) {
    abonentAddress = address;
  }

  public Address getFromAddress() {
    return fromAddress;
  }

  public void setFromAddress(Address address) {
    fromAddress = address;
  }

  public Address getToAddress() {
    return toAddress;
  }

  public void setToAddress(Address address) {
    toAddress = address;
  }

  public String getSmeId() {
    return smeId;
  }

  public void setSmeId(String id) {
    smeId = id == null || id.length() == 0 ? null : id;
  }

  public String getSrcSmeId() {
    return srcSmeId;
  }

  public void setSrcSmeId(String id) {
    srcSmeId = id == null || id.length() == 0 ? null : id;
  }

  public String getDstSmeId() {
    return dstSmeId;
  }

  public void setDstSmeId(String id) {
    dstSmeId = id == null || id.length() == 0 ? null : id;
  }

  public String getRouteId() {
    return routeId;
  }

  public void setRouteId(String id) {
    routeId = id == null || id.length() == 0 ? null : id;
  }

  public Long getSmsId() {
    return smsId;
  }

  public void setSmsId(Long id) {
    smsId = id;
  }


  public Date getFromDate() {
    return fromDate;
  }

  public void setFromDate(Date date) {
    fromDate = date;
  }

  public Date getTillDate() {
    return tillDate;
  }

  public void setTillDate(Date date) {
    tillDate = date;
  }

  public SmsRow.Status getStatus() {
    return status;
  }

  public void setStatus(SmsRow.Status status) {
    this.status = status;
  }

  public Integer getLastResult() {
    return lastResult;
  }

  public void setLastResult(Integer lastResult) {
    this.lastResult = lastResult;
  }

  @Override
  public String toString() {
    return "ArchiveMessageFilter{" +
        "rowsMax=" + rowsMaximum +
        ", abAddr=" + abonentAddress +
        ", fromAddr=" + fromAddress +
        ", toAddr=" + toAddress +
        ", smeId='" + smeId + '\'' +
        ", srcSmeId='" + srcSmeId + '\'' +
        ", dstSmeId='" + dstSmeId + '\'' +
        ", routeId='" + routeId + '\'' +
        ", smsId=" + smsId +
        ", fromDate=" + fromDate +
        ", tillDate=" + tillDate +
        ", status=" + status +
        ", lastResult=" + lastResult +
        '}';
  }
}
