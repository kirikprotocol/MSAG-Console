package ru.novosoft.smsc.admin.archive_daemon;


import org.apache.log4j.Category;
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
  private static final Category logger = Category.getInstance(SmsQuery.class);

  public final static int SMS_UNDEFINED_VALUE = -1;
  private int rowsMaximum = 500;

  private String abonentAddress = "*";
  private Address abonentAddressMask = null;
  private boolean isFilterAbonentAddress = false;

  private String fromAddress = "*";
  private Address fromAddressMask = null;
  private boolean isFilterFromAddress = false;

  private String toAddress = "*";
  private Address toAddressMask = null;
  private boolean isFilterToAddress = false;

  private String smeId = "*";
  private boolean isFilterSmeId = false;

  private String srcSmeId = "*";
  private boolean isFilterSrcSmeId = false;

  private String dstSmeId = "*";
  private boolean isFilterDstSmeId = false;

  private String routeId = "*";
  private boolean isFilterRouteId = false;

  private String smsId = "*";
  public boolean isFilterSmsId = false;
  private long smsIdValue = -1;

  private Date fromDate = Functions.truncateTime(new Date());
  private boolean isFilterFromDate = true;

  private Date tillDate = new Date();
  private boolean isFilterTillDate = false;

  private int status = SMS_UNDEFINED_VALUE;
  private boolean isFilterStatus = false;

  private int lastResult = SMS_UNDEFINED_VALUE;
  private boolean isFilterLastResult = false;

  private String sortBy = "lastDate";

  public int getRowsMaximum() {
    return rowsMaximum;
  }

  public void setRowsMaximum(int max) {
    rowsMaximum = max;
  }

  public String getAbonentAddress() {
    return abonentAddress;
  }

  public boolean isStringHaveValue(String str) {
    return str != null && str.length() > 0 && !str.equals("*");
  }

  public void setAbonentAddress(String address) {
    abonentAddress = address;
    if (isStringHaveValue(address)) {
      try {
        abonentAddressMask = new Address(address);
        isFilterAbonentAddress = true;
      } catch (IllegalArgumentException e) {
        logger.warn("Invalid address specified: " + address);
        abonentAddressMask = null;
        isFilterAbonentAddress = false;
      }
    } else {
      abonentAddressMask = null;
      isFilterAbonentAddress = false;

    }
  }

  public String getFromAddress() {
    return fromAddress;
  }

  public void setFromAddress(String address) {
    fromAddress = address;
    if (isStringHaveValue(address)) {
      try {
        fromAddressMask = new Address(address);
        isFilterFromAddress = true;
      } catch (IllegalArgumentException e) {
        logger.warn("Invalid address specified: " + address);
        fromAddressMask = null;
        isFilterFromAddress = false;
      }
    } else {
      fromAddressMask = null;
      isFilterFromAddress = false;
    }
  }

  public String getToAddress() {
    return toAddress;
  }

  public void setToAddress(String address) {
    toAddress = address;
    if (isStringHaveValue(address)) {
      try {
        toAddressMask = new Address(address);
        isFilterToAddress = true;
      } catch (IllegalArgumentException e) {
        logger.warn("Invalid address specified: " + address);
        toAddressMask = null;
        isFilterToAddress = false;
      }
    } else {
      toAddressMask = null;
      isFilterToAddress = false;
    }
  }

  public String getSmeId() {
    return smeId;
  }

  public void setSmeId(String id) {
    smeId = id;
    isFilterSmeId = isStringHaveValue(id);
  }

  public String getSrcSmeId() {
    return srcSmeId;
  }

  public void setSrcSmeId(String id) {
    srcSmeId = id;
    isFilterSrcSmeId = isStringHaveValue(id);
  }

  public String getDstSmeId() {
    return dstSmeId;
  }

  public void setDstSmeId(String id) {
    dstSmeId = id;
    isFilterDstSmeId = isStringHaveValue(id);
  }

  public String getRouteId() {
    return routeId;
  }

  public void setRouteId(String id) {
    routeId = id;
    isFilterRouteId = isStringHaveValue(id);
  }

  public String getSmsId() {
    return smsId;
  }

  public void setSmsId(String id) throws AdminException {
    smsId = id;
    isFilterSmsId = false;
    smsIdValue = -1;
    if (isStringHaveValue(id))
      try {
        isFilterSmsId = true;
        smsIdValue = Long.parseLong(id);
      }
      catch (NumberFormatException e) {
        smsId = "*";
        throw new IllegalArgumentException("Invalid numeric format for sms id");
      }
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

  public int getStatus() {
    return status;
  }

  public void setStatus(int status) {
    this.status = status;
    isFilterStatus = status != SMS_UNDEFINED_VALUE;
  }

  public int getLastResult() {
    return lastResult;
  }

  public void setLastResult(int lastResult) {
    this.lastResult = lastResult;
    isFilterLastResult = lastResult != SMS_UNDEFINED_VALUE;
  }

  public long getSmsIdValue() {
    return smsIdValue;
  }

  public Address getAbonentAddressMask() {
    return abonentAddressMask;
  }

  public Address getFromAddressMask() {
    return fromAddressMask;
  }

  public Address getToAddressMask() {
    return toAddressMask;
  }

  public String getJournalInfo() {
    StringBuilder result = new StringBuilder();
    if (isFilterAbonentAddress) result.append("abonent address: ").append(abonentAddress).append(", ");
    if (isFilterFromAddress) result.append("from address: ").append(fromAddress).append(", ");
    if (isFilterToAddress) result.append("to address: ").append(toAddress).append(", ");
    if (isFilterSmeId) result.append("sme id: ").append(smeId).append(", ");
    if (isFilterSrcSmeId) result.append("src sme id: ").append(srcSmeId).append(", ");
    if (isFilterDstSmeId) result.append("dst sme id: ").append(dstSmeId).append(", ");
    if (isFilterRouteId) result.append("route id: ").append(routeId).append(", ");
    if (isFilterSmsId) result.append("sms id: ").append(smsId).append(", ");
    if (isFilterFromDate) result.append("from date: ").append(fromDate).append(", ");
    if (isFilterTillDate) result.append("till date: ").append(tillDate).append(", ");
    if (isFilterStatus) {
      String status;
      switch (this.status) {
        case 0:
          status = "ENROUTE";
          break;
        case 1:
          status = "DELIVERED";
          break;
        case 2:
          status = "EXPIRED";
          break;
        case 3:
          status = "UNDELIVERABLE";
          break;
        case 4:
          status = "DELETED";
          break;
        default:
          status = "ALL";
      }
      result.append("sms status: ").append(status).append(", ");
    }
    if (isFilterLastResult) {
      result.append("last result: ").append(lastResult).append(", ");
    }
    return result.toString();
  }
}
