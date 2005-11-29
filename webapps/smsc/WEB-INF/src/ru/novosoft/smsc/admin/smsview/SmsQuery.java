package ru.novosoft.smsc.admin.smsview;

/**
 * Title:        SMSC Java Administration
 * Description:
 * Copyright:    Copyright (c) 2002
 * Company:      Novosoft
 * @author
 * @version 1.0
 */

import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.AdminException;

import java.util.Date;

import org.apache.log4j.Category;

public class SmsQuery
{
  Category logger = Category.getInstance(SmsQuery.class);

  public final static int SMS_OPERATIVE_STORAGE_TYPE = 100;
  public final static int SMS_ARCHIVE_STORAGE_TYPE = 200;
  public final static int SMS_UNDEFINED_VALUE = -1;

  private int storageType = SMS_ARCHIVE_STORAGE_TYPE;
  private int rowsMaximum = 500;

  private String abonentAddress = "*";
  private Mask   abonentAddressMask = null;
  public boolean isFilterAbonentAddress = false;

  private String fromAddress = "*";
  private Mask   fromAddressMask = null;
  public boolean isFilterFromAddress = false;

  private String toAddress = "*";
  private Mask   toAddressMask = null;
  public boolean isFilterToAddress = false;

  private String smeId = "*";
  public boolean isFilterSmeId = false;

  private String srcSmeId = "*";
  public boolean isFilterSrcSmeId = false;

  private String dstSmeId = "*";
  public boolean isFilterDstSmeId = false;

  private String routeId = "*";
  public boolean isFilterRouteId = false;

  private String smsId = "*";
  public boolean isFilterSmsId = false;
  public long    smsIdValue = -1;

  private Date fromDate = Functions.truncateTime(new Date());
  public boolean isFilterFromDate = true;

  private Date tillDate = new Date();
  public boolean isFilterTillDate = false;

  private int status = SMS_UNDEFINED_VALUE;
  public boolean isFilterStatus = false;

  private int lastResult = SMS_UNDEFINED_VALUE;
  public boolean isFilterLastResult = false;

  private String sortBy = "lastDate";

  public int getStorageType()
  {
    return storageType;
  }

  public void setStorageType(int type)
  {
    storageType = type;
  }

  public int getRowsMaximum()
  {
    return rowsMaximum;
  }

  public void setRowsMaximum(int max)
  {
    rowsMaximum = max;
  }

  public String getAbonentAddress()
  {
    return abonentAddress;
  }

  public boolean isStringHaveValue(String str) {
    if( str != null && str.length() > 0 && !str.equals("*") ) return true;
    return false;
  }

  public void setAbonentAddress(String address)
  {
    abonentAddress = address;
    if( isStringHaveValue(address) ) {
      try {
        abonentAddressMask = new Mask(address);
        isFilterAbonentAddress = true;
      } catch (AdminException e) {
        logger.warn("Invalid address specified: "+address);
        abonentAddressMask = null;
        isFilterAbonentAddress = false;
      }
    } else {
      abonentAddressMask = null;
      isFilterAbonentAddress = false;

    }
  }

  public String getFromAddress()
  {
    return fromAddress;
  }

  public void setFromAddress(String address)
  {
    fromAddress = address;
    if( isStringHaveValue(address) ) {
      try {
        fromAddressMask = new Mask(address);
        isFilterFromAddress = true;
      } catch (AdminException e) {
        logger.warn("Invalid address specified: "+address);
        fromAddressMask = null;
        isFilterFromAddress = false;
      }
    } else {
      fromAddressMask = null;
      isFilterFromAddress = false;
    }
  }

  public String getToAddress()
  {
    return toAddress;
  }

  public void setToAddress(String address)
  {
    toAddress = address;
    if( isStringHaveValue(address) ) {
      try {
        toAddressMask = new Mask(address);
        isFilterToAddress = true;
      } catch (AdminException e) {
        logger.warn("Invalid address specified: "+address);
        toAddressMask = null;
        isFilterToAddress = false;
      }
    } else {
      toAddressMask = null;
      isFilterToAddress = false;
    }
  }

  public String getSmeId()
  {
    return smeId;
  }

  public void setSmeId(String id)
  {
    smeId = id;
    isFilterSmeId = isStringHaveValue(id);
  }

  public String getSrcSmeId()
  {
    return srcSmeId;
  }

  public void setSrcSmeId(String id)
  {
    srcSmeId = id;
    isFilterSrcSmeId = isStringHaveValue(id);
  }

  public String getDstSmeId()
  {
    return dstSmeId;
  }

  public void setDstSmeId(String id)
  {
    dstSmeId = id;
    isFilterDstSmeId = isStringHaveValue(id);
  }

  public String getRouteId()
  {
    return routeId;
  }

  public void setRouteId(String id)
  {
    routeId = id;
    isFilterRouteId = isStringHaveValue(id);
  }

  public String getSmsId()
  {
    return smsId;
  }

  public void setSmsId(String id) throws AdminException
  {
	smsId = id;
	isFilterSmsId = false;
	smsIdValue = -1;
	if(isStringHaveValue(id))
		try
		{
			isFilterSmsId = true;
			smsIdValue = Long.valueOf(id).longValue();
		}
		catch(NumberFormatException e)
		{
			smsId = "*";
			throw new AdminException("Invalid numeric format for sms id");
		}
  }

  public void setFilterFromDate(boolean enabled)
  {
    isFilterFromDate = enabled;
  }

  public boolean getFilterFromDate()
  {
    return isFilterFromDate;
  }

  public Date getFromDate()
  {
    return fromDate;
  }

  public void setFromDate(Date date)
  {
    fromDate = date;
  }

  public void setFilterTillDate(boolean enabled)
  {
    isFilterTillDate = enabled;
  }

  public boolean getFilterTillDate()
  {
    return isFilterTillDate;
  }

  public Date getTillDate()
  {
    return tillDate;
  }

  public void setTillDate(Date date)
  {
    tillDate = date;
  }

  public String getSortBy()
  {
    return sortBy;
  }

  public void setSortBy(String by)
  {
    sortBy = by;
  }

  public int getStatus()
  {
    return status;
  }

  public void setStatus(int status)
  {
    this.status = status;
    isFilterStatus = status != SMS_UNDEFINED_VALUE;
  }

  public int getLastResult()
  {
    return lastResult;
  }

  public void setLastResult(int lastResult)
  {
    this.lastResult = lastResult;
    isFilterLastResult = lastResult != SMS_UNDEFINED_VALUE;
  }

  public long getSmsIdValue() {
    return smsIdValue;
  }

  public Mask getAbonentAddressMask() {
    return abonentAddressMask;
  }

  public Mask getFromAddressMask() {
    return fromAddressMask;
  }

  public Mask getToAddressMask() {
    return toAddressMask;
  }
}
