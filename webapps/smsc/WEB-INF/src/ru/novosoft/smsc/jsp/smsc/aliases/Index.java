package ru.novosoft.smsc.jsp.smsc.aliases;


import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.util.SessionContentManager;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.ListPropertiesHelper;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.IncorrectValueException;
import ru.novosoft.smsc.jsp.util.helper.Validation;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasFilter;

import javax.servlet.http.HttpServletRequest;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * User: artem
 * Date: 21.12.2006
 */
public class Index extends PageBean {

  public static final int RESULT_ADD = PRIVATE_RESULT;
  public static final int RESULT_EDIT = PRIVATE_RESULT + 1;

  protected byte hide = AliasFilter.HIDE_UNKNOWN;

  protected String mbQuery = null;

  protected String mbAdd = null;
  protected String mbDelete = null;
  protected String mbClear = null;

  private Alias selectedAlias = null;

  private boolean initialized = false;

  private AliasesStaticTableHelper tableHelper;
  private ListPropertiesHelper aliases;
  private ListPropertiesHelper addresses;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    return RESULT_OK;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);

    aliases = new ListPropertiesHelper("Aliases", "aliases", 30, Validation.NON_EMPTY, true);
    aliases.setUseBaseRowValue(true);
    aliases.setShowColumnsTitle(false);

    addresses = new ListPropertiesHelper("Addresses", "addresses", 30, Validation.NON_EMPTY, true);
    aliases.setUseBaseRowValue(true);
    addresses.setShowColumnsTitle(false);

    tableHelper = new AliasesStaticTableHelper("aliasesTable", appContext.getSmsc().getAliases(), logger);
    tableHelper.setEditAllowed(isEditAllowed());
    tableHelper.setPageSize(preferences.getAliasesPageSize());
    tableHelper.setMaxTotalSize(preferences.getMaxAliasesTotalSize() + 1);

    result = readStoredContent(request);
    if (result != RESULT_OK)
      return result;

    try {
      this.tableHelper.processRequest(request);
      this.aliases.processRequest(request);
      this.addresses.processRequest(request);

      if (this.tableHelper.eventDataCellSelected() != null) {
        result = processEdit(request, this.tableHelper.eventDataCellSelected().getCellId());

      } else if (mbAdd != null) {
        mbAdd = null;
        result = processAdd(request);

      } else if (mbDelete != null) {
        mbDelete = null;
        result = processDelete(request);

      } else if (mbQuery != null) {
        mbQuery = null;
        result = processQuery();

      } else if (mbClear != null) {
        mbClear = null;
        result = processClear();
      }

      if (initialized) {
        this.tableHelper.setFilter(createFilter());
        this.tableHelper.fillTable(request);
        if (this.tableHelper.getTotalSize() >= preferences.getMaxAliasesTotalSize())
          return _error(new SMSCJspException("Results size is more than " + preferences.getMaxAliasesTotalSize() + ". Show first " + String.valueOf(preferences.getMaxAliasesTotalSize() + 1) + " results.", SMSCJspException.ERROR_CLASS_WARNING));
      }

    } catch (TableHelperException e) {
      logger.error("Can't process request", e);
      return _error(new SMSCJspException("Can't create table", SMSCJspException.ERROR_CLASS_ERROR, e));
    } catch (AdminException e) {
      logger.error("Can't process request", e);
      return _error(new SMSCJspException("Can't create table", SMSCJspException.ERROR_CLASS_ERROR, e));
    } catch (IncorrectValueException e) {
      logger.error("Incorrect filter data", e);
      return _error(new SMSCJspException("Incorrect filter data", SMSCJspException.ERROR_CLASS_ERROR, e));
    }

    return result;
  }

  private AliasFilter createFilter() throws AdminException {
    final AliasFilter filter = new AliasFilter();
    filter.setAddresses(removeEmptyStrings(addresses.getPropsAsArray()));
    filter.setAliases(removeEmptyStrings(aliases.getPropsAsArray()));
    filter.setHide(hide);
    return filter;
  }

  private String[] removeEmptyStrings(String[] strings) {
    final List res = new ArrayList();
    for (int i = 0; i < strings.length; i++) {
      if (strings[i]!= null && !strings[i].equals(""))
        res.add(strings[i]);
    }

    String[] result = new String[res.size()];
    for (int i = 0; i < res.size(); i++)
      result[i] = (String) res.get(i);

    return result;
  }

  private int readStoredContent(HttpServletRequest request) {
    final TemporaryContent content = (TemporaryContent)SessionContentManager.getContentFromSession(request, SessionContentManager.SessionItemId.BEAN_TEMPORARY_CONTENT, this.getClass());

    if (content != null) {
      aliases.setProps(content.getAliases());
      addresses.setProps(content.getAddresses());
      hide = content.getHide();
      tableHelper.setStartPosition(content.getStartPosition());
      initialized = content.isInitialized();

      SessionContentManager.removeContentFromSession(request, SessionContentManager.SessionItemId.BEAN_TEMPORARY_CONTENT, this.getClass());
    }

    return RESULT_OK;
  }

  private void storeContent(HttpServletRequest request) {
    final TemporaryContent content = new TemporaryContent(tableHelper.getStartPosition(), aliases.getPropsAsArray(), addresses.getPropsAsArray(), hide, initialized);
    SessionContentManager.putContentIntoSession(request, SessionContentManager.SessionItemId.BEAN_TEMPORARY_CONTENT, this.getClass(), content);
  }


  private int processAdd(HttpServletRequest request) {
    storeContent(request);
    return RESULT_ADD;
  }

  private int processDelete(HttpServletRequest request) {
    for (Iterator iter = tableHelper.getSelectedAliasesList(request).iterator(); iter.hasNext(); ) {
      final String aliasName = (String)iter.next();
      appContext.getSmsc().getAliases().remove(aliasName);
      journalAppend(SubjectTypes.TYPE_alias, aliasName, Actions.ACTION_DEL);
      appContext.getStatuses().setAliasesChanged(true);
    }
    return processQuery();
  }

  private int processClear() {
    initialized = false;
    aliases.clear();
    addresses.clear();
    hide = AliasFilter.HIDE_UNKNOWN;
    return RESULT_OK;
  }

  private int processQuery() {
    initialized = true;
    tableHelper.setStartPosition(0);
    return RESULT_OK;
  }

  private int processEdit(HttpServletRequest request, String cellId) {
    try {
      selectedAlias = appContext.getSmsc().getAliases().get(new Mask(cellId).getMask());
      storeContent(request);
      return (selectedAlias != null ) ? RESULT_EDIT : _error(new SMSCJspException("Can't load alias", SMSCJspException.ERROR_CLASS_ERROR));
    } catch (AdminException e) {
      return _error(new SMSCJspException("Can't load alias", SMSCJspException.ERROR_CLASS_ERROR, e));
    }
  }

  public boolean isEditAllowed() {
    try {
      return hostsManager.getServiceInfo(Constants.SMSC_SME_ID).isOnline();
    } catch (AdminException e) {
      logger.debug("Couldn't get SMSC service status", e);
    }
    return false;
  }

  public Alias getSelectedAlias() {
    return selectedAlias;
  }


