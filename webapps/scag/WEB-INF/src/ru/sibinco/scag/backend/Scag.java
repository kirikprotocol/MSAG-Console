package ru.sibinco.scag.backend;

import org.w3c.dom.Element;
import ru.sibinco.WHOISDIntegrator.TariffMatrixManager;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.backend.daemon.ServiceInfo;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.meta.MetaEndpoint;
import ru.sibinco.scag.backend.protocol.commands.Apply;
import ru.sibinco.scag.backend.protocol.commands.ApplyConfig;
import ru.sibinco.scag.backend.protocol.commands.CommandCall;
import ru.sibinco.scag.backend.protocol.commands.Type;
import ru.sibinco.scag.backend.protocol.commands.endpoints.*;
import ru.sibinco.scag.backend.protocol.commands.operators.ReloadOperators;
import ru.sibinco.scag.backend.protocol.commands.routes.ApplyHttpRoutes;
import ru.sibinco.scag.backend.protocol.commands.routes.ApplySmppRoutes;
import ru.sibinco.scag.backend.protocol.commands.rules.RemoveRule;
import ru.sibinco.scag.backend.protocol.commands.services.ReloadServices;
import ru.sibinco.scag.backend.protocol.commands.tariffmatrix.ReloadTariffMatrix;
import ru.sibinco.scag.backend.protocol.response.Response;
import ru.sibinco.scag.backend.routing.ScagRoutingManager;
import ru.sibinco.scag.backend.routing.http.HttpRoutingManager;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;


/**
 * Created by igork Date: 25.05.2004 Time: 15:50:34
 */
public class Scag extends Proxy {

    private final String id;

    private static final String SCAG_COMPONENT_ID = "msag";
    //************************ SMPP ROUTES **************************************//
    private static final String LOAD_SMPP_ROUTES_METHOD_ID = "loadSmppTraceRoutes";
    private static final String TRACE_SMPP_ROUTE_METHOD_ID = "traceSmppRoute";
    //*********************** HTTP ROUTES **************************************//
    private static final String LOAD_HTTP_ROUTES_METHOD_ID = "loadHttpTraceRoutes";
    private static final String TRACE_HTTP_ROUTE_METHOD_ID = "traceHttpRoute";

    private static final String ADD_RULE_METHOD_ID = "addRule";
    private static final String UPDATE_RULE_METHOD_ID = "updateRule";
    private static final char LOGGER_DELIMITER = ',';
    private static final String LIST_SME_METHOD_ID = "listSme";
    private static final String LIST_SMSC_METHOD_ID = "listSmsc";

    public Scag(final ServiceInfo gwServiceInfo, final int port) {
        super(gwServiceInfo.getHost(), port);
        logger.debug( "Scag.Scag(final ServiceInfo gwServiceInfo, final int port)=Scag(" + gwServiceInfo + "," + port + ")" );
        logger.debug( "Scag.Scag() gwServiceInfo.getId()=" + gwServiceInfo.getId() );
        id  = gwServiceInfo.getId();
    }

    public Scag(final String host, final int port) {
        super(host, port);
        logger.debug( "Scag.Scag(final String host, final int port)=Scag(" + host + "," + port + ")" );
        logger.debug( "Scag.Scag() id, id2" );
        id  = SCAG_COMPONENT_ID;
    }

    public Scag(final String host, final int port, String serviceId) {
        super(host, port);
        logger.debug( "Scag.Scag(final String host, final int port, String serviceId)=Scag(" + host + "," + port + "," + serviceId +")" );
        logger.debug( "Scag.Scag() serviceId=" + serviceId );
        id  = serviceId;
    }

    public String getId() {
        return id;
    }

