/*
 * Author: igork
 * Date: 27.05.2002
 * Time: 18:59:55
 */
package ru.novosoft.smsc.admin.smsc_service;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.acl.AclInfo;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.DistributionListManager;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.profiler.ProfileEx;
import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.profile.ProfileDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.profile.ProfileQuery;
import ru.novosoft.smsc.util.*;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.xml.Utils;
import ru.novosoft.util.conpool.NSConnectionPool;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;


public class Smsc extends Service
{
  private static final String SMSC_COMPONENT_ID = "SMSC";

  private static final String APPLY_ALIASES_METHOD_ID = "apply_aliases";

  private static final String APPLY_ROUTES_METHOD_ID = "apply_routes";
  private static final String LOAD_ROUTES_METHOD_ID = "load_routes";
  private static final String TRACE_ROUTE_METHOD_ID = "trace_route";

  private static final String PROFILE_LOOKUP_METHOD_ID = "lookup_profile";
  private static final String PROFILE_LOOKUP_EX_METHOD_ID = "profile_lookup_ex";
  private static final String PROFILE_UPDATE_METHOD_ID = "update_profile";
  private static final String PROFILE_DELETE_METHOD_ID = "profile_delete";

  private static final String FLUSH_STATISTICS_METHOD_ID = "flush_statistics";
  private static final String PROCESS_CANCEL_MESSAGES_METHOD_ID = "process_cancel_messages";
  private static final String APPLY_SMSC_CONFIG_METHOD_ID = "apply_smsc_config";
//  private static final String APPLY_SERVICES_METHOD_ID = "apply_services";
  private static final String APPLY_LOCALE_RESOURCES_METHOD_ID = "apply_locale_resources";

  private static final String MSC_REGISTRATE_METHOD_ID = "msc_registrate";
  private static final String MSC_UNREGISTER_METHOD_ID = "msc_unregister";
  private static final String MSC_BLOCK_METHOD_ID = "msc_block";
  private static final String MSC_CLEAR_METHOD_ID = "msc_clear";
  private static final String MSC_LIST_METHOD_ID = "msc_list";

  private static final String SME_ADD_METHOD_ID = "sme_add";
  private static final String SME_REMOVE_METHOD_ID = "sme_remove";
  private static final String SME_UPDATE_METHOD_ID = "sme_update";
  private static final String SME_STATUS_ID = "sme_status";
  private static final String SME_DISCONNECT_ID = "sme_disconnect";

  private static final String LOG_GET_CATEGORIES_ID = "log_get_categories";
  private static final String LOG_SET_CATEGORIES_ID = "log_set_categories";

  private static final String ACL_LIST_NAMES = "acl_list_names";
  private static final String ACL_GET_INFO = "acl_get";
  private static final String ACL_REMOVE = "acl_remove";
  private static final String ACL_CREATE = "acl_create";
  private static final String ACL_UPDATE_INFO = "acl_update_info";
  private static final String ACL_LOOKUP_ADDRESSES = "acl_lookup_addresses";
  private static final String ACL_REMOVE_ADDRESSES = "acl_remove_addresses";
  private static final String ACL_ADD_ADDRESSES = "acl_add_addresses";


  private File configFolder = null;
  private Map smeStatuses = new HashMap();
  private DistributionListAdmin distributionListAdmin = null;

  private AliasSet aliases = null;
  private ProfileDataSource profileDataSource = null;

  private Category logger = Category.getInstance(this.getClass());
  private long serviceRefreshTimeStamp = 0;
  private static final char LOGGER_DELIMITER = ',';

  public Smsc(String smscHost, int smscPort, String smscConfFolderString, NSConnectionPool connectionPool) throws AdminException
  {
    super(new ServiceInfo(Constants.SMSC_SME_ID, smscHost, smscPort, "", "", true, null, ServiceInfo.STATUS_STOPPED));

    try {
      this.configFolder = new File(smscConfFolderString);
      Document aliasesDoc = Utils.parse(new FileReader(new File(configFolder, "aliases.xml")));
      aliases = new AliasSet(aliasesDoc.getDocumentElement());
      profileDataSource = new ProfileDataSource(connectionPool);
    } catch (FactoryConfigurationError error) {
      logger.error("Couldn't configure xml parser factory", error);
      throw new AdminException("Couldn't configure xml parser factory: " + error.getMessage());
    } catch (ParserConfigurationException e) {
      logger.error("Couldn't configure xml parser", e);
      throw new AdminException("Couldn't configure xml parser: " + e.getMessage());
    } catch (SAXException e) {
      logger.error("Couldn't parse", e);
      throw new AdminException("Couldn't parse: " + e.getMessage());
    } catch (IOException e) {
      logger.error("Couldn't read", e);
      throw new AdminException("Couldn't read: " + e.getMessage());
    } catch (NullPointerException e) {
      logger.error("Couldn't parse", e);
      throw new AdminException("Couldn't parse: " + e.getMessage());
    }

    distributionListAdmin = new DistributionListManager(connectionPool);
  }

