package ru.novosoft.smsc.admin.smsc_service;

/*
 * Author: igork
 * Date: 27.05.2002
 * Time: 18:59:55
 */

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
import ru.novosoft.smsc.admin.profiler.ProfileDataFile;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.route.SmeStatus;
import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.Type;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.profile.ProfileDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.profile.ProfileQuery;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.WebAppFolders;
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

  private static final String APPLY_RESCHEDULE_ID = "apply_reschedule";

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

  private ProfileDataFile profileDataFile = null;

  private long serviceRefreshTimeStamp = 0;
  private static final char LOGGER_DELIMITER = ',';

  public Smsc(final String smscHost, final int smscPort, final String smscConfFolderString, final NSConnectionPool connectionPool, SMSCAppContext smscAppContext) throws AdminException
  {
    super(new ServiceInfo(Constants.SMSC_SME_ID, smscHost, "", "", true, null, ServiceInfo.STATUS_STOPPED), smscPort);

    try {
      this.configFolder = new File(smscConfFolderString);
      final Document aliasesDoc = Utils.parse(new FileReader(new File(configFolder, "aliases.xml")));
      aliases = new AliasSet(aliasesDoc.getDocumentElement());
      profileDataSource = new ProfileDataSource(connectionPool);
      profileDataFile = new ProfileDataFile();
      profileDataFile.init(getSmscConfig(), getConfigFolder().getAbsolutePath());

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
   // distributionListAdmin = new DistributionListManager(super.getInfo(),smscPort /*,connectionPool*/);
  }

  protected PrintWriter storeAliases(final PrintWriter out)
  {
    Functions.storeConfigHeader(out, "aliases", "AliasRecords.dtd");
    aliases.store(out);
    Functions.storeConfigFooter(out, "aliases");
    return out;
  }

  public synchronized List loadRoutes(final RouteSubjectManager routeSubjectManager)
          throws AdminException
  {
    routeSubjectManager.trace();
    if (ServiceInfo.STATUS_RUNNING != getInfo().getStatus())
      throw new AdminException("SMSC is not running.");

    final Object res = call(SMSC_COMPONENT_ID, LOAD_ROUTES_METHOD_ID, Type.Types[Type.StringListType], new HashMap());

    return res instanceof List ? (List) res : null;
  }

  public synchronized List traceRoute(final String dstAddress, final String srcAddress, final String srcSysId)
          throws AdminException
  {
    if (ServiceInfo.STATUS_RUNNING != getInfo().getStatus())
      throw new AdminException("SMSC is not running.");

    final Map args = new HashMap();
    args.put("dstAddress", dstAddress);
    args.put("srcAddress", srcAddress);
    args.put("srcSysId", srcSysId);
    final Object res = call(SMSC_COMPONENT_ID, TRACE_ROUTE_METHOD_ID, Type.Types[Type.StringListType], args);

    return res instanceof List ? (List) res : null;
  }

  public synchronized void applyRoutes(final RouteSubjectManager routeSubjectManager) throws AdminException
  {
    routeSubjectManager.apply();
    if (ServiceInfo.STATUS_RUNNING == getInfo().getStatus()) {
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

      if (ServiceInfo.STATUS_RUNNING == getInfo().getStatus()) {
        call(SMSC_COMPONENT_ID, APPLY_ALIASES_METHOD_ID, Type.Types[Type.StringType], new HashMap());
      }
      else
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

  public synchronized Profile profileLookup(final Mask mask) throws AdminException
  {
    final Map args = new HashMap();
    args.put("address", mask.getMask());
    final Object result = call(SMSC_COMPONENT_ID, PROFILE_LOOKUP_METHOD_ID, Type.Types[Type.StringListType], args);
    if (result instanceof List)
      return new Profile(mask, (List) result);
    else
      throw new AdminException("Error in response");
  }

  public synchronized ProfileEx profileLookupEx(final Mask mask) throws AdminException
  {
    final Map args = new HashMap();
    args.put("address", mask.getMask());
    final Object result = call(SMSC_COMPONENT_ID, PROFILE_LOOKUP_EX_METHOD_ID, Type.Types[Type.StringListType], args);
    if (result instanceof List)
      return new ProfileEx(mask, (List) result);
    else
      throw new AdminException("Error in response");
  }

  public synchronized int profileUpdate(final Mask mask, final Profile newProfile) throws AdminException
  {
    final Map args = new HashMap();
    args.put("address", mask.getMask());
    final List profileArg = new LinkedList();
    profileArg.add(newProfile.getCodepageString());
    profileArg.add(newProfile.getReportOptionsString());
    profileArg.add(newProfile.getLocale());
    profileArg.add(newProfile.getAliasHideString());
    profileArg.add(newProfile.isAliasModifiable() ? "true" : "false");
    profileArg.add(newProfile.getDivert());
    profileArg.add(newProfile.getDivertActive());
    profileArg.add(newProfile.isDivertModifiable() ? "true" : "false");
    profileArg.add(newProfile.isUssd7bit() ? "true" : "false");
    profileArg.add(newProfile.isUdhConcat() ? "true" : "false");
    profileArg.add(newProfile.isTranslit() ? "true" : "false");

    args.put("profile", profileArg);
    return ((Long) call(SMSC_COMPONENT_ID, PROFILE_UPDATE_METHOD_ID, Type.Types[Type.IntType], args)).intValue();
  }

  public synchronized void profileDelete(final Mask mask) throws AdminException
  {
    final Map args = new HashMap();
    args.put("address", mask.getMask());
    call(SMSC_COMPONENT_ID, PROFILE_DELETE_METHOD_ID, Type.Types[Type.IntType], args);
    profileDataSource.delete(mask);
  }

  public synchronized QueryResultSet profilesQuery(final ProfileQuery query) throws AdminException
  {
    return profileDataSource.query(query);
  }

 public synchronized QueryResultSet profilesQueryFromFile(final ProfileQuery query) throws AdminException {
     return profileDataFile.query(query);
 }

  public synchronized void applyProfiles()
  {
    // nothing to do
  }

  public synchronized void processCancelMessages(final Collection messageIds) throws AdminException
  {
    final StringBuffer ids = new StringBuffer(messageIds.size() * 10);
    final StringBuffer srcs = new StringBuffer(messageIds.size() * 10);
    final StringBuffer dsts = new StringBuffer(messageIds.size() * 10);
    for (Iterator i = messageIds.iterator(); i.hasNext();) {
      final CancelMessageData data = (CancelMessageData) i.next();
      ids.append(data.getMessageId());
      srcs.append(data.getSourceAddress());
      dsts.append(data.getDestinationAddress());
      if (i.hasNext()) {
        ids.append(',');
        srcs.append(',');
        dsts.append(',');
      }
    }
    final Map params = new HashMap();
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
      final File confFile = new File(WebAppFolders.getSmscConfFolder(), "config.xml");
      return new Config(confFile);
    } catch (Throwable t) {
      logger.error("Couldn't get SMSC config", t);
      return null;
    }
  }

  public synchronized void saveSmscConfig(final Config config) throws AdminException
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
    if (ServiceInfo.STATUS_RUNNING == getInfo().getStatus()) {
      call(SMSC_COMPONENT_ID, APPLY_SMSC_CONFIG_METHOD_ID, Type.Types[Type.StringType], new HashMap());
    }
  }

  public synchronized void mscRegistrate(final String msc) throws AdminException
  {
    final Map args = new HashMap();
    args.put("msc", msc);
    call(SMSC_COMPONENT_ID, MSC_REGISTRATE_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized void mscUnregister(final String msc) throws AdminException
  {
    final Map args = new HashMap();
    args.put("msc", msc);
    call(SMSC_COMPONENT_ID, MSC_UNREGISTER_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized void mscBlock(final String msc) throws AdminException
  {
    final Map args = new HashMap();
    args.put("msc", msc);
    call(SMSC_COMPONENT_ID, MSC_BLOCK_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized void mscClear(final String msc) throws AdminException
  {
    final Map args = new HashMap();
    args.put("msc", msc);
    call(SMSC_COMPONENT_ID, MSC_CLEAR_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized List mscList() throws AdminException
  {
    final Object result = call(SMSC_COMPONENT_ID, MSC_LIST_METHOD_ID, Type.Types[Type.StringListType], new HashMap());
    if (result instanceof List)
      return (List) result;
    else
      throw new AdminException("Error in response");
  }

  public synchronized boolean isLocaleRegistered(final String locale)
  {
    final Config config = getSmscConfig();
    if (null == config)
      return false;
    String localesString;
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
		final List result = new SortedList();
		final Config config = getSmscConfig();
		if (config != null)
		{
			String localesString = config.getString("core.locales");
			final StringTokenizer tokenizer = new StringTokenizer(localesString, ",");
			while (tokenizer.hasMoreTokens())
			{
				result.add(tokenizer.nextToken().trim());
			}
		}
		return result;
	}

  public synchronized DistributionListAdmin getDistributionListAdmin()
  {
    return distributionListAdmin;
  }

  private Map putSmeIntoMap(final SME sme)
  {
    final Map params = new HashMap();
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

  public synchronized void smeAdd(final SME sme) throws AdminException
  {
    final Object result = call(SMSC_COMPONENT_ID, SME_ADD_METHOD_ID, Type.Types[Type.BooleanType], putSmeIntoMap(sme));
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized void smeRemove(final String smeId) throws AdminException
  {
    final Map params = new HashMap();
    params.put("id", smeId);
    final Object result = call(SMSC_COMPONENT_ID, SME_REMOVE_METHOD_ID, Type.Types[Type.BooleanType], params);
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized void smeUpdate(final SME sme) throws AdminException
  {
    final Object result = call(SMSC_COMPONENT_ID, SME_UPDATE_METHOD_ID, Type.Types[Type.BooleanType], putSmeIntoMap(sme));
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized SmeStatus getSmeStatus(final String id) throws AdminException
  {
    final long currentTime = System.currentTimeMillis();
    if (currentTime - Constants.ServicesRefreshTimeoutMillis > serviceRefreshTimeStamp) {
      serviceRefreshTimeStamp = currentTime;
      smeStatuses.clear();
      final Object result = call(SMSC_COMPONENT_ID, SME_STATUS_ID, Type.Types[Type.StringListType], new HashMap());
      if (!(result instanceof List))
        throw new AdminException("Error in response");

      for (Iterator i = ((List) result).iterator(); i.hasNext();) {
        final String s = (String) i.next();
        final SmeStatus smeStatus = new SmeStatus(s);
        smeStatuses.put(smeStatus.getId(), smeStatus);
      }
    }
    return (SmeStatus) smeStatuses.get(id);
  }

  public synchronized void disconnectSmes(final List smeIdsToDisconnect) throws AdminException
  {
    final Map params = new HashMap();
    params.put("ids", smeIdsToDisconnect);
    final Object result = call(SMSC_COMPONENT_ID, SME_DISCONNECT_ID, Type.Types[Type.BooleanType], params);
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized Map getLogCategories() throws AdminException
  {
    final Map return_result = new HashMap();
    final Object resultO = call(SMSC_COMPONENT_ID, LOG_GET_CATEGORIES_ID, Type.Types[Type.StringListType], new HashMap());
    if (resultO instanceof List) {
      final List result = (List) resultO;
      for (Iterator i = result.iterator(); i.hasNext();) {
        final String cat = (String) i.next();
        final int delim_pos = cat.lastIndexOf(LOGGER_DELIMITER);
        if (0 <= delim_pos) {
          final String name = cat.substring(0, delim_pos);
          final String value = cat.substring(delim_pos + 1);
          return_result.put(name, value);
        }
        else
          logger.error("Error in response: string \"" + cat + "\" misformatted.");
      }
    }
    else
      throw new AdminException("Error in response");
    return return_result;
  }

  public synchronized void setLogCategories(final Map cats) throws AdminException
  {
    final Map params = new HashMap();
    final List catsList = new LinkedList();
    params.put("categories", catsList);
    for (Iterator i = cats.entrySet().iterator(); i.hasNext();) {
      final Map.Entry entry = (Map.Entry) i.next();
      final String catName = (String) entry.getKey();
      final String catPriority = (String) entry.getValue();
      catsList.add(catName + LOGGER_DELIMITER + catPriority);
    }
    call(SMSC_COMPONENT_ID, LOG_SET_CATEGORIES_ID, Type.Types[Type.BooleanType], params);
  }

  public synchronized void applyReschedule() throws AdminException
  {
    if (ServiceInfo.STATUS_RUNNING == getInfo().getStatus()) {
      call(SMSC_COMPONENT_ID, APPLY_RESCHEDULE_ID, Type.Types[Type.StringType], new HashMap());
    }
     else {
       logger.debug("Couldn't call apply reschedule method on SMSC - SMSC is not running. Status is " + getInfo().getStatusStr() + " (" + getInfo().getStatus() + ")");
    }
  }

  public synchronized void applyLocaleResources() throws AdminException
  {
    if (ServiceInfo.STATUS_RUNNING == getInfo().getStatus()) {
      call(SMSC_COMPONENT_ID, APPLY_LOCALE_RESOURCES_METHOD_ID, Type.Types[Type.StringType], new HashMap());
    }
     else {
       logger.debug("Couldn't call apply local resources method on SMSC - SMSC is not running. Status is " + getInfo().getStatusStr() + " (" + getInfo().getStatus() + ")");
   }
 }

  public File getConfigFolder()
  {
    return configFolder;
  }


  public List aclListNames() throws AdminException
  {
    final Object result = call(SMSC_COMPONENT_ID, ACL_LIST_NAMES, Type.Types[Type.StringListType], new HashMap());
    if (!(result instanceof List))
      throw new AdminException("Error in response");

    final List list = (List) result;
    final List resultList = new ArrayList();
    for (Iterator i = list.iterator(); i.hasNext();) {
      final String aclPair = (String) i.next();
      final int pos = aclPair.indexOf(',');
      final String idStr = aclPair.substring(0, pos);
      final String name = aclPair.substring(pos + 1);
      try {
        resultList.add(new AclInfo(Long.parseLong(idStr), name));
      } catch (NumberFormatException e) {
        logger.error("Could not parse acl. id:\"" + idStr + "\" name:\"" + name + "\"", e);
      }
    }
    return resultList;
  }

  public AclInfo aclGetInfo(final long aclId) throws AdminException
  {
    final Map params = new HashMap();
    params.put("id", new Long(aclId));
    final Object result = call(SMSC_COMPONENT_ID, ACL_GET_INFO, Type.Types[Type.StringListType], params);
    if (!(result instanceof List))
      throw new AdminException("Error in response");

    return new AclInfo((List) result);
  }

  public void aclRemove(final long aclId) throws AdminException
  {
    final Map params = new HashMap();
    params.put("id", new Long(aclId));
    call(SMSC_COMPONENT_ID, ACL_REMOVE, Type.Types[Type.BooleanType], params);
  }

  public long aclCreate(final String name, final String description, final List addresses, final char cache_type) throws AdminException
  {
    final Map params = new HashMap();
    params.put("name", name);
    params.put("description", description);
    params.put("cache_type", String.valueOf(cache_type));
    params.put("addresses", addresses);
    final Object result = call(SMSC_COMPONENT_ID, ACL_CREATE, Type.Types[Type.IntType], params);
    if (result instanceof Long) {
      final Long aclId = (Long) result;
      return aclId.longValue();
    }
    else
      throw new AdminException("Error in response");
  }

  public void aclUpdateInfo(final long aclId, final String name, final String description, final char cache_type) throws AdminException
  {
    final Map params = new HashMap();
    params.put("id", new Long(aclId));
    params.put("name", name);
    params.put("description", description);
    params.put("cache_type", String.valueOf(cache_type));
    call(SMSC_COMPONENT_ID, ACL_UPDATE_INFO, Type.Types[Type.BooleanType], params);
  }

  public List aclLookupAddresses(final long aclId, final String addressPrefix) throws AdminException
  {
    final Map params = new HashMap();
    params.put("id", new Long(aclId));
    params.put("prefix", addressPrefix);
    final Object result = call(SMSC_COMPONENT_ID, ACL_LOOKUP_ADDRESSES, Type.Types[Type.StringListType], params);
    if (!(result instanceof List))
      throw new AdminException("Error in response");

    return (List) result;
  }

  public void aclRemoveAddresses(final long aclId, final List addresses) throws AdminException
  {
    final Map params = new HashMap();
    params.put("id", new Long(aclId));
    params.put("addresses", addresses);
    call(SMSC_COMPONENT_ID, ACL_REMOVE_ADDRESSES, Type.Types[Type.BooleanType], params);
  }

  public void aclAddAddresses(final long aclId, final List addresses) throws AdminException
  {
    final Map params = new HashMap();
    params.put("id", new Long(aclId));
    params.put("addresses", addresses);
    call(SMSC_COMPONENT_ID, ACL_ADD_ADDRESSES, Type.Types[Type.BooleanType], params);
  }
}
