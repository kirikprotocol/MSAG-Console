/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: May 20, 2003
 * Time: 7:29:27 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.jsp.smsview;

import ru.novosoft.smsc.admin.smsview.SmsView;
import ru.novosoft.smsc.admin.smsview.SmsQuery;
import ru.novosoft.smsc.admin.smsview.SmsDetailedRow;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.List;

public class SmsViewDetailsFormBean extends IndexBean
{
  private SmsView view = new SmsView();
  private SmsDetailedRow detailedRow = null;

  private String smsId = "";
  private String mbView = null;
  private int storageType = SmsQuery.SMS_ARCHIVE_STORAGE_TYPE;

  public int process(SMSCAppContext appContext, List errors, java.security.Principal loginedPrincipal)
  {
    if (this.appContext == null && appContext instanceof SMSCAppContext)
    {
      view.setDataSource(appContext.getConnectionPool());
      view.setSmsc(appContext.getSmsc());
    }

    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbView != null) result = processSmsQuery();

    mbView = null;
    return result;
  }

  public int processSmsQuery()
  {
    try {
      detailedRow = view.getSms(smsId, storageType);
      return RESULT_OK;
    } catch (AdminException ex) {
      ex.printStackTrace();
      return error(SMSCErrors.error.smsview.QueryFailed, ex.getMessage());
    }
  }

  public String getSmsId() {
    return smsId;
  }
  public void setSmsId(String smsId) {
    this.smsId = smsId;
  }

  public int getStorageType() {
    return storageType;
  }
  public void setStorageType(int storageType) {
    this.storageType = storageType;
  }

  public String getMbView() {
    return mbView;
  }
  public void setMbView(String mbView) {
    this.mbView = mbView;
  }

  public SmsDetailedRow getDetailedRow() {
    return detailedRow;
  }
}
