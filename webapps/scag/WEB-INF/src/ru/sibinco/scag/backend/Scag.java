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
import ru.sibinco.scag.backend.protocol.commands.endpoints.AddCenter;
import ru.sibinco.scag.backend.protocol.commands.endpoints.AddSvc;
import ru.sibinco.scag.backend.protocol.commands.endpoints.DeleteCenter;
import ru.sibinco.scag.backend.protocol.commands.endpoints.DeleteSvc;
import ru.sibinco.scag.backend.protocol.commands.endpoints.UpdateCenter;
import ru.sibinco.scag.backend.protocol.commands.endpoints.UpdateSvcInfo;
import ru.sibinco.scag.backend.protocol.commands.rules.AddRule;
import ru.sibinco.scag.backend.protocol.commands.rules.RemoveRule;
import ru.sibinco.scag.backend.protocol.response.Response;
import ru.sibinco.scag.backend.routing.ScagRoutingManager;

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
    private static final String LOAD_ROUTES_METHOD_ID = "loadRoutes";
    private static final String TRACE_ROUTE_METHOD_ID = "traceRoute";
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

    public void removeRule(final String ruleId) throws SibincoException {
        try {
        final Response response = super.runCommand(new RemoveRule(ruleId));
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
       args.put("ruleId",ruleId);
       args.put("transport",transport);
       final Object res = call(ADD_RULE_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], args);
       return res instanceof List ? (List) res : null;
   }

    public synchronized List updateRule(final String ruleId, final String transport) throws SibincoException {
        String err = "Couldn't update rule , nested: ";
        HashMap args = new HashMap();
        args.put("ruleId",ruleId);
        args.put("transport",transport);
        final Object res = call(UPDATE_RULE_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], args);
        return res instanceof List ? (List) res : null;
    }


    public synchronized List loadRoutes(final ScagRoutingManager scagRoutingManager)
            throws SibincoException {
        scagRoutingManager.trace();
        String err = "Couldn't load active routes configuration, nested: ";
        final Object res = call(LOAD_ROUTES_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], new HashMap());
        return res instanceof List ? (List) res : null;
    }


    public Object call(final String commandId, final String err, final Type returnType, final Map arguments) throws SibincoException {
        //if (info.status != ServiceInfo.STATUS_RUNNING)
        // throw new SibincoException("Service \"" + info.getId() + "\" is not running");
        // refreshComponents();
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


    public synchronized List traceRoute(final String dstAddress, final String srcAddress, final String srcSysId)
            throws SibincoException {
        String err = "Couldn't trace route , nested: ";
        final Map args = new HashMap();
        args.put("dstAddress", dstAddress);
        args.put("srcAddress", srcAddress);
        args.put("srcSysId", srcSysId);
        final Object res = call(TRACE_ROUTE_METHOD_ID, err, Type.Types[Type.STRING_LIST_TYPE], args);
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
            result.add(new String());

        return result;
    }

}