  protected PrintWriter storeAliases(PrintWriter out)
  {
    Functions.storeConfigHeader(out, "aliases", "AliasRecords.dtd");
    aliases.store(out);
    Functions.storeConfigFooter(out, "aliases");
    return out;
  }

  public synchronized List loadRoutes(RouteSubjectManager routeSubjectManager)
      throws AdminException
  {
    routeSubjectManager.trace();
    if (getInfo().getStatus() != ServiceInfo.STATUS_RUNNING)
      throw new AdminException("SMSC is not running.");

    Object res = call(SMSC_COMPONENT_ID, LOAD_ROUTES_METHOD_ID, Type.Types[Type.StringListType], new HashMap());

    return ((res instanceof List) ? (List) res : null);
  }

  public synchronized List traceRoute(String dstAddress, String srcAddress, String srcSysId)
      throws AdminException
  {
    if (getInfo().getStatus() != ServiceInfo.STATUS_RUNNING)
      throw new AdminException("SMSC is not running.");

    HashMap args = new HashMap();
    args.put("dstAddress", dstAddress);
    args.put("srcAddress", srcAddress);
    args.put("srcSysId", srcSysId);
    Object res = call(SMSC_COMPONENT_ID, TRACE_ROUTE_METHOD_ID, Type.Types[Type.StringListType], args);

    return ((res instanceof List) ? (List) res : null);
  }

