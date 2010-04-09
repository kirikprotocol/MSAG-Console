package mobi.eyeline.welcomesms.beans;

import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Map;

import mobi.eyeline.welcomesms.backend.datasources.NetworkFilter;
import mobi.eyeline.welcomesms.backend.datasources.NetworkDataSource;
import mobi.eyeline.welcomesms.backend.tables.NetworkTable;

/**
 * author: alkhal
 */
public class Networks extends WelcomeSMSBean{

  public static final int RESULT_EDIT = WelcomeSMSBean.PRIVATE_RESULT + 1;
  public static final int RESULT_ADD =  WelcomeSMSBean.PRIVATE_RESULT + 2;

  private String mbAdd = null;
  private String mbDelete = null;

  private NetworkTable tableHelper = new NetworkTable("networks");

  private String name;
  private String countryId;
  private String mcc;
  private String mnc;

  private String selectedId;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      NetworkFilter filter = new NetworkFilter();
      tableHelper.setPageSize(20);
      tableHelper.setDataSource(new NetworkDataSource(welcomeSMSContext));
      tableHelper.setFilter(filter);
      tableHelper.setContext(welcomeSMSContext);
      if(name != null && (name = name.trim()).length() != 0) {
        filter.setName(name);
      }
      if(mcc != null && (mcc = mcc.trim()).length() != 0) {
        filter.setMcc(convertStringToInt(mcc));
      }
      if(mnc != null && (mnc = mnc.trim()).length() != 0) {
        filter.setMnc(convertStringToInt(mnc));
      }
      if(countryId != null && (countryId = countryId.trim()).length() != 0) {
        filter.setCountryId(countryId);
      }
    } catch (Exception e) {
      logger.error(e,e);
      e.printStackTrace();
      return RESULT_ERROR;
    }

    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    try {
      if (tableHelper.isDataCellSelected()) {
        result = processEdit(tableHelper.getSelectedCellId());

      } else if (mbAdd != null) {
        mbAdd = null;
        result = processAdd();

      } else if (mbDelete != null) {
        mbDelete = null;
        result = processDelete(request);
      }

      this.tableHelper.fillTable();

    } catch (Exception e) {
      logger.error("Can't process request", e);
      return _error(new SMSCJspException("Can't create table", SMSCJspException.ERROR_CLASS_ERROR, e));
    }

    return result;
  }

  private int processDelete(HttpServletRequest request) {
    try {
      welcomeSMSContext.getNetworksManager().removeNetworks(tableHelper.getSelected(request));
      tableHelper.setStartPosition(0);
    } catch (Exception e) {
      logger.error(e,e);
      e.printStackTrace();
    }

    return RESULT_OK;
  }

  public Map getCountries() {
    return welcomeSMSContext.getLocationService().getCountries();
  }

  private int processAdd() {
    return RESULT_ADD;
  }

  private int processEdit(String cellId) {
    selectedId = cellId;
    return RESULT_EDIT;
  }

  public String getMbAdd() {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd) {
    this.mbAdd = mbAdd;
  }

  public String getMbDelete() {
    return mbAdd;
  }

  public void setMbDelete(String mbDelete) {
    this.mbDelete = mbDelete;
  }

  public PagedStaticTableHelper getTableHelper() {
    return tableHelper;
  }


  public String getSelectedId() {
    return selectedId;
  }

  public String getName() {
    return (name == null) ? "" : name;
  }

  public void setName(String name) {
    this.name = name;
  }

  private Integer convertStringToInt(String s) {
    try {
      return Integer.valueOf(s);
    } catch (NumberFormatException e) {
      logger.warn(e,e);
    }
    return null;
  }

  public String getCountryId() {
    return countryId;
  }

  public void setCountryId(String countryId) {
    this.countryId = countryId;
  }

  public String getMcc() {
    return mcc;
  }

  public void setMcc(String mcc) {
    this.mcc = mcc;
  }

  public String getMnc() {
    return mnc;
  }

  public void setMnc(String mnc) {
    this.mnc = mnc;
  }

}