/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:35:08 PM
 */
package ru.novosoft.smsc.admin.service;

import org.w3c.dom.Element;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.protocol.*;
import ru.novosoft.smsc.admin.utli.Proxy;
import ru.novosoft.smsc.util.xml.Utils;

import java.util.*;


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
  public Object call(final String componentId, final String methodId, final Type returnType, final Map arguments) throws AdminException
  {
//    if (info.status != ServiceInfo.STATUS_RUNNING)
//      throw new AdminException("Service \"" + info.getId() + "\" is not running");

    refreshComponents();

    final Component component = (Component) info.getComponents().get(componentId);
    if (null == component)
      throw new AdminException("Service \"" + info.getId() + "\" is not connected");
    final Method method = (Method) component.getMethods().get(methodId);
    if (null == method)
      throw new AdminException("Version of service \"" + info.getId() + "\" and SMSC Administration Application is not compatible");

    if (method.equals(component.getMethods().get(method.getName()))) {
      final Response r = runCommand(new CommandCall(info.getId(), component.getName(), method.getName(), returnType, arguments));
      if (Response.StatusOk != r.getStatus())
        throw new AdminException("Error occured: " + r.getDataAsString());
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
          throw new AdminException("Unknown result type");
      }
    } else {
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

      throw new AdminException("Incorrect method \"" + (null == method ? "<null>" : method.getName()) + "\" signature");
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

  private void refreshComponents() throws AdminException
  {
    if (info.getComponents().isEmpty()) {
      logger.debug("refreshComponents");
      final Response r = runCommand(new CommandListComponents(info.getId()));
      if (Response.StatusOk != r.getStatus())
        throw new AdminException("Error occured: " + r.getDataAsString());
      info.setComponents(r.getData().getDocumentElement());
      logger.debug("found " + info.getComponents().keySet().size() + " components: " + info.getComponents().keySet());
    }
  }

  protected void setInfo(final ServiceInfo info)
  {
    logger.debug("Set info. Status: " + info.getStatusStr() + " [" + info.getStatus() + ']');

    if (ServiceInfo.STATUS_RUNNING == info.status
        && (null == info.components || 0 == info.components.size())
        && null != this.info.components && 0 < this.info.components.size()) {
      info.components = this.info.components;
    }
    this.info = info;
  }
}