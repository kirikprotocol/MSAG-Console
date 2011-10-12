package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.Functions;

import java.util.Date;

/**
 * Запрос для извлечения записей ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */
public class SmsQuery {

  private int rowsMaximum = 500;

  private Address abonentAddress;

  private Address fromAddress;

  private Address toAddress;

  private String smeId;

  private String srcSmeId;

  private String dstSmeId;

  private String routeId;

  private Long smsId;

  private Date fromDate = Functions.truncateTime(new Date());
  private boolean isFilterFromDate = true;

  private Date tillDate = new Date();
  private boolean isFilterTillDate = false;

  private Integer status;

  private Integer lastResult;

  private String sortBy = "lastDate";

  public int getRowsMaximum() {
    return rowsMaximum;
  }

  public void setRowsMaximum(int max) {
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
    smeId = id;
  }

  public String getSrcSmeId() {
    return srcSmeId;
  }

  public void setSrcSmeId(String id) {
    srcSmeId = id;
  }

  public String getDstSmeId() {
    return dstSmeId;
  }

  public void setDstSmeId(String id) {
    dstSmeId = id;
  }

  public String getRouteId() {
    return routeId;
  }

  public void setRouteId(String id) {
    routeId = id;
  }

  public Long getSmsId() {
    return smsId;
  }

  public void setSmsId(Long id) throws AdminException {
    smsId = id;
  }

  public void setFilterFromDate(boolean enabled) {
    isFilterFromDate = enabled;
  }

  public boolean getFilterFromDate() {
    return isFilterFromDate;
  }

  public Date getFromDate() {
    return fromDate;
  }

  public void setFromDate(Date date) {
    fromDate = date;
  }

  public void setFilterTillDate(boolean enabled) {
    isFilterTillDate = enabled;
  }

  public boolean getFilterTillDate() {
    return isFilterTillDate;
  }

  public Date getTillDate() {
    return tillDate;
  }

  public void setTillDate(Date date) {
    tillDate = date;
  }

  public String getSortBy() {
    return sortBy;
  }

  public void setSortBy(String by) {
    sortBy = by;
  }

  public Integer getStatus() {
    return status;
  }

  public void setStatus(Integer status) {
    this.status = status;
  }

  public Integer getLastResult() {
    return lastResult;
  }

  public void setLastResult(Integer lastResult) {
    this.lastResult = lastResult;
  }


}
