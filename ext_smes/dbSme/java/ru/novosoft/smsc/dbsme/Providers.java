package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.jsp.util.tables.*;
import ru.novosoft.smsc.jsp.util.tables.impl.*;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 11, 2003
 * Time: 1:54:30 PM
 */
public class Providers extends DbsmeBean
{
  public static final int RESULT_EDIT = DbsmeBean.PRIVATE_RESULT;
  public static final int RESULT_ADD = DbsmeBean.PRIVATE_RESULT + 1;
  public static final int PRIVATE_RESULT = DbsmeBean.PRIVATE_RESULT + 2;

  public static final String prefix = "DBSme.DataProviders";
  public static final int prefix_length = prefix.length();

  private String mbEdit = null;

  private String providerName = null;
  private QueryResultSet providers = new EmptyResultSet();
  private String[] checked = new String[0];
  private Set checkedSet = new HashSet();
  private String mbAdd = null;
  private String mbDelete = null;
  private String mbEnable = null;
  private String mbDisable = null;

  private class _DataItem extends AbstractDataItem
  {
    protected _DataItem(String name, String address, boolean enabled)
    {
      values.put("name", name);
      values.put("address", address);
      values.put("enabled", new Boolean(enabled));
    }
  }

  private class _DataSource extends AbstractDataSourceImpl
  {

    public _DataSource()
    {
      super(new String[]{"name", "address", "enabled"});
    }

    public QueryResultSet query(_Query query_to_run)
    {
      clear();
      for (Iterator i = getSectionChildSectionNames(prefix).iterator(); i.hasNext();) {
        String fullProviderName = (String) i.next();
        add(new _DataItem(StringEncoderDecoder.decodeDot(fullProviderName.substring(prefix_length + 1)),
                          getOptionalString(fullProviderName + ".address"),
                          getOptionalBool(fullProviderName + ".enabled")));
      }
      return super.query(query_to_run);
    }
  }

  private class _Query extends AbstractQueryImpl
  {
    public _Query(int expectedResultsQuantity, Filter filter, Vector sortOrder, int startPosition)
    {
      super(expectedResultsQuantity, filter, sortOrder, startPosition);
    }
  }

  private class _Filter implements Filter
  {
    public boolean isEmpty()
    {
      return true;
    }

    public boolean isItemAllowed(DataItem item)
    {
      return true;
    }
  }


  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    pageSize = 20;
    if (sort != null) {
      //preferences.getAliasesSortOrder().set(0, sort);
    } else {
      //sort = (String) preferences.getAliasesSortOrder().get(0);
      sort = "name";
    }
    checkedSet.addAll(Arrays.asList(checked));
    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbEdit != null && mbEdit.length() > 0 && providerName != null && providerName.length() > 0)
      return RESULT_EDIT;
    if (mbAdd != null)
      return RESULT_ADD;
    if (mbDelete != null) {
      result = delete();
      if (result == RESULT_DONE)
        return result;
    }
    if (mbEnable != null) {
      result = setEnabled(true);
      if (result == RESULT_DONE)
        return result;
    }
    if (mbDisable != null) {
      result = setEnabled(false);
      if (result == RESULT_DONE)
        return result;
    }

    Vector sortVector = new Vector();
    sortVector.add(sort);
    providers = new _DataSource().query(new _Query(pageSize, new _Filter(), sortVector, startPosition));
    totalSize = providers.getTotalSize();


    return result;
  }

  private int setEnabled(boolean enable)
  {
    int result = RESULT_DONE;
    logger.debug("setEnabled(" + enable + ")");
    for (int i = 0; i < checked.length; i++) {
      String providerId = checked[i];
      int tmpResult = setProviderEnabled(providerId, enable);
      if (tmpResult != RESULT_DONE)
        result = tmpResult;
    }

    try {
      config.save();
    } catch (Exception e) {
      logger.error("Couldn't save temporary config, nested: " + e.getMessage(), e);
      return error(DBSmeErrors.error.couldntSaveTempConfig, e);
    }

    return result;
  }

  private int delete()
  {
    for (int i = 0; i < checked.length; i++)
      config.removeSection("DBSme.DataProviders." + StringEncoderDecoder.encodeDot(checked[i]));

    try {
      config.save();
    } catch (Exception e) {
      logger.error("Couldn't save temporary config, nested: " + e.getMessage(), e);
      return error(DBSmeErrors.error.couldntSaveTempConfig, e);
    }

    getContext().setConfigChanged(true);

    checkedSet.clear();
    checked = new String[0];
    return RESULT_DONE;
  }

  public boolean isDpChecked(String dpName)
  {
    return checkedSet.contains(dpName);
  }

  public String[] getChecked()
  {
    return checked;
  }

  public void setChecked(String[] checked)
  {
    this.checked = checked;
  }

  public QueryResultSet getProviders()
  {
    return providers;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getProviderName()
  {
    return providerName;
  }

  public void setProviderName(String providerName)
  {
    this.providerName = providerName;
  }

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public boolean isConfigChanged()
  {
    return getContext().isConfigChanged();
  }

  public String getMbEnable()
  {
    return mbEnable;
  }

  public void setMbEnable(String mbEnable)
  {
    this.mbEnable = mbEnable;
  }

  public String getMbDisable()
  {
    return mbDisable;
  }

  public void setMbDisable(String mbDisable)
  {
    this.mbDisable = mbDisable;
  }
}
