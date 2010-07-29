package ru.sibinco.lib.backend.service;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.backend.protocol.Response;
import ru.sibinco.lib.backend.protocol.CommandListComponents;
import ru.sibinco.lib.backend.protocol.CommandCall;
import ru.sibinco.lib.backend.util.xml.Utils;

import java.util.Map;
import java.util.List;
import java.util.LinkedList;

import org.w3c.dom.Element;


/**
 * Created by igork Date: 25.05.2004 Time: 15:50:34
 */
public class Service extends Proxy
{
  private ServiceInfo info = null;

  public Service(final ServiceInfo info)
  {
    this(info, -1);
  }

  public Service(final ServiceInfo info, final int port)
  {
    super(info.getHost(), port);
    this.info = info;
  }

  public ServiceInfo getInfo()
  {
    return info;
  }

  /**
   * Вызывает метод на компоненте сервиса.
   *
   * @param arguments Map поименованных аргументов (String -> Object). Аргументы могут быть String Integer или Boolean класса.
   * @return Значение, которое вернул вызванный метод (String, Integer или Boolean)
   */
  public Object call(final String componentId, final String methodId, final Type returnType, final Map arguments) throws SibincoException
  {
//    if (info.status != ServiceInfo.STATUS_RUNNING)
//      throw new AdminException("Service \"" + info.getId() + "\" is not running");

    refreshComponents();

    final Component component = (Component) info.getComponents().get(componentId);
    if (null == component)
      throw new SibincoException("Service \"" + info.getId() + "\" is not connected");
    final Method method = (Method) component.getMethods().get(methodId);
    if (null == method)
      throw new SibincoException("Version of service \"" + info.getId() + "\" and SMSC Administration Application is not compatible");

    if (method.equals(component.getMethods().get(method.getName()))) {
      final Response r = runCommand(new CommandCall(info.getId(), component.getName(), method.getName(), returnType, arguments));
      if (Response.StatusOk != r.getStatus())
        throw new SibincoException("Error occured: " + r.getDataAsString());
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
    else {
      logger.error("Incorrect method \"" + (null == method ? "<null>" : method.getName()) + "\" signature");

      // for debug purposes
      try {
        if (null != component)
          logger.debug("Component: " + component.getName());
        if (null != method)
          logger.debug("Called method:" + "\n  name: " + method.getName() + "\n  type: " + method.getType().getName() + "\n  params: " + method.getParams());
        if (null != component && null != component.getMethods().get(method.getName())) {
          final Method foundMethod = (Method) component.getMethods().get(method.getName());
          logger.debug("Found method:" + "\n  name: " + foundMethod.getName() + "\n  type: " + foundMethod.getType().getName() + "\n  params: "
                  + foundMethod.getParams());
        }
      } catch (Throwable e) {
      }

      throw new SibincoException("Incorrect method \"" + (null == method ? "<null>" : method.getName()) + "\" signature");
    }
  }

  private List translateStringList(final String listStr)
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

  private void refreshComponents() throws SibincoException
  {
    if (info.getComponents().isEmpty()) {
      logger.debug("refreshComponents");
      final Response r;

        r = runCommand(new CommandListComponents(info.getId()));
      if (Response.StatusOk != r.getStatus())
        throw new SibincoException("Error occured: " + r.getDataAsString());
      info.setComponents(r.getData().getDocumentElement());
    logger.debug("found " + info.getComponents().keySet().size() + " components: " + info.getComponents().keySet());
    }
  }

  protected void setInfo(final ServiceInfo info)
  {
    logger.debug("Set info. Status: " + info.getStatusStr() + " [" + info.getStatus() + ']');

    if (ServiceInfo.STATUS_RUNNING == info.getStatus()
            && (null == info.components || 0 == info.components.size())
            && null != this.info.components && 0 < this.info.components.size()) {
      info.components = this.info.components;
    }
    this.info = info;
  }
}