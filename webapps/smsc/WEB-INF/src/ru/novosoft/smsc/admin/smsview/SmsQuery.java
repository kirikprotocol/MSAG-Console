package ru.novosoft.smsc.admin.smsview;

/**
 * Title:        SMSC Java Administration
 * Description:
 * Copyright:    Copyright (c) 2002
 * Company:      Novosoft
 * @author
 * @version 1.0
 */

import java.util.Vector;
import java.util.Date;

public class SmsQuery
{
  public static int SMS_OPERATIVE_STORAGE_TYPE = 100;
  public static int SMS_ARCHIVE_STORAGE_TYPE = 200;

  public int    storageType = SMS_ARCHIVE_STORAGE_TYPE;

  public String fromAddress = "***";
  public String toAddress = "***";

  public Date   fromDate;
  public Date   tillDate;

  public int    rowsCount = 10;
  public int    rowIndex = 0;

  public String sortBy = "Date";

};