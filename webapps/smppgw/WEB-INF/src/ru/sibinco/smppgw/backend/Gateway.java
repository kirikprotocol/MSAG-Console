package ru.sibinco.smppgw.backend;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.service.ServiceInfo;
import ru.sibinco.lib.backend.service.Type;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.backend.protocol.Response;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.sme.SmeStatus;
import ru.sibinco.smppgw.backend.protocol.commands.*;
import ru.sibinco.smppgw.backend.sme.GwSme;
import ru.sibinco.smppgw.backend.sme.SmscInfo;
import ru.sibinco.smppgw.backend.routing.GwRoutingManager;
import org.w3c.dom.Element;

import java.util.*;


/**
 * Created by igork Date: 25.05.2004 Time: 15:50:34
 */
public class Gateway extends Proxy
{
  private final String id;
  private static final String SMPPGW_COMPONENT_ID = "SMPPGW";
  private static final String LOAD_ROUTES_METHOD_ID = "loadRoutes";
  private static final String TRACE_ROUTE_METHOD_ID = "traceRoute";
  private static final String SME_STATUS_ID = "statusSme";
  private long serviceRefreshTimeStamp = 0;
  private Map smeStatuses = new HashMap();
  public Gateway(final ServiceInfo gwServiceInfo, final int port)
  {
    super(gwServiceInfo.getHost(), port);
    id = gwServiceInfo.getId();
  }

  public String getId()
  {
    return id;
  }

  public void apply(final String subject) throws SibincoException
  {
    final Response response = super.runCommand(new Apply(subject));
    if (Response.StatusOk != response.getStatus())
      throw new SibincoException("Couldn't apply, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
  }

  public void updateSmeInfo(final GwSme gwSme) throws SibincoException
  {
    final Response response = super.runCommand(new UpdateSmeInfo(gwSme));
    if (Response.StatusOk != response.getStatus())
      throw new SibincoException("Couldn't update sme info, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
  }

  public void deleteSme(final String smeId) throws SibincoException
  {
    final Response response = super.runCommand(new DeleteSme(smeId));
    if (Response.StatusOk != response.getStatus())
      throw new SibincoException("Couldn't delete sme, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
  }

  public void addSme(final GwSme gwSme) throws SibincoException
  {
    final Response response = super.runCommand(new AddSme(gwSme));
    if (Response.StatusOk != response.getStatus())
      throw new SibincoException("Couldn't update sme info, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
  }

  public void regSmsc(final SmscInfo SmscInfo) throws SibincoException
  {
    final Response response = super.runCommand(new RegSmsc(SmscInfo));
    if (Response.StatusOk != response.getStatus())
      throw new SibincoException("Couldn't register Smsc , nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
  }

  public void unregSmsc(final SmscInfo SmscInfo) throws SibincoException
   {
     final Response response = super.runCommand(new UnregSmsc(SmscInfo));
     if (Response.StatusOk != response.getStatus())
       throw new SibincoException("Couldn't unregister Smsc , nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
   }

  public synchronized List loadRoutes(final GwRoutingManager gwRoutingManager)
          throws SibincoException
  {
    gwRoutingManager.trace();
    String err="Couldn't load active routes configuration, nested: ";
    final Object res = call( LOAD_ROUTES_METHOD_ID, err,Type.Types[Type.StringListType], new HashMap());
    return res instanceof List ? (List) res : null;
  }

  public synchronized SmeStatus getSmeStatus(final String id) throws SibincoException
  {
    final long currentTime = System.currentTimeMillis();
    if (currentTime - Constants.ServicesRefreshTimeoutMillis > serviceRefreshTimeStamp) {
      serviceRefreshTimeStamp = currentTime;
      smeStatuses.clear(); String err="Couldn't get sme status, nested: ";
      final Object result = call(SME_STATUS_ID,err, Type.Types[Type.StringListType], new HashMap());
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
  public Object call( final String commandId,final String err, final Type returnType, final Map arguments) throws SibincoException
   {
     //if (info.status != ServiceInfo.STATUS_RUNNING)
    // throw new SibincoException("Service \"" + info.getId() + "\" is not running");
    // refreshComponents();
       final Response r = runCommand(new CommandCall(commandId,  returnType, arguments));
       if (Response.StatusOk != r.getStatus())
         throw new SibincoException("Error occured: "+err + r.getDataAsString());
       final Element resultElem = (Element) r.getData().getElementsByTagName("variant").item(0);
       final Type resultType = Type.getInstance(resultElem.getAttribute("type"));
       switch (resultType.getId()) {
         case Type.StringType:
           return Utils.getNodeText(resultElem);
         case Type.IntType:
           return Long.decode(Utils.getNodeText(resultElem));
         case Type.BooleanType:
           return Boolean.valueOf(Utils.getNodeText(resultElem));
         case Type.StringListType:
           return translateStringList(Utils.getNodeText(resultElem));
         default:
           throw new SibincoException("Unknown result type");
       }
   }


  public synchronized List traceRoute(final String dstAddress, final String srcAddress, final String srcSysId)
          throws SibincoException
  {
    String err="Couldn't trace route , nested: ";
    final Map args = new HashMap();
    args.put("dstAddress", dstAddress);
    args.put("srcAddress", srcAddress);
    args.put("srcSysId", srcSysId);
    //final Object res = call(SMPPGW_COMPONENT_ID, TRACE_ROUTE_METHOD_ID, Type.Types[Type.StringListType], args);
    final Object res =call(TRACE_ROUTE_METHOD_ID,err,Type.Types[Type.StringListType], args);
    return res instanceof List ? (List) res : null;
  }


  protected List translateStringList(final String listStr)
    {
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
