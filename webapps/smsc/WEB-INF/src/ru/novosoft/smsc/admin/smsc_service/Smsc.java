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
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.DistributionListManager;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.profiler.ProfileEx;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.route.SmeStatus;
import ru.novosoft.smsc.admin.service.*;
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
  private File configFolder = null;
  private Component smsc_component = null;
  private Method apply_routes_method = null;
  private Method apply_aliases_method = null;
  private Method load_routes_method = null;
  private Method trace_route_method = null;
  private Method profile_lookup_method = null;
  private Method profile_lookup_ex_method = null;
  private Method profile_update_method = null;
  private Method profile_delete_method = null;
  private Method flush_statistics_method = null;
  private Method process_cancel_messages_method = null;
  private Method apply_smsc_config_method = null;
  private Method apply_services_method = null;
  private Method apply_locale_resources_method = null;

  private Method msc_registrate_method = null;
  private Method msc_unregister_method = null;
  private Method msc_block_method = null;
  private Method msc_clear_method = null;
  private Method msc_list_method = null;
  private Method sme_add_method = null;
  private Method sme_remove_method = null;
  private Method sme_update_method = null;
  private Method sme_status = null;
  private Method sme_disconnect = null;
  private Method log_get_categories = null;
  private Method log_set_categories = null;

  private Map smeStatuses = new HashMap();


  private DistributionListAdmin distributionListAdmin = null;

  private AliasSet aliases = null;
  private ProfileDataSource profileDataSource = null;

  private Category logger = Category.getInstance(this.getClass());
  private long serviceRefreshTimeStamp = 0;
  private static final char LOGGER_DELIMITER = ',';

  public Smsc(String smscHost, int smscPort, String smscConfFolderString, NSConnectionPool connectionPool) throws AdminException, Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    super(new ServiceInfo(Constants.SMSC_SME_ID, smscHost, smscPort, "", "", null, ServiceInfo.STATUS_STOPPED));

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

    refreshComponents();
    Object res = call(smsc_component, load_routes_method, Type.Types[Type.StringListType], new HashMap());

    return ((res instanceof List) ? (List) res : null);
  }

  public synchronized List traceRoute(String dstAddress, String srcAddress, String srcSysId)
          throws AdminException
  {
    if (getInfo().getStatus() != ServiceInfo.STATUS_RUNNING)
      throw new AdminException("SMSC is not running.");

    refreshComponents();
    HashMap args = new HashMap();
    args.put("dstAddress", dstAddress);
    args.put("srcAddress", srcAddress);
    args.put("srcSysId", srcSysId);
    Object res = call(smsc_component, trace_route_method, Type.Types[Type.StringListType], args);

    return ((res instanceof List) ? (List) res : null);
  }

  public synchronized void applyRoutes(RouteSubjectManager routeSubjectManager) throws AdminException
  {
    routeSubjectManager.apply();
    if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING) {
      refreshComponents();
      call(smsc_component, apply_routes_method, Type.Types[Type.StringType], new HashMap());
    }
  }

  public synchronized void applyAliases() throws AdminException
  {
    try {
      final File smscConfFolder = WebAppFolders.getSmscConfFolder();

      final File aliasConfigFile = new File(smscConfFolder, "aliases.xml");
      final File newFile = Functions.createNewFilenameForSave(aliasConfigFile);
      storeAliases(new PrintWriter(new FileWriter(newFile), true)).close();
      Functions.renameNewSavedFileToOriginal(newFile, aliasConfigFile);

      if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING) {
        refreshComponents();
        call(smsc_component, apply_aliases_method, Type.Types[Type.StringType], new HashMap());
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
    refreshComponents();
    HashMap args = new HashMap();
    args.put("address", mask.getMask());
    Object result = call(smsc_component, profile_lookup_method, Type.Types[Type.StringListType], args);
    if (result instanceof List)
      return new Profile(mask, (List) result);
    else
      throw new AdminException("Error in response");
  }

  public synchronized ProfileEx profileLookupEx(Mask mask) throws AdminException
  {
    refreshComponents();
    HashMap args = new HashMap();
    args.put("address", mask.getMask());
    Object result = call(smsc_component, profile_lookup_ex_method, Type.Types[Type.StringListType], args);
    if (result instanceof List)
      return new ProfileEx(mask, (List) result);
    else
      throw new AdminException("Error in response");
  }

  public synchronized int profileUpdate(Mask mask, Profile newProfile) throws AdminException
  {
    refreshComponents();
    HashMap args = new HashMap();
    args.put("address", mask.getMask());
    List profileArg = new LinkedList();
    profileArg.add(newProfile.getCodepageString());
    profileArg.add(newProfile.getReportOptionsString());
    profileArg.add(newProfile.getLocale());
    profileArg.add(newProfile.isAliasHide() ? "true" : "false");
    profileArg.add(newProfile.isAliasModifiable() ? "true" : "false");
    profileArg.add(newProfile.getDivert());
    profileArg.add(newProfile.isDivertActive() ? "true" : "false");
    profileArg.add(newProfile.isDivertModifiable() ? "true" : "false");

    args.put("profile", profileArg);
    return ((Long) call(smsc_component, profile_update_method, Type.Types[Type.IntType], args)).intValue();
  }

  public synchronized void profileDelete(Mask mask) throws AdminException
  {
    refreshComponents();
    HashMap args = new HashMap();
    args.put("address", mask.getMask());
    call(smsc_component, profile_delete_method, Type.Types[Type.IntType], args);
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
    refreshComponents();
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
    call(smsc_component, process_cancel_messages_method, Type.Types[Type.StringType], params);
  }

  public synchronized void flushStatistics() throws AdminException
  {
    refreshComponents();
    call(smsc_component, flush_statistics_method, Type.Types[Type.StringType], new HashMap());
  }

  protected void checkComponents()
  {
    super.checkComponents();
    if (apply_aliases_method == null || apply_routes_method == null || profile_lookup_method == null || profile_update_method == null || flush_statistics_method == null || process_cancel_messages_method == null || apply_smsc_config_method == null || apply_services_method == null || msc_registrate_method == null || msc_unregister_method == null || msc_block_method == null || msc_clear_method == null || msc_list_method == null || sme_add_method == null || sme_remove_method == null || sme_update_method == null || sme_status == null || sme_disconnect == null || log_get_categories == null || log_set_categories == null) {
      smsc_component = (Component) getInfo().getComponents().get("SMSC");
      apply_aliases_method = (Method) smsc_component.getMethods().get("apply_aliases");

      apply_routes_method = (Method) smsc_component.getMethods().get("apply_routes");
      load_routes_method = (Method) smsc_component.getMethods().get("load_routes");
      trace_route_method = (Method) smsc_component.getMethods().get("trace_route");

      profile_lookup_method = (Method) smsc_component.getMethods().get("lookup_profile");
      profile_lookup_ex_method = (Method) smsc_component.getMethods().get("profile_lookup_ex");
      profile_update_method = (Method) smsc_component.getMethods().get("update_profile");
      profile_delete_method = (Method) smsc_component.getMethods().get("profile_delete");

      flush_statistics_method = (Method) smsc_component.getMethods().get("flush_statistics");
      process_cancel_messages_method = (Method) smsc_component.getMethods().get("process_cancel_messages");
      apply_smsc_config_method = (Method) smsc_component.getMethods().get("apply_smsc_config");
      apply_services_method = (Method) smsc_component.getMethods().get("apply_services");
      apply_locale_resources_method = (Method) smsc_component.getMethods().get("apply_locale_resources");

      msc_registrate_method = (Method) smsc_component.getMethods().get("msc_registrate");
      msc_unregister_method = (Method) smsc_component.getMethods().get("msc_unregister");
      msc_block_method = (Method) smsc_component.getMethods().get("msc_block");
      msc_clear_method = (Method) smsc_component.getMethods().get("msc_clear");
      msc_list_method = (Method) smsc_component.getMethods().get("msc_list");

      sme_add_method = (Method) smsc_component.getMethods().get("sme_add");
      sme_remove_method = (Method) smsc_component.getMethods().get("sme_remove");
      sme_update_method = (Method) smsc_component.getMethods().get("sme_update");
      sme_status = (Method) smsc_component.getMethods().get("sme_status");
      sme_disconnect = (Method) smsc_component.getMethods().get("sme_disconnect");
      log_get_categories = (Method) smsc_component.getMethods().get("log_get_categories");
      log_set_categories = (Method) smsc_component.getMethods().get("log_set_categories");
    }
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
      config.save("ISO-8859-1");
    } catch (Throwable t) {
      logger.error("Couldn't store SMSC config", t);
      throw new AdminException("Couldn't store SMSC config: " + t.getMessage());
    }
  }

  public synchronized void applyConfig() throws AdminException
  {
    if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING) {
      refreshComponents();
      call(smsc_component, apply_smsc_config_method, Type.Types[Type.StringType], new HashMap());
    }
  }

  public synchronized void mscRegistrate(String msc) throws AdminException
  {
    refreshComponents();
    HashMap args = new HashMap();
    args.put("msc", msc);
    call(smsc_component, msc_registrate_method, Type.Types[Type.StringType], args);
  }

  public synchronized void mscUnregister(String msc) throws AdminException
  {
    refreshComponents();
    HashMap args = new HashMap();
    args.put("msc", msc);
    call(smsc_component, msc_unregister_method, Type.Types[Type.StringType], args);
  }

  public synchronized void mscBlock(String msc) throws AdminException
  {
    refreshComponents();
    HashMap args = new HashMap();
    args.put("msc", msc);
    call(smsc_component, msc_block_method, Type.Types[Type.StringType], args);
  }

  public synchronized void mscClear(String msc) throws AdminException
  {
    refreshComponents();
    HashMap args = new HashMap();
    args.put("msc", msc);
    call(smsc_component, msc_clear_method, Type.Types[Type.StringType], args);
  }

  public synchronized List mscList() throws AdminException
  {
    refreshComponents();
    Object result = call(smsc_component, msc_list_method, Type.Types[Type.StringListType], new HashMap());
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
    refreshComponents();
    Object result = call(smsc_component, sme_add_method, Type.Types[Type.BooleanType], putSmeIntoMap(sme));
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized void smeRemove(String smeId) throws AdminException
  {
    refreshComponents();
    Map params = new HashMap();
    params.put("id", smeId);
    Object result = call(smsc_component, sme_remove_method, Type.Types[Type.BooleanType], params);
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized void smeUpdate(SME sme) throws AdminException
  {
    refreshComponents();
    Object result = call(smsc_component, sme_update_method, Type.Types[Type.BooleanType], putSmeIntoMap(sme));
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized SmeStatus getSmeStatus(String id) throws AdminException
  {
    final long currentTime = System.currentTimeMillis();
    if (currentTime - Constants.ServicesRefreshTimeoutMillis > serviceRefreshTimeStamp) {
      refreshComponents();
      serviceRefreshTimeStamp = currentTime;
      smeStatuses.clear();
      Object result = call(smsc_component, sme_status, Type.Types[Type.StringListType], new HashMap());
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
    refreshComponents();
    Map params = new HashMap();
    params.put("ids", smeIdsToDisconnect);
    Object result = call(smsc_component, sme_disconnect, Type.Types[Type.BooleanType], params);
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new AdminException("Error in response");
  }

  public synchronized Map getLogCategories() throws AdminException
  {
    Map return_result = new HashMap();
    refreshComponents();
    Object resultO = call(smsc_component, log_get_categories, Type.Types[Type.StringListType], new HashMap());
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
    refreshComponents();
    Map params = new HashMap();
    LinkedList catsList = new LinkedList();
    params.put("categories", catsList);
    for (Iterator i = cats.entrySet().iterator(); i.hasNext();) {
      Map.Entry entry = (Map.Entry) i.next();
      final String catName = (String) entry.getKey();
      final String catPriority = (String) entry.getValue();
      catsList.add(catName + LOGGER_DELIMITER + catPriority);
    }
    call(smsc_component, log_set_categories, Type.Types[Type.BooleanType], params);
  }

  public synchronized void applyLocaleResources() throws AdminException
  {
    if (getInfo().getStatus() == ServiceInfo.STATUS_RUNNING) {
      refreshComponents();
      call(smsc_component, apply_locale_resources_method, Type.Types[Type.StringType], new HashMap());
    }
  }

  public File getConfigFolder()
  {
    return configFolder;
  }
}