// Properties ==========================================================================================================


  public String getMbDelete() {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete) {
    this.mbDelete = mbDelete;
  }

  public String getMbAdd() {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd) {
    this.mbAdd = mbAdd;
  }

  public byte getHideByte() {
    return hide;
  }

  public String getHide() {
    return String.valueOf(hide);
  }

  public void setHide(String hide) {
    try {
      this.hide = Byte.decode(hide).byteValue();
    } catch (NumberFormatException e) {
      this.hide = AliasFilter.HIDE_UNKNOWN;
    }
  }

  public String getMbQuery() {
    return mbQuery;
  }

  public void setMbQuery(String mbApply) {
    this.mbQuery = mbApply;
  }

  public boolean isInitialized() {
    return initialized;
  }

  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  protected String getBeanId() {
    return "ru.novosoft.smsc.jsp.smsc.aliases.Index";
  }

  public String getMbClear() {
    return mbClear;
  }

  public void setMbClear(String mbClear) {
    this.mbClear = mbClear;
  }

  public AliasesStaticTableHelper getTableHelper() {
    return tableHelper;
  }

  public ListPropertiesHelper getAliases() {
    return aliases;
  }

  public ListPropertiesHelper getAddresses() {
    return addresses;
  }


//  Temporary content ==================================================================================================


  private class TemporaryContent {
    private final int startPosition;
    private final String[] aliases;
    private final String[] addresses;
    private final byte hide;
    private final boolean initialized;

    public TemporaryContent(int startPosition, String[] aliases, String[] addresses, byte hide, boolean initialized) {
      this.startPosition = startPosition;
      this.aliases = aliases;
      this.addresses = addresses;
      this.hide = hide;
      this.initialized = initialized;
    }

    public int getStartPosition() {
      return startPosition;
    }

    public String[] getAliases() {
      return aliases;
    }

    public String[] getAddresses() {
      return addresses;
    }

    public boolean isInitialized() {
      return initialized;
    }

    public byte getHide() {
      return hide;
    }
  }
}