    protected void apply(final String subject) throws SibincoException {
        final Response response = super.runCommand(new Apply(subject));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't apply, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    protected void applyConfig() throws SibincoException {
        final Response response = super.runCommand(new ApplyConfig());
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't apply config, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    //service points
    protected void addSvc(final Svc svc) throws SibincoException {
        final Response response = super.runCommand(new AddSvc(svc));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't add sme, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    protected void updateSvcInfo(final Svc svc) throws SibincoException {
        final Response response = super.runCommand(new UpdateSvcInfo(svc));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't update sme info, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    protected void updateSvc(final Svc svc) throws SibincoException {
        final Response response = super.runCommand(new UpdateSvc(svc));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't update sme info, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }


    protected void deleteSvc(final String svcId) throws SibincoException {
        final Response response = super.runCommand(new DeleteSvc(svcId,0));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't delete sme, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public void disconnectService(final String serviceId) throws SibincoException {
        try {
          final Response response = super.runCommand(new DeleteSvc(serviceId,1));
          if (Response.STATUS_OK != response.getStatus())
              throw new SibincoException("Couldn't disconnect sme, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
        } catch (SibincoException se) {
          if (getStatus() == STATUS_DISCONNECTED)
            throw new StatusDisconnectedException(host,port);
          else throw se;
        }
    }

    //centers
    protected void addCenter(final Center center) throws SibincoException {
        final Response response = super.runCommand(new AddCenter(center));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't register Smsc , nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    protected void updateCenter(final Center center) throws SibincoException {
        final Response response = super.runCommand(new UpdateCenter(center));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't modify Smsc , nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    protected void deleteCenter(final Center center) throws SibincoException {
        final Response response = super.runCommand(new DeleteCenter(center));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't delete Smsc, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }
// META
    protected void addMetaEntity(final MetaEndpoint meta) throws SibincoException {
        logger.info( "addMetaEntity");
        final Response response = super.runCommand(new AddMetaEntity(meta));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't register MetaEntity , nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    protected void updateMetaEntity(final MetaEndpoint meta) throws SibincoException {
        final Response response = super.runCommand(new UpdateMetaEntity(meta));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't modify MetaEntity , nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    protected void deleteMetaEntity(final MetaEndpoint meta) throws SibincoException {
        final Response response = super.runCommand(new DeleteMetaEntity(meta));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't delete MetaEntity, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    protected void addMetaEndpoint(final String[] pair) throws SibincoException {
        final Response response = super.runCommand(new AddMetaEndpoint(pair));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't add MetaEndpoint, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    protected void removeMetaEndpoint(final String[] pair) throws SibincoException {
        final Response response = super.runCommand(new RemoveMetaEndpoint(pair));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't delete MetaEntity, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }
//META

    //rules command invoked directly(not by invokeCommand(...)) because of specific logic
    public void removeRule(final String ruleId, final String transport) throws SibincoException {
        try {
        final Response response = super.runCommand(new RemoveRule(ruleId,transport));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't delete Rule, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
        } catch (SibincoException se) {
          if (getStatus() == STATUS_DISCONNECTED)
            throw new StatusDisconnectedException(host,port);
          else throw se;
        }
    }

    public List addRule(final String ruleId, final String transport) throws SibincoException {
       String err = "Couldn't add rule , nested: ";
       HashMap args = new HashMap();
       args.put("serviceId",ruleId);
       args.put("transport",transport);
       final Object res = call(ADD_RULE_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], args);
       return res instanceof List ? (List) res : null;
   }

    public List updateRule(final String ruleId, final String transport) throws SibincoException {
//        if( true ){
//            throw new SibincoException( "Test exception!!!" );
//        }
        String err = "Couldn't update rule , nested: ";
        HashMap args = new HashMap();
        args.put("serviceId",ruleId);
        args.put("transport",transport);
        final Object res = call(UPDATE_RULE_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], args);
        return res instanceof List ? (List)res : null;
    }

    //load routes
    public synchronized List loadSmppRoutes(final ScagRoutingManager scagRoutingManager)
            throws SibincoException {
        scagRoutingManager.trace();
        String err = "Couldn't load active smpp routes configuration, nested: ";
        final Object res = call(LOAD_SMPP_ROUTES_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], new HashMap());
        return res instanceof List ? (List) res : null;
    }

    public synchronized List loadHttpRoutes(final HttpRoutingManager httpRoutingManager)
            throws SibincoException {
        httpRoutingManager.trace();
        String err = "Couldn't load active http routes configuration, nested: ";
        final Object res = call(LOAD_HTTP_ROUTES_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], new HashMap());
        return res instanceof List ? (List) res : null;
    }

    //operators
    protected void reloadOperators() throws SibincoException {
       final Response response = super.runCommand(new ReloadOperators());
       if (Response.STATUS_OK!=response.getStatus())
         throw new SibincoException("Couldn't reload operators, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    //apply routes
    protected void applySmppRoutes() throws SibincoException {
        final Response response = super.runCommand(new ApplySmppRoutes());
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't apply smpp routes, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    protected void applyHttpRoutes() throws SibincoException {
       final Response response = super.runCommand(new ApplyHttpRoutes());
       if (Response.STATUS_OK!=response.getStatus())
         throw new SibincoException("Couldn't apply http routes, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    //service providers
    protected void reloadServices() throws SibincoException {
      final Response response = super.runCommand(new ReloadServices());
      if (Response.STATUS_OK!=response.getStatus())
        throw new SibincoException("Couldn't reload services, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    //tariff matrix
    protected void reloadTariffMatrix() throws SibincoException {
      final Response response = super.runCommand(new ReloadTariffMatrix());
      if (Response.STATUS_OK!=response.getStatus())
        throw new SibincoException(TariffMatrixManager.WHOISD_ERROR_PREFIX+", nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public List getSmscInfo() throws SibincoException {
        String err = "Couldn't get SMSC info , nested: ";
        final Object res = call(LIST_SMSC_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], new HashMap());
        return res instanceof List ? (List) res : null;
    }

    public List getSmeInfo() throws SibincoException {
        String err = "Couldn't get SME info , nested: ";
        final Object res = call(LIST_SME_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], new HashMap());
        return res instanceof List ? (List) res : null;
    }

    public Object call(final String commandId, final String err, final Type returnType, final Map arguments) throws SibincoException {
        try {
            logger.debug( "Scag.call() start command='" + commandId + "'" );
            final Response r = runCommand(new CommandCall(commandId, returnType, arguments));
//            if (Response.STATUS_OK != r.getStatus()){
            if( Response.STATUS_ERROR == r.getStatus() ){
                logger.error( "Scag.call() STATUS_ERROR!!!" );
                throw new SibincoException( "Error occured: '" + err + "' | '" + r.getDataAsString() + "'" );
            }
            final Element resultElem = (Element) r.getData().getElementsByTagName("variant").item(0);
            final Type resultType = Type.getInstance(resultElem.getAttribute("type"));
            switch (resultType.getId()) {
                case Type.STRING_TYPE:
                    return Utils.getNodeText(resultElem);
                case Type.INT_TYPE:
                    return Long.decode(Utils.getNodeText(resultElem));
                case Type.BOOLEAN_TYPE:
                    return Boolean.valueOf(Utils.getNodeText(resultElem));
                case Type.STRING_LIST_TYPE:
                    return translateStringList(Utils.getNodeText(resultElem));
                default:
                    logger.error( "Scag:call():default");
                    throw new SibincoException("Unknown result type");
             }
        } catch (SibincoException se) {
           logger.debug( "Scag.call() SibincoException" );
           if (getStatus() == STATUS_DISCONNECTED) {
               logger.debug( "Scag.call() StatusDisconnectedException" );
               throw new StatusDisconnectedException(host,port);
           }
           else {
               throw se;
           }
         }
    }

    public synchronized List traceSmppRoute(final String dstAddress, final String srcAddress, final String srcSysId)
            throws SibincoException {
        String err = "Couldn't trace route , nested: ";
        final Map args = new HashMap();
        args.put("dstAddress", dstAddress);
        args.put("srcAddress", srcAddress);
        args.put("srcSysId", srcSysId);
        final Object res = call(TRACE_SMPP_ROUTE_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], args);
        return res instanceof List ? (List) res : null;
    }

    public synchronized List traceHttpRoute(final int serviceId, final int routeId, final String abonent, final String site, final String path, final int port)
            throws SibincoException {
        String err = "Couldn't trace route , nested: ";
        final Map args = new HashMap();
        args.put("sid", new Integer(serviceId));
        args.put("rid", new Integer(routeId));
        args.put("abonent", abonent);
        args.put("site", site);
        args.put("path", path);
        args.put("port", new Integer(port));
        final Object res = call(TRACE_HTTP_ROUTE_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], args);
        return res instanceof List ? (List) res : null;
    }


    protected List translateStringList(final String listStr) {
        if (null == listStr || 0 == listStr.length())
            return new LinkedList();
        final StringBuffer buffer = new StringBuffer(listStr.length());
        final List result = new LinkedList();
        for (int i = 0; i < listStr.length(); i++) {
            char c = listStr.charAt(i);
            if (',' == c) {
                result.add(buffer.toString());
                buffer.setLength(0);
                continue;
            }
            if ('\\' == c && i < listStr.length() - 1) {
                c = listStr.charAt(++i);
            }
            buffer.append(c);
        }
        if (0 < buffer.length())
            result.add(buffer.toString());
        else if (',' == listStr.charAt(listStr.length() - 1))
            result.add("");

        return result;
    }

    public synchronized Map getLogCategories() throws SibincoException {
        final Map return_result = new HashMap();
        String err = "Couldn't get LogCategories , nested: ";
        logger.debug( "Scag.getLogCatagories() call()" );
        final Object result0 = call( "getLogCategories", err, Type.Types[Type.STRING_LIST_TYPE], new HashMap() );
        logger.debug( "Scag.getLogCatagories() result0 = " + result0.toString() );
        if (result0 instanceof List) {
            final List result = (List) result0;
            for (Iterator iterator = result.iterator(); iterator.hasNext();) {
                final String cat = (String) iterator.next();
                final int delim_pos = cat.lastIndexOf(LOGGER_DELIMITER);
                if (0 <= delim_pos) {
                    final String name = cat.substring(0, delim_pos);
                    final String value = cat.substring(delim_pos + 1);
                    return_result.put(name, value);
                } else
                    logger.error("Error in response: string \"" + cat + "\" misformated.");
            }
        } else
            throw new SibincoException("Error in response");
        return return_result;
    }

    public synchronized void setLogCategories(final Map cats) throws SibincoException {
        logger.debug( "Scag.setLogCategories()" );
        final Map params = new HashMap();
        final List catsList = new LinkedList();
        params.put("categories", catsList);
        String err = "Couldn't set LogCategories , nested: ";
        for (Iterator iterator = cats.entrySet().iterator(); iterator.hasNext();) {
            final Map.Entry entry = (Map.Entry) iterator.next();
            final String catName = (String) entry.getKey();
            final String catPriority = (String) entry.getValue();
            catsList.add(catName + LOGGER_DELIMITER + catPriority);
            params.put( catName, catPriority ); //added code
        }
        call("setLogCategories", err, Type.Types[Type.BOOLEAN_TYPE], params);
    }

    public synchronized void storeLogCategories(final Map cats) throws SibincoException {
        logger.debug( "Scag.storeLogCategories()" );
        final Map params = new HashMap(cats);
        String err = "Couldn't set LogCategories , nested: ";
        call("storeLogCategories", err, Type.Types[Type.BOOLEAN_TYPE], params);
    }

    //common logic of command executing
    //1. "save" or "delete" button pressed
    public void invokeCommand(final String commandName, final Object paramsObject, final SCAGAppContext appContext,
                              final Manager manager, final String configFilename)
                              throws SibincoException {
        logger.debug( "Scag.invokeCommand().start commandName='" +commandName + "'" );
        System.out.println("Scag.invokeCommand().start commandName='" +commandName + "'");
        try{
        //2.save current config to temporary file(smpp.xml->smpp.xml.old)

        File configFile = new File(configFilename);
        File temporary = Functions.createTempFilename(configFile.getName(),".old",configFile.getParentFile());
        Functions.RenameFile(configFile,temporary);
       //3.save config
        logger.debug( "Scag:invokeCommand:store file , manager '" + manager.getClass().getName() + "'" );
        manager.store();
//moved from end
        logger.debug("Scag.invokeCommand().store file hsDaemon");
        appContext.getHSDaemon().store(configFile);

       //3.1 parse method used to localy validate saved config
        try {
            logger.debug("Scag.invokeCommand() parse" );
            manager.parse();
        } catch (Throwable e) {
            Functions.renameNewSavedFileToOriginal(temporary,configFile,true);
            logger.error("Scag.invokeCommand() Throwable" );
            throw new SibincoException(e);
        }
       //4.send command
        try
        {
          try
          {
              Method commandMethod;
              logger.debug( "Scag.invokeCommand() commandName='" +commandName + "' paramsObject==null is '" + (paramsObject==null) + "'" );
              if (paramsObject==null) {
                  commandMethod = this.getClass().getDeclaredMethod(commandName, new Class[]{});
                  commandMethod.invoke(appContext.getScag(),null);
              } else {
                  commandMethod = this.getClass().getDeclaredMethod(commandName, new Class[]{paramsObject.getClass()});
                  commandMethod.invoke(appContext.getScag(),new Object[]{paramsObject});
              }
          } catch (NoSuchMethodException e) {
              logger.error("Scag.invokeCommand NoSuchMethodException! scag doesn't support command "+commandName,e);
              e.printStackTrace();
          } catch (IllegalAccessException e) {
              logger.error("Scag.invokeCommand IllegalAccessException! scag doesn't support command "+commandName,e);
              e.printStackTrace();
          } catch (InvocationTargetException e) {
              logger.error("Scag.invokeCommand InvocationTargetException! scag doesn't support command " + commandName, e);
              throw (SibincoException)e.getTargetException();
          }
        }
        catch (SibincoException se) {
            //5. does service started?
            if (getStatus() == STATUS_DISCONNECTED) {
              //5. N0(service isn't started)
              //9. save smpp.xml to backup!
              //10. delete smpp.xml.old
            //          Functions.SavedFileToBackup(temporary,".old");
            //          appContext.getHSDaemon().store(configFile);
                logger.error("Scag.invokeCommand1 SibincoException (getStatus() == STATUS_DISCONNECTED)\n NEW DATA STORED ONLY IN FILE\t");
                appContext.getHSDaemon().store(Functions.ReturnSavedFileToBackup(temporary,".old"));
                throw new StatusDisconnectedException(host,port);
            } else {
                //5. YES(service is started)
                //6. service return error
                //7. Restore config from temporary file(smpp.xml.new -> smpp.xml)
                logger.error("Scag.invokeCommand1 SibincoException (getStatus() != STATUS_DISCONNECTED)\nRETURN OLD FILE\t");
                Functions.renameNewSavedFileToOriginal(temporary,configFile,true);
                appContext.getHSDaemon().store(configFile);
                //Paint error on the screen by throwing exception(next string)
                throw se;
            }
        }
      //we don't have any exception, so
      //9. save smpp.xml to backup!
      //10. delete smpp.xml.new
//o      Functions.SavedFileToBackup(temporary,".old");
      logger.error("Scag:invokeCommand1:done!ReturnSavedFileToBackup");
      appContext.getHSDaemon().store(Functions.ReturnSavedFileToBackup(temporary,".old"));
//moved above
//o      appContext.getHSDaemon().store(configFile);
      logger.error("Scag:invokeCommand1:done!:end");
   } catch (IOException e) {
     //e.printStackTrace();
     logger.error("Scag:invokeCommand1:IO operation failed - couldn't save config",e);
     throw new SibincoException("IO operation failed - couldn't save config");
   }
  }
//2
//    public void invokeCommand(final String commandName, final Object paramsObject, final SCAGAppContext appContext,
//                              final Manager manager, final String configFilename, boolean store) throws SibincoException {
//     try{
//       //2.save current config to temporary file(smpp.xml->smpp.xml.old)
//       File configFile = new File(configFilename);
//       File temporary = Functions.createTempFilename(configFile.getName(),".old",configFile.getParentFile());
//       Functions.RenameFile(configFile,temporary);
//       //3.save config
//       logger.error("Scag:invokeCommand2:store");
//       if( store ){
//           logger.error("Scag:invokeCommand2:store:in");
//            manager.store();
//       }
//       logger.error("Scag:invokeCommand2:store:daemon");
//       appContext.getHSDaemon().store(configFile);
//       //3.1 parse method used to localy validate saved config
//       try {
//         manager.parse();
//       } catch (Throwable e) {
//         Functions.renameNewSavedFileToOriginal(temporary,configFile,true);
//         throw new SibincoException(e);
//       }
//       //4.send command
//     try {
//       try {
//            Method commandMethod;
//            if (paramsObject==null) {
//                commandMethod = this.getClass().getDeclaredMethod(commandName, new Class[]{});
//                commandMethod.invoke(appContext.getScag(),null);
//            } else {
//                commandMethod = this.getClass().getDeclaredMethod(commandName, new Class[]{paramsObject.getClass()});
//                commandMethod.invoke(appContext.getScag(),new Object[]{paramsObject});
//            }
//        } catch (NoSuchMethodException e) {
//            logger.error("scag doesn't support command "+commandName,e);
//            e.printStackTrace();
//        } catch (IllegalAccessException e) {
//            e.printStackTrace();
//        } catch (InvocationTargetException e) {
//            throw (SibincoException)e.getTargetException();
//        }
//      } catch (SibincoException se) {
//        //5. does service started?
//        if (getStatus() == STATUS_DISCONNECTED) {
//          //5. N0(service isn't started)
//          //9. save smpp.xml to backup!
//          //10. delete smpp.xml.old
//            logger.error("Scag:invokeCommand2:SibincoException:(getStatus() == STATUS_DISCONNECTED)");
//            Functions.SavedFileToBackup(temporary,".old");
//            Functions.ReturnSavedFileToBackup(temporary,".old");
//            appContext.getHSDaemon().store( Functions.ReturnSavedFileToBackup(temporary,".old") );
//            throw new StatusDisconnectedException(host,port);
//        } else {
//          //5. YES(service is started)
//          //6. service return error
//          //7. Restore config from temporary file(smpp.xml.new -> smpp.xml)
//            logger.error("Scag:invokeCommand2:SibincoException:(getStatus() != STATUS_DISCONNECTED)");
//            Functions.renameNewSavedFileToOriginal(temporary,configFile,true);
//            appContext.getHSDaemon().store(configFile);
//          //Paint error on the screen by throwing exception(next string)
//            throw se;
//        }
//      }
//      //we don't have any exception, so
//      //9. save smpp.xml to backup!
//      //10. delete smpp.xml.new
////      Functions.SavedFileToBackup(temporary,".old");
//      logger.error("Scag:invokeCommand2:done!:end");
//      appContext.getHSDaemon().store( Functions.ReturnSavedFileToBackup(temporary,".old") );
////moved above
////      appContext.getHSDaemon().store(configFile);
//      logger.error("Scag:invokeCommand2:SibincoException:done!");
//   } catch (IOException e) {
//     //e.printStackTrace();
//     logger.error("Scag:invokeCommand2:IO operation failed - couldn't save config",e);
//     throw new SibincoException("IO operation failed - couldn't save config");
//   }
//  }
}
