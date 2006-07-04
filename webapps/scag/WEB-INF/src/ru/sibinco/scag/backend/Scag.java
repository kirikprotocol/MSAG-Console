package ru.sibinco.scag.backend;

import org.w3c.dom.Element;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.backend.daemon.ServiceInfo;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.protocol.commands.Apply;
import ru.sibinco.scag.backend.protocol.commands.CommandCall;
import ru.sibinco.scag.backend.protocol.commands.Type;
import ru.sibinco.scag.backend.protocol.commands.routes.ApplyHttpRoutes;
import ru.sibinco.scag.backend.protocol.commands.routes.ApplySmppRoutes;
import ru.sibinco.scag.backend.protocol.commands.tariffmatrix.ReloadTariffMatrix;
import ru.sibinco.scag.backend.protocol.commands.services.ReloadServices;
import ru.sibinco.scag.backend.protocol.commands.operators.ReloadOperators;
import ru.sibinco.scag.backend.protocol.commands.endpoints.AddCenter;
import ru.sibinco.scag.backend.protocol.commands.endpoints.AddSvc;
import ru.sibinco.scag.backend.protocol.commands.endpoints.DeleteCenter;
import ru.sibinco.scag.backend.protocol.commands.endpoints.DeleteSvc;
import ru.sibinco.scag.backend.protocol.commands.endpoints.UpdateCenter;
import ru.sibinco.scag.backend.protocol.commands.endpoints.UpdateSvcInfo;
import ru.sibinco.scag.backend.protocol.commands.rules.RemoveRule;
import ru.sibinco.scag.backend.protocol.response.Response;
import ru.sibinco.scag.backend.routing.ScagRoutingManager;
import ru.sibinco.scag.backend.routing.http.HttpRoutingManager;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;


/**
 * Created by igork Date: 25.05.2004 Time: 15:50:34
 */
public class Scag extends Proxy {                           

    private final String id;

    private static final String SCAG_COMPONENT_ID = "scag";
    //************************ SMPP ROUTES **************************************//
    private static final String LOAD_SMPP_ROUTES_METHOD_ID = "loadSmppTraceRoutes";
    private static final String TRACE_SMPP_ROUTE_METHOD_ID = "traceSmppRoute";
    //*********************** HTTP ROUTES **************************************//
    private static final String LOAD_HTTP_ROUTES_METHOD_ID = "loadHttpTraceRoutes";
    private static final String TRACE_HTTP_ROUTE_METHOD_ID = "traceHttpRoute";

    private static final String ADD_RULE_METHOD_ID = "addRule";
    private static final String UPDATE_RULE_METHOD_ID = "updateRule";

    public Scag(final ServiceInfo gwServiceInfo, final int port) {
        super(gwServiceInfo.getHost(), port);
        id = gwServiceInfo.getId();
    }

    public Scag(final String host, final int port) {
        super(host, port);
        id = SCAG_COMPONENT_ID;
    }

    public Scag(final String host, final int port, String serviceId) {
        super(host, port);
        id = serviceId;
    }
                                    
    public String getId() {
        return id;
    }

    public void apply(final String subject) throws SibincoException {
        final Response response = super.runCommand(new Apply(subject));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't apply, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public void addSvc(final Svc svc) throws SibincoException {
        final Response response = super.runCommand(new AddSvc(svc));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't add sme, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public void updateSvcInfo(final Svc svc) throws SibincoException {
        final Response response = super.runCommand(new UpdateSvcInfo(svc));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't update sme info, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public void deleteSvc(final String svcId) throws SibincoException {
        final Response response = super.runCommand(new DeleteSvc(svcId));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't delete sme, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public void addCenter(final Center center) throws SibincoException {
        final Response response = super.runCommand(new AddCenter(center));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't register Smsc , nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public void updateCenter(final Center center) throws SibincoException {
        final Response response = super.runCommand(new UpdateCenter(center));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't modify Smsc , nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public void deleteCenter(final Center center) throws SibincoException {
        final Response response = super.runCommand(new DeleteCenter(center));
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't delete Smsc, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

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

    public synchronized List addRule(final String ruleId, final String transport) throws SibincoException {
       String err = "Couldn't add rule , nested: ";
       HashMap args = new HashMap();
       args.put("serviceId",ruleId);
       args.put("transport",transport);
       final Object res = call(ADD_RULE_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], args);
       return res instanceof List ? (List) res : null;
   }

    public synchronized List updateRule(final String ruleId, final String transport) throws SibincoException {
        String err = "Couldn't update rule , nested: ";
        HashMap args = new HashMap();
        args.put("serviceId",ruleId);
        args.put("transport",transport);
        final Object res = call(UPDATE_RULE_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], args);
        return res instanceof List ? (List) res : null;
    }

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

    public void reloadOperators() throws SibincoException {
       final Response response = super.runCommand(new ReloadOperators());
       if (Response.STATUS_OK!=response.getStatus())
         throw new SibincoException("Couldn't reload operators, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public void applySmppRoutes() throws SibincoException {
        final Response response = super.runCommand(new ApplySmppRoutes());
        if (Response.STATUS_OK != response.getStatus())
            throw new SibincoException("Couldn't apply smpp routes, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public void applyHttpRoutes() throws SibincoException {
       final Response response = super.runCommand(new ApplyHttpRoutes());
       if (Response.STATUS_OK!=response.getStatus())
         throw new SibincoException("Couldn't apply http routes, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public void reloadServices() throws SibincoException {
      final Response response = super.runCommand(new ReloadServices());
      if (Response.STATUS_OK!=response.getStatus())
        throw new SibincoException("Couldn't reload services, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
    }

    public void reloadTariffMatrix() throws SibincoException {
      try {
      final Response response = super.runCommand(new ReloadTariffMatrix());
      if (Response.STATUS_OK!=response.getStatus())
        throw new SibincoException("Couldn't reload tariff matrix, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
      }  catch (SibincoException se) {
           if (getStatus() == STATUS_DISCONNECTED)
             throw new StatusDisconnectedException(host,port);
           else throw se;
      }
    }

    public Object call(final String commandId, final String err, final Type returnType, final Map arguments) throws SibincoException {
        try {
        final Response r = runCommand(new CommandCall(commandId, returnType, arguments));
        if (Response.STATUS_OK != r.getStatus())
            throw new SibincoException("Error occured: " + err + r.getDataAsString());
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
                throw new SibincoException("Unknown result type");
         }
        } catch (SibincoException se) {
           if (getStatus() == STATUS_DISCONNECTED)
             throw new StatusDisconnectedException(host,port);
           else throw se;
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

    public synchronized List traceHttpRoute(final String abonent, final String site, final String path, final int port)
            throws SibincoException {
        String err = "Couldn't trace route , nested: ";
        final Map args = new HashMap();
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

}
