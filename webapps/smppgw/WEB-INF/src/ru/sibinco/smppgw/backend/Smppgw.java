package ru.sibinco.smppgw.backend;

import ru.sibinco.lib.backend.service.Service;
import ru.sibinco.lib.backend.service.ServiceInfo;
import ru.sibinco.lib.backend.service.Type;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.WebAppFolders;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.backend.sme.SmeStatus;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.smppgw.backend.routing.GwRoutingManager;
import ru.sibinco.smppgw.backend.protocol.alias.AliasSet;
import ru.sibinco.lib.Constants;
import java.util.List;
import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.io.*;

import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 15:44:02
 * To change this template use File | Settings | File Templates.
 */
public class Smppgw  extends Service
{
  private static final String SMPPGW_COMPONENT_ID = "SMPPGW";
  private static final String APPLY_ALIASES_METHOD_ID = "apply_aliases";
  private static final String APPLY_ROUTES_METHOD_ID = "apply_routes";
  private static final String LOAD_ROUTES_METHOD_ID = "loadRoutes";
  private static final String TRACE_ROUTE_METHOD_ID = "traceRoute";

  private static final String SME_ADD_METHOD_ID = "sme_add";
  private static final String SME_REMOVE_METHOD_ID = "sme_remove";
  private static final String SME_UPDATE_METHOD_ID = "sme_update";
  private static final String SME_STATUS_ID = "sme_status";
  private static final String SMPPGW_SME_ID = "SMPPGW";
  private File configFolder = null;
  private AliasSet aliases = null;
  private long serviceRefreshTimeStamp = 0;
  private Map smeStatuses = new HashMap();
  public Smppgw(ServiceInfo info)
  {
    super(info);
  }
  public Smppgw(final String smppgwHost,final int smppgwPort,final String smscConfFolderString,  SmppGWAppContext smscAppContext) throws SibincoException
  {
  super(new ServiceInfo(SMPPGW_SME_ID, smppgwHost, "", "", true, null, ServiceInfo.STATUS_STOPPED), smppgwPort);
    try {
          this.configFolder = new File(smscConfFolderString);
          File file=new File(configFolder, "aliases.xml");
          FileReader fileReader=new FileReader(file);
          final Document aliasesDoc = Utils.parse(fileReader);
          aliases = new AliasSet(aliasesDoc.getDocumentElement(), smscAppContext);
        } catch (FactoryConfigurationError error) {
          logger.error("Couldn't configure xml parser factory", error);
          throw new SibincoException("Couldn't configure xml parser factory: " + error.getMessage());
        } catch (ParserConfigurationException e) {
          logger.error("Couldn't configure xml parser", e);
          throw new SibincoException("Couldn't configure xml parser: " + e.getMessage());
        } catch (SAXException e) {
          logger.error("Couldn't parse", e);
          throw new SibincoException("Couldn't parse: " + e.getMessage());
        } catch (IOException e) {
          logger.error("Couldn't read", e);
          throw new SibincoException("Couldn't read: " + e.getMessage());
        } catch (NullPointerException e) {
          logger.error("Couldn't parse", e);
          throw new SibincoException("Couldn't parse: " + e.getMessage());
        }


  }
  public synchronized List loadRoutes(final GwRoutingManager gwRoutingManager)
          throws SibincoException
  {
    gwRoutingManager.trace();
    if (ServiceInfo.STATUS_RUNNING != getInfo().getStatus())
      throw new SibincoException("SMPPGW is not running.");

    final Object res = call(SMPPGW_COMPONENT_ID, LOAD_ROUTES_METHOD_ID, Type.Types[Type.StringListType], new HashMap());

    return res instanceof List ? (List) res : null;
  }