  public synchronized void applyRoutes(RouteSubjectManager routeSubjectManager) throws AdminException
  {
    routeSubjectManager.apply();
    if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING) {
      call(SMSC_COMPONENT_ID, APPLY_ROUTES_METHOD_ID, Type.Types[Type.StringType], new HashMap());
    }
  }

  public synchronized void applyAliases() throws AdminException
  {
    try {
      final File smscConfFolder = WebAppFolders.getSmscConfFolder();

      final File aliasConfigFile = new File(smscConfFolder, "aliases.xml");
      final File newFile = Functions.createNewFilenameForSave(aliasConfigFile);
      storeAliases(new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()))).close();
      Functions.renameNewSavedFileToOriginal(newFile, aliasConfigFile);

      if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING) {
        call(SMSC_COMPONENT_ID, APPLY_ALIASES_METHOD_ID, Type.Types[Type.StringType], new HashMap());
      } else
        logger.debug("Couldn't call apply method on SMSC - SMSC is not running. Status is " + getInfo().getStatusStr() + " (" + getInfo().getStatus() + ")");
    } catch (FileNotFoundException e) {
      throw new AdminException("Couldn't apply_routes new settings: Couldn't write to destination config file: " + e.getMessage());
    } catch (IOException e) {
      throw new AdminException("Couldn't apply_routes new settings: " + e.getMessage());
    }
  }

  public synchronized AliasSet getAliases()
  {
    return aliases;
  }

  public synchronized Profile profileLookup(Mask mask) throws AdminException
  {
    HashMap args = new HashMap();
    args.put("address", mask.getMask());
    Object result = call(SMSC_COMPONENT_ID, PROFILE_LOOKUP_METHOD_ID, Type.Types[Type.StringListType], args);
    if (result instanceof List)
      return new Profile(mask, (List) result);
    else
      throw new AdminException("Error in response");
  }

  public synchronized ProfileEx profileLookupEx(Mask mask) throws AdminException
  {
    HashMap args = new HashMap();
    args.put("address", mask.getMask());
    Object result = call(SMSC_COMPONENT_ID, PROFILE_LOOKUP_EX_METHOD_ID, Type.Types[Type.StringListType], args);
    if (result instanceof List)
      return new ProfileEx(mask, (List) result);
    else
      throw new AdminException("Error in response");
  }

  public synchronized int profileUpdate(Mask mask, Profile newProfile) throws AdminException
  {
    HashMap args = new HashMap();
    args.put("address", mask.getMask());
    List profileArg = new LinkedList();
    profileArg.add(newProfile.getCodepageString());
    profileArg.add(newProfile.getReportOptionsString());
    profileArg.add(newProfile.getLocale());
    profileArg.add(newProfile.isAliasHide() ? "true" : "false");
    profileArg.add(newProfile.isAliasModifiable() ? "true" : "false");
    profileArg.add(newProfile.getDivert());
    profileArg.add(newProfile.getDivertActive());
    profileArg.add(newProfile.isDivertModifiable() ? "true" : "false");
    profileArg.add(newProfile.isUssd7bit() ? "true" : "false");
    profileArg.add(newProfile.isUdhConcat() ? "true" : "false");

    args.put("profile", profileArg);
    return ((Long) call(SMSC_COMPONENT_ID, PROFILE_UPDATE_METHOD_ID, Type.Types[Type.IntType], args)).intValue();
  }

  public synchronized void profileDelete(Mask mask) throws AdminException
  {
    HashMap args = new HashMap();
    args.put("address", mask.getMask());
    call(SMSC_COMPONENT_ID, PROFILE_DELETE_METHOD_ID, Type.Types[Type.IntType], args);
    profileDataSource.delete(mask);
  }

  public synchronized QueryResultSet profilesQuery(ProfileQuery query) throws AdminException
  {
    return profileDataSource.query(query);
  }

  public synchronized void applyProfiles()
  {
    // nothing to do
  }

  public synchronized void processCancelMessages(Collection messageIds) throws AdminException
  {
    StringBuffer ids = new StringBuffer(messageIds.size() * 10);
    StringBuffer srcs = new StringBuffer(messageIds.size() * 10);
    StringBuffer dsts = new StringBuffer(messageIds.size() * 10);
    for (Iterator i = messageIds.iterator(); i.hasNext();) {
      CancelMessageData data = (CancelMessageData) i.next();
      ids.append(data.getMessageId());
      srcs.append(data.getSourceAddress());
      dsts.append(data.getDestinationAddress());
      if (i.hasNext()) {
        ids.append(',');
        srcs.append(',');
        dsts.append(',');
      }
    }
    Map params = new HashMap();
    params.put("ids", ids.toString());
    params.put("sources", srcs.toString());
    params.put("destinations", dsts.toString());
    call(SMSC_COMPONENT_ID, PROCESS_CANCEL_MESSAGES_METHOD_ID, Type.Types[Type.StringType], params);
  }

  public synchronized void flushStatistics() throws AdminException
  {
    call(SMSC_COMPONENT_ID, FLUSH_STATISTICS_METHOD_ID, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized Config getSmscConfig()
  {
    try {
      File confFile = new File(WebAppFolders.getSmscConfFolder(), "config.xml");
      return new Config(confFile);
    } catch (Throwable t) {
      logger.error("Couldn't get SMSC config", t);
      return null;
    }
  }

  public synchronized void saveSmscConfig(Config config) throws AdminException
  {
    try {
      config.save();
    } catch (Throwable t) {
      logger.error("Couldn't store SMSC config", t);
      throw new AdminException("Couldn't store SMSC config: " + t.getMessage());
    }
  }

  public synchronized void applyConfig() throws AdminException
  {
    if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING) {
      call(SMSC_COMPONENT_ID, APPLY_SMSC_CONFIG_METHOD_ID, Type.Types[Type.StringType], new HashMap());
    }
  }

  public synchronized void mscRegistrate(String msc) throws AdminException
  {
    HashMap args = new HashMap();
    args.put("msc", msc);
    call(SMSC_COMPONENT_ID, MSC_REGISTRATE_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized void mscUnregister(String msc) throws AdminException
  {
    HashMap args = new HashMap();
    args.put("msc", msc);
    call(SMSC_COMPONENT_ID, MSC_UNREGISTER_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized void mscBlock(String msc) throws AdminException
  {
    HashMap args = new HashMap();
    args.put("msc", msc);
    call(SMSC_COMPONENT_ID, MSC_BLOCK_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized void mscClear(String msc) throws AdminException
  {
    HashMap args = new HashMap();
    args.put("msc", msc);
    call(SMSC_COMPONENT_ID, MSC_CLEAR_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized List mscList() throws AdminException
  {
    Object result = call(SMSC_COMPONENT_ID, MSC_LIST_METHOD_ID, Type.Types[Type.StringListType], new HashMap());
    if (result instanceof List)
      return (List) result;
    else
      throw new AdminException("Error in response");
  }

  public synchronized boolean isLocaleRegistered(String locale)
  {
    Config config = getSmscConfig();
    if (config == null)
      return false;
    String localesString = null;
    try {
      localesString = config.getString("core.locales");
    } catch (Config.ParamNotFoundException e) {
      logger.warn("isLocaleRegistered: Parameter core.locales not found");
      return false;
    } catch (Config.WrongParamTypeException e) {
      logger.warn("isLocaleRegistered: Parameter core.locales is not string");
      return false;
    }
    return localesString.matches(".*\\b" + locale + "\\b.*");
  }

  public synchronized List getRegisteredLocales() throws Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    List result = new SortedList();
    Config config = getSmscConfig();
    String localesString = null;
    localesString = config.getString("core.locales");
    StringTokenizer tokenizer = new StringTokenizer(localesString, ",");
    while (tokenizer.hasMoreTokens()) {
      result.add(tokenizer.nextToken().trim());
    }
    return result;
  }

  public synchronized DistributionListAdmin getDistributionListAdmin()
  {
    return distributionListAdmin;
  }

  private Map putSmeIntoMap(SME sme)
  {
    Map params = new HashMap();
    params.put("id", sme.getId());
    params.put("priority", new Integer(sme.getPriority()));
    params.put("typeOfNumber", new Integer(sme.getTypeOfNumber()));
    params.put("numberingPlan", new Integer(sme.getNumberingPlan()));
    params.put("interfaceVersion", new Integer(sme.getInterfaceVersion()));
    params.put("systemType", sme.getSystemType());
    params.put("password", sme.getPassword());
    params.put("addrRange", sme.getAddrRange());
    params.put("smeN", new Integer(sme.getSmeN()));
    params.put("wantAlias", new Boolean(sme.isWantAlias()));
    params.put("forceDC", new Boolean(sme.isForceDC()));
    params.put("timeout", new Integer(sme.getTimeout()));
    params.put("receiptSchemeName", sme.getReceiptSchemeName());
    params.put("disabled", new Boolean(sme.isDisabled()));
    params.put("mode", sme.getModeStr());
    params.put("proclimit", new Integer(sme.getProclimit()));
    params.put("schedlimit", new Integer(sme.getSchedlimit()));
    return params;
  }

  public synchronized void smeAdd(SME sme) throws AdminException
  {
    Object result = call(SMSC_COMPONENT_ID, SME_ADD_METHOD_ID, Type.Types[Type.BooleanType], putSmeIntoMap(sme));
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized void smeRemove(String smeId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", smeId);
    Object result = call(SMSC_COMPONENT_ID, SME_REMOVE_METHOD_ID, Type.Types[Type.BooleanType], params);
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized void smeUpdate(SME sme) throws AdminException
  {
    Object result = call(SMSC_COMPONENT_ID, SME_UPDATE_METHOD_ID, Type.Types[Type.BooleanType], putSmeIntoMap(sme));
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized SmeStatus getSmeStatus(String id) throws AdminException
  {
    final long currentTime = System.currentTimeMillis();
    if (currentTime - Constants.ServicesRefreshTimeoutMillis > serviceRefreshTimeStamp) {
      serviceRefreshTimeStamp = currentTime;
      smeStatuses.clear();
      Object result = call(SMSC_COMPONENT_ID, SME_STATUS_ID, Type.Types[Type.StringListType], new HashMap());
      if (!(result instanceof List))
        throw new AdminException("Error in response");

      for (Iterator i = ((List) result).iterator(); i.hasNext();) {
        String s = (String) i.next();
        SmeStatus smeStatus = new SmeStatus(s);
        smeStatuses.put(smeStatus.getId(), smeStatus);
      }
    }
    return (SmeStatus) smeStatuses.get(id);
  }

  public synchronized void disconnectSmes(List smeIdsToDisconnect) throws AdminException
  {
    Map params = new HashMap();
    params.put("ids", smeIdsToDisconnect);
    Object result = call(SMSC_COMPONENT_ID, SME_DISCONNECT_ID, Type.Types[Type.BooleanType], params);
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized Map getLogCategories() throws AdminException
  {
    Map return_result = new HashMap();
    Object resultO = call(SMSC_COMPONENT_ID, LOG_GET_CATEGORIES_ID, Type.Types[Type.StringListType], new HashMap());
    if (resultO instanceof List) {
      List result = (List) resultO;
      for (Iterator i = result.iterator(); i.hasNext();) {
        String cat = (String) i.next();
        final int delim_pos = cat.lastIndexOf(LOGGER_DELIMITER);
        if (delim_pos >= 0) {
          String name = cat.substring(0, delim_pos);
          String value = cat.substring(delim_pos + 1);
          return_result.put(name, value);
        } else
          logger.error("Error in response: string \"" + cat + "\" misformatted.");
      }
    } else
      throw new AdminException("Error in response");
    return return_result;
  }

  public synchronized void setLogCategories(Map cats) throws AdminException
  {
    Map params = new HashMap();
    LinkedList catsList = new LinkedList();
    params.put("categories", catsList);
    for (Iterator i = cats.entrySet().iterator(); i.hasNext();) {
      Map.Entry entry = (Map.Entry) i.next();
      final String catName = (String) entry.getKey();
      final String catPriority = (String) entry.getValue();
      catsList.add(catName + LOGGER_DELIMITER + catPriority);
    }
    call(SMSC_COMPONENT_ID, LOG_SET_CATEGORIES_ID, Type.Types[Type.BooleanType], params);
  }

  public synchronized void applyLocaleResources() throws AdminException
  {
    if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING) {
      call(SMSC_COMPONENT_ID, APPLY_LOCALE_RESOURCES_METHOD_ID, Type.Types[Type.StringType], new HashMap());
    }
  }

  public File getConfigFolder()
  {
    return configFolder;
  }


  public List aclListNames() throws AdminException
  {
    Object result = call(SMSC_COMPONENT_ID, ACL_LIST_NAMES, Type.Types[Type.StringListType], new HashMap());
    if (!(result instanceof List))
      throw new AdminException("Error in response");

    List list = (List) result;
    List resultList = new ArrayList();
    for (Iterator i = list.iterator(); i.hasNext();) {
      String aclPair = (String) i.next();
      int pos = aclPair.indexOf(',');
      String idStr = aclPair.substring(0, pos);
      String name = aclPair.substring(pos + 1);
      try {
        resultList.add(new AclInfo(Long.parseLong(idStr), name));
      } catch (NumberFormatException e) {
        logger.error("Could not parse acl. id:\"" + idStr + "\" name:\"" + name + "\"", e);
      }
    }
    return resultList;
  }

  public AclInfo aclGetInfo(long aclId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", new Long(aclId));
    Object result = call(SMSC_COMPONENT_ID, ACL_GET_INFO, Type.Types[Type.StringListType], params);
    if (!(result instanceof List))
      throw new AdminException("Error in response");

    return new AclInfo((List) result);
  }

  public void aclRemove(long aclId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", new Long(aclId));
    call(SMSC_COMPONENT_ID, ACL_REMOVE, Type.Types[Type.BooleanType], params);
  }

  public void aclCreate(String name, String description, List addresses, char cache_type) throws AdminException
  {
    Map params = new HashMap();
    params.put("name", name);
    params.put("description", description);
    params.put("cache_type", String.valueOf(cache_type));
    params.put("addresses", addresses);
    call(SMSC_COMPONENT_ID, ACL_CREATE, Type.Types[Type.BooleanType], params);
  }

  public void aclUpdateInfo(long aclId, String name, String description, char cache_type) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", new Long(aclId));
    params.put("name", name);
    params.put("description", description);
    params.put("cache_type", String.valueOf(cache_type));
    call(SMSC_COMPONENT_ID, ACL_UPDATE_INFO, Type.Types[Type.BooleanType], params);
  }

  public List aclLookupAddresses(long aclId, String addressPrefix) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", new Long(aclId));
    params.put("prefix", addressPrefix);
    Object result = call(SMSC_COMPONENT_ID, ACL_LOOKUP_ADDRESSES, Type.Types[Type.StringListType], params);
    if (!(result instanceof List))
      throw new AdminException("Error in response");

    return (List) result;
  }

  public void aclRemoveAddresses(long aclId, List addresses) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", new Long(aclId));
    params.put("addresses", addresses);
    call(SMSC_COMPONENT_ID, ACL_REMOVE_ADDRESSES, Type.Types[Type.BooleanType], params);
  }

  public void aclAddAddresses(long aclId, List addresses) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", new Long(aclId));
    params.put("addresses", addresses);
    call(SMSC_COMPONENT_ID, ACL_ADD_ADDRESSES, Type.Types[Type.BooleanType], params);
  }
}
