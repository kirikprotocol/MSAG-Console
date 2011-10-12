package ru.novosoft.smsc.web.controllers.sms_view;

import ru.novosoft.smsc.util.Address;

import java.util.Date;

/**
 * @author Artem Snopkov
 */
public class SmsQuery {

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
  private Integer lastResult;
  private Integer maxRows = 1000;
  private SmsStatus smsStatus;

  SmsQuery() {
  }

  public SmsQuery(SmsQuery q) {
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
    maxRows = q.getMaxRows();
    smsStatus = q.getSmsStatus();
  }

  public void clear() {
    abonentAddress = null;
    fromAddress = null;
    toAddress = null;
    smeId = null;
    srcSmeId = null;
    dstSmeId = null;
    routeId = null;
    smsId = null;
    fromDate = null;
    tillDate = null;
    lastResult = null;
    smsStatus = null;
  }

  public Address getAbonentAddress() {
    return abonentAddress;
  }

  public void setAbonentAddress(Address abonentAddress) {
    this.abonentAddress = abonentAddress;
  }

  public Address getFromAddress() {
    return fromAddress;
  }

  public void setFromAddress(Address fromAddress) {
    this.fromAddress = fromAddress;
  }

  public Address getToAddress() {
    return toAddress;
  }

  public void setToAddress(Address toAddress) {
    this.toAddress = toAddress;
  }

  public String getSmeId() {
    return smeId;
  }

  public void setSmeId(String smeId) {
    if (smeId != null && smeId.trim().length() == 0)
      smeId = null;
    this.smeId = smeId;
  }

  public String getSrcSmeId() {
    return srcSmeId;
  }

  public void setSrcSmeId(String srcSmeId) {
    if (srcSmeId != null && srcSmeId.trim().length() == 0)
      srcSmeId = null;
    this.srcSmeId = srcSmeId;
  }

  public String getDstSmeId() {
    return dstSmeId;
  }

  public void setDstSmeId(String dstSmeId) {
    if (dstSmeId != null && dstSmeId.trim().length() == 0)
      dstSmeId = null;
    this.dstSmeId = dstSmeId;
  }

  public String getRouteId() {
    return routeId;
  }

  public void setRouteId(String routeId) {
    if (routeId != null && routeId.trim().length() == 0)
      routeId = null;
    this.routeId = routeId;
  }

  public Long getSmsId() {
    return smsId;
  }

  public void setSmsId(Long smsId) {
    if (smsId!= null && smsId == Long.MIN_VALUE)
      smsId = null;
    this.smsId = smsId;
  }

  public Date getFromDate() {
    return fromDate;
  }

  public void setFromDate(Date fromDate) {
    this.fromDate = fromDate;
  }

  public Date getTillDate() {
    return tillDate;
  }

  public void setTillDate(Date tillDate) {
    this.tillDate = tillDate;
  }

  public Integer getLastResult() {
    return lastResult;
  }

  public void setLastResult(Integer lastResult) {
    if (lastResult != null && lastResult == Integer.MIN_VALUE)
      lastResult = null;
    this.lastResult = lastResult;
  }

  public Integer getMaxRows() {
    return maxRows;
  }

  public void setMaxRows(Integer maxRows) {
    if (maxRows != null && maxRows == Integer.MIN_VALUE)
      maxRows = null;
    this.maxRows = maxRows;
  }

  public SmsStatus getSmsStatus() {
    return smsStatus;
  }

  public void setSmsStatus(SmsStatus smsStatus) {
    this.smsStatus = smsStatus;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    SmsQuery smsQuery = (SmsQuery) o;

    if (abonentAddress != null ? !abonentAddress.equals(smsQuery.abonentAddress) : smsQuery.abonentAddress != null)
      return false;
    if (dstSmeId != null ? !dstSmeId.equals(smsQuery.dstSmeId) : smsQuery.dstSmeId != null) return false;
    if (fromAddress != null ? !fromAddress.equals(smsQuery.fromAddress) : smsQuery.fromAddress != null) return false;
    if (fromDate != null ? !fromDate.equals(smsQuery.fromDate) : smsQuery.fromDate != null) return false;
    if (lastResult != null ? !lastResult.equals(smsQuery.lastResult) : smsQuery.lastResult != null) return false;
    if (maxRows != null ? !maxRows.equals(smsQuery.maxRows) : smsQuery.maxRows != null) return false;
    if (routeId != null ? !routeId.equals(smsQuery.routeId) : smsQuery.routeId != null) return false;
    if (smeId != null ? !smeId.equals(smsQuery.smeId) : smsQuery.smeId != null) return false;
    if (smsId != null ? !smsId.equals(smsQuery.smsId) : smsQuery.smsId != null) return false;
    if (smsStatus != smsQuery.smsStatus) return false;
    if (srcSmeId != null ? !srcSmeId.equals(smsQuery.srcSmeId) : smsQuery.srcSmeId != null) return false;
    if (tillDate != null ? !tillDate.equals(smsQuery.tillDate) : smsQuery.tillDate != null) return false;
    if (toAddress != null ? !toAddress.equals(smsQuery.toAddress) : smsQuery.toAddress != null) return false;

    return true;
  }
}
