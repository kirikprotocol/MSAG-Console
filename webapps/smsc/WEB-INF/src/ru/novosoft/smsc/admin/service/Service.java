/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:35:08 PM
 */
package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.protocol.CommandCall;
import ru.novosoft.smsc.admin.protocol.CommandListComponents;
import ru.novosoft.smsc.admin.protocol.Response;
import ru.novosoft.smsc.admin.utli.Proxy;
import ru.novosoft.smsc.util.xml.Utils;

import java.util.*;


public class Service extends Proxy
{
  private ServiceInfo info = null;
  private Category logger = Category.getInstance(this.getClass());

  public Service(ServiceInfo info)
  {
    super(info.getHost(), info.getPort());
    this.info = info;
  }

  public ServiceInfo getInfo()
  {
    return info;
  }

  /**
   * Вызывает метод на компоненте сервиса.
   * @param arguments Map поименованных аргументов (String -> Object). Аргументы могут быть
   * String Integer или Boolean класса.
   * @return Значение, которое вернул вызванный метод (String, Integer или Boolean)
   */
  public Object call(String componentId, String methodId, Type returnType, Map arguments) throws AdminException
  {
//    if (info.status != ServiceInfo.STATUS_RUNNING)
//      throw new AdminException("Service \"" + info.getId() + "\" is not running");

    refreshComponents();

    final Component component = (Component) info.getComponents().get(componentId);
    if (component == null)
      throw new AdminException("Service \"" + info.getId() + "\" is not connected");
    final Method method = (Method) component.getMethods().get(methodId);
    if (method == null)
      throw new AdminException("Version of service \"" + info.getId() + "\" and SMSC Administration Application is not compatible");

    if (method.equals(component.getMethods().get(method.getName()))) {
      Response r = runCommand(new CommandCall(info.getId(), component.getName(), method.getName(), returnType, arguments));
      if (r.getStatus() != Response.StatusOk)
        throw new AdminException("Error occured: " + r.getDataAsString());
      Element resultElem = (Element) r.getData().getElementsByTagName("variant").item(0);
      Type resultType = Type.getInstance(resultElem.getAttribute("type"));
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
      logger.error("Incorrect method \"" + (method == null ? "<null>" : method.getName()) + "\" signature");

      // for debug purposes
      try {
        if (component != null)
          logger.debug("Component: " + component.getName());
        if (method != null)
          logger.debug("Called method:" + "\n  name: " + method.getName() + "\n  type: " + method.getType().getName() + "\n  params: " + method.getParams());
        if (component != null && component.getMethods().get(method.getName()) != null) {
          Method foundMethod = (Method) component.getMethods().get(method.getName());
          logger.debug("Found method:" + "\n  name: " + foundMethod.getName() + "\n  type: " + foundMethod.getType().getName() + "\n  params: " + foundMethod.getParams());
        }
      } catch (Throwable e) {
      }

      throw new AdminException("Incorrect method \"" + (method == null ? "<null>" : method.getName()) + "\" signature");
    }
  }

  private List translateStringList(String listStr)
  {
    if (listStr == null || listStr.length() == 0)
      return new LinkedList();
    StringBuffer buffer = new StringBuffer(listStr.length());
    List result = new LinkedList();
    for (int i = 0; i < listStr.length(); i++) {
      char c = listStr.charAt(i);
      if (c == ',') {
        result.add(buffer.toString());
        buffer.setLength(0);
        continue;
      }
      if (c == '\\' && i < (listStr.length() - 1)) {
        c = listStr.charAt(++i);
      }
      buffer.append(c);
    }
    if (buffer.length() > 0)
      result.add(buffer.toString());
    else if (listStr.charAt(listStr.length() - 1) == ',')
      result.add(new String());

    return result;
  }

  private void refreshComponents() throws AdminException
  {
    if (info.getComponents().isEmpty()) {
      logger.debug("refreshComponents");
      Response r = runCommand(new CommandListComponents(info.getId()));
      if (r.getStatus() != Response.StatusOk)
        throw new AdminException("Error occured: " + r.getDataAsString());
      info.setComponents(r.getData().getDocumentElement());
      logger.debug("found " + info.getComponents().keySet().size() + " components: " + info.getComponents().keySet());
    }
  }

  protected void setInfo(ServiceInfo info)
  {
    logger.debug("Set info. Status: " + info.getStatusStr() + " [" + info.getStatus() + ']');

    if (info.status == ServiceInfo.STATUS_RUNNING
            && (info.components == null || info.components.size() == 0)
            && this.info.components != null && this.info.components.size() > 0) {
      info.components = this.info.components;
    }
    this.info = info;
  }
}