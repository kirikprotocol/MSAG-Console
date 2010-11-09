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
  private Integer maxRows;

  SmsQuery() {
  }

  public void clear() {
    Address abonentAddress = null;
    Address fromAddress = null;
    Address toAddress = null;
    String smeId = null;
    String srcSmeId = null;
    String dstSmeId = null;
    String routeId = null;
    Long smsId = null;
    Date fromDate = null;
    Date tillDate = null;
    Integer lastResult = null;
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
    this.smeId = smeId;
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

  public String getRouteId() {
    return routeId;
  }

  public void setRouteId(String routeId) {
    this.routeId = routeId;
  }

  public Long getSmsId() {
    return smsId;
  }

  public void setSmsId(Long smsId) {
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
    this.lastResult = lastResult;
  }

  public Integer getMaxRows() {
    return maxRows;
  }

  public void setMaxRows(Integer maxRows) {
    this.maxRows = maxRows;
  }
}