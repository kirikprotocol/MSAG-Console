package ru.sibinco.smppgw.backend;

import ru.sibinco.lib.backend.service.Service;
import ru.sibinco.lib.backend.service.ServiceInfo;
import ru.sibinco.lib.backend.service.Type;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.WebAppFolders;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.smppgw.backend.routing.GwRoutingManager;
import ru.sibinco.smppgw.backend.protocol.alias.AliasSet;

import java.util.List;
import java.util.HashMap;
import java.util.Map;
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
  private static final String LOAD_ROUTES_METHOD_ID = "load_routes";
  private static final String TRACE_ROUTE_METHOD_ID = "trace_route";

  private File configFolder = null;
  private AliasSet aliases = null;
  public Smppgw(ServiceInfo info)
  {
    super(info);
  }
  public Smppgw(final String smppgwHost,final int smppgwPort,final String smscConfFolderString,  SmppGWAppContext smscAppContext) throws SibincoException
  {
  super(new ServiceInfo(Constants.SMPPGW_SME_ID, smppgwHost, "", "", true, null, ServiceInfo.STATUS_STOPPED), smppgwPort);
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

}