  public synchronized List traceRoute(final String dstAddress, final String srcAddress, final String srcSysId)
          throws SibincoException
  {
    if (ServiceInfo.STATUS_RUNNING != getInfo().getStatus())
      throw new SibincoException("SMSC is not running.");

    final Map args = new HashMap();
    args.put("dstAddress", dstAddress);
    args.put("srcAddress", srcAddress);
    args.put("srcSysId", srcSysId);
    final Object res = call(SMPPGW_COMPONENT_ID, TRACE_ROUTE_METHOD_ID, Type.Types[Type.StringListType], args);

    return res instanceof List ? (List) res : null;
  }
  public synchronized void applyRoutes(final GwRoutingManager gwRoutingManager) throws SibincoException
   {
     gwRoutingManager.apply();
     if (ServiceInfo.STATUS_RUNNING == getInfo().getStatus()) {
       call(SMPPGW_COMPONENT_ID, APPLY_ROUTES_METHOD_ID, Type.Types[Type.StringType], new HashMap());
     }
   }

   public synchronized void applyAliases() throws SibincoException
   {
     try {
       final File smscConfFolder = WebAppFolders.getSmscConfFolder();

       final File aliasConfigFile = new File(smscConfFolder, "aliases.xml");
       final File newFile = Functions.createNewFilenameForSave(aliasConfigFile);
       storeAliases(new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()))).close();
       Functions.renameNewSavedFileToOriginal(newFile, aliasConfigFile);

       if (ServiceInfo.STATUS_RUNNING == getInfo().getStatus()) {
         call(SMPPGW_COMPONENT_ID, APPLY_ALIASES_METHOD_ID, Type.Types[Type.StringType], new HashMap());
       }
       else
         logger.debug("Couldn't call apply method on SMSC - SMSC is not running. Status is " + getInfo().getStatusStr() + " (" + getInfo().getStatus() + ")");
     } catch (FileNotFoundException e) {
       throw new SibincoException("Couldn't apply_routes new settings: Couldn't write to destination config file: " + e.getMessage());
     } catch (IOException e) {
       throw new SibincoException("Couldn't apply_routes new settings: " + e.getMessage());
     }
   }

  public AliasSet getAliases()
  {
    return aliases;
  }

     protected PrintWriter storeAliases(final PrintWriter out)
  {
    Functions.storeConfigHeader(out, "aliases", "AliasRecords.dtd");
    aliases.store(out);
    Functions.storeConfigFooter(out, "aliases");
    return out;
  }

  public synchronized void smeAdd(final Sme sme) throws SibincoException
  {
    final Object result = call(SMPPGW_COMPONENT_ID, SME_ADD_METHOD_ID, Type.Types[Type.BooleanType], putSmeIntoMap(sme));
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new SibincoException("Error in response");
  }

  public synchronized void smeRemove(final String smeId) throws SibincoException
  {
    final Map params = new HashMap();
    params.put("id", smeId);
    final Object result = call(SMPPGW_COMPONENT_ID, SME_REMOVE_METHOD_ID, Type.Types[Type.BooleanType], params);
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new SibincoException("Error in response");
  }

  public synchronized void smeUpdate(final Sme sme) throws SibincoException
  {
    final Object result = call(SMPPGW_COMPONENT_ID, SME_UPDATE_METHOD_ID, Type.Types[Type.BooleanType], putSmeIntoMap(sme));
    if (!(result instanceof Boolean && ((Boolean) result).booleanValue()))
      throw new SibincoException("Error in response");
  }
   public synchronized SmeStatus getSmeStatus(final String id) throws SibincoException
  {
    final long currentTime = System.currentTimeMillis();
    if (currentTime - Constants.ServicesRefreshTimeoutMillis > serviceRefreshTimeStamp) {
      serviceRefreshTimeStamp = currentTime;
      smeStatuses.clear();
      final Object result = call(SMPPGW_COMPONENT_ID, SME_STATUS_ID, Type.Types[Type.StringListType], new HashMap());
      if (!(result instanceof List))
        throw new SibincoException("Error in response");

      for (Iterator i = ((List) result).iterator(); i.hasNext();) {
        final String s = (String) i.next();
        final SmeStatus smeStatus = new SmeStatus(s);
        smeStatuses.put(smeStatus.getId(), smeStatus);
      }
    }
    return (SmeStatus) smeStatuses.get(id);
  }
   private Map putSmeIntoMap(final Sme sme)
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
}