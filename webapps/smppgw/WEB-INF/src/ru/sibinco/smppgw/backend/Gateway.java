package ru.sibinco.smppgw.backend;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.service.ServiceInfo;
import ru.sibinco.lib.backend.service.Type;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.backend.protocol.Response;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.smppgw.backend.protocol.commands.*;
import ru.sibinco.smppgw.backend.sme.GwSme;
import ru.sibinco.smppgw.backend.routing.GwRoutingManager;
import org.w3c.dom.Element;

import java.util.List;
import java.util.LinkedList;
import java.util.Map;
import java.util.HashMap;


/**
 * Created by igork Date: 25.05.2004 Time: 15:50:34
 */
public class Gateway extends Proxy
{
  private final String id;
  private static final String LOAD_ROUTES_METHOD_ID = "loadRoutes";
  private static final String TRACE_ROUTE_METHOD_ID = "traceRoute";

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
  public Object loadRoutes(final String subject) throws SibincoException
   {
     final Response response = super.runCommand(new LoadRoutes(subject));
     if (Response.StatusOk != response.getStatus())
       throw new SibincoException("Couldn't load active routes configuration, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
     final Element resultElem = (Element) response.getData().getElementsByTagName("variant").item(0);
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
   public synchronized List loadRoutes(final GwRoutingManager gwRoutingManager)
          throws SibincoException
  {
    gwRoutingManager.trace();

    //final Object res = call(SMPPGW_COMPONENT_ID, LOAD_ROUTES_METHOD_ID, Type.Types[Type.StringListType], new HashMap());
    final Object res =loadRoutes(LOAD_ROUTES_METHOD_ID);
    return res instanceof List ? (List) res : null;
  }
 public  Object traceRoute(final String subject,final Map args) throws SibincoException
     {
      //final Response response = super.runCommand(new CommandCall(TRACE_ROUTE_METHOD_ID, TRACE_ROUTE_METHOD_ID, Type.Types[Type.StringListType], args));
       final Response response = runCommand(new CommandCall(subject, Type.Types[Type.StringListType], args));
       if (Response.StatusOk != response.getStatus())
             throw new SibincoException("Couldn't trace route , nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
      final Element resultElem = (Element) response.getData().getElementsByTagName("variant").item(0);
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

    final Map args = new HashMap();
    args.put("dstAddress", dstAddress);
    args.put("srcAddress", srcAddress);
    args.put("srcSysId", srcSysId);
    //final Object res = call(SMPPGW_COMPONENT_ID, TRACE_ROUTE_METHOD_ID, Type.Types[Type.StringListType], args);
    final Object res =traceRoute(TRACE_ROUTE_METHOD_ID,args);
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
