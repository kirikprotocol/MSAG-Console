package ru.novosoft.smsc.admin.smsview;

/**
 * Title:        SMSC Java Administration
 * Description:
 * Copyright:    Copyright (c) 2002
 * Company:      Novosoft
 * @author
 * @version 1.0
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.CancelMessageData;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsview.archive.SmsArchiveSource;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.Vector;

public class SmsView
{
  private Smsc smsc = null;

  private SmsOperativeSource operative = new SmsOperativeSource();
  private SmsArchiveSource archive = new SmsArchiveSource();

  public void init(SMSCAppContext appContext) throws AdminException
  {
    if (smsc == null) {
      this.smsc = appContext.getSmsc();
      // operative.setDataSource(appContext.getConnectionPool());
      operative.init(appContext);
      archive.init(appContext);

    }
  }

  public SmsSet getSmsSet(SmsQuery query) throws AdminException
  {
    int storage = query.getStorageType();
    if (storage == SmsQuery.SMS_OPERATIVE_STORAGE_TYPE)
      return operative.getSmsSet(query);
    else if (storage == SmsQuery.SMS_ARCHIVE_STORAGE_TYPE)
      return archive.getSmsSet(query);

    throw new AdminException("Unsupported storage type: " + storage);
  }


     public int getArhiveCount(SmsQuery query) throws AdminException
  {
   /* int storage = query.getStorageType();
    if (storage == SmsQuery.SMS_OPERATIVE_STORAGE_TYPE)
      return operative.getSmsCount(query);
    else if (storage == SmsQuery.SMS_ARCHIVE_STORAGE_TYPE)
     */ return archive.getSmsCount(query);

   // throw new AdminException("Unsupported storage type: " + storage);
  }

  public SmsSet getOperativeCount(SmsQuery query) throws AdminException
  {
      return operative.getSmsCount(query);
  }
  public int delOperativeSmsSet(SmsSet set) throws AdminException
  {
    if (smsc == null) throw new AdminException("Smsc not defined");
    int deleted = 0;

    Vector output = new Vector();
    for (int i = 0; i < set.getRowsCount(); i++) {
      SmsRow row = set.getRow(i);
      if (row != null) {
        output.addElement(new CancelMessageData(row.getIdString(), row.getOriginatingAddress(), row.getDestinationAddress()));
        deleted++;
      }
    }

    try {
      smsc.processCancelMessages(output);
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException("Failed to cancel messages on SMSC. Details: " + exc.getMessage());
    }
    return deleted;
  }

}

