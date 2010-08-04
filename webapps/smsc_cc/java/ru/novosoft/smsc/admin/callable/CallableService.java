package ru.novosoft.smsc.admin.callable;

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.protocol.Proxy;
import ru.novosoft.smsc.admin.protocol.Response;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.util.XmlUtils;

import java.util.*;


/**
 * Класс описывает абстрактный сервис, с которым можно взаимодействовать по коммандному протоколу.
 * Конкретные сервисы должны наследоваться от данного класса.
 */
public abstract class CallableService {

  private static final Category logger = Category.getInstance(CallableService.class);

  private final ServiceManager serviceManager;
  private final String serviceId;
  private final int port;
  private final int iotimeout;

  private Map<String, Component> components;
  private Proxy proxy;

  public CallableService(ServiceManager serviceManager, String serviceId, int port, int iotimeout) {
    this.serviceManager = serviceManager;
    this.serviceId = serviceId;
    this.port = port;
    this.iotimeout = iotimeout;
  }

  private Proxy getProxy() throws AdminException {
    if (proxy == null) {
      ServiceInfo info = serviceManager.getService(serviceId);
      if (info == null)
        throw new CallableServiceException("service_not_found", serviceId);
      if (info.getOnlineHost() == null)
        throw new CallableServiceException("service_offline", serviceId);
      proxy = new Proxy(info.getOnlineHost(), port, iotimeout);
    }
    return proxy;
  }

  private Map<String, Component> getComponents(Proxy proxy) throws AdminException {
    if (components == null) {
      final Response r = proxy.runCommand(new CommandListComponents(serviceId));
      if (Response.StatusOk != r.getStatus())
        throw new CallableServiceException("error_returned", r.getDataAsString());
      final NodeList list = r.getData().getDocumentElement().getElementsByTagName("component");
      components = new HashMap<String, Component>();
      for (int i = 0; i < list.getLength(); i++) {
        final Element compElem = (Element) list.item(i);
        final Component c = new Component(compElem);
        components.put(c.getName(), c);
      }
    }

    return components;
  }

  /**
   * Вызывает определенный метод на сервисе
   *
   * @param componentId название компоненты внутри сервиса
   * @param methodId    название метода
   * @param returnType  тип возвращаемого значения
   * @param arguments   список аргументов (название, значение)
   * @return экземпляр типа, указанного в returnType
   * @throws AdminException если произошла ошибка при обращении к сервису
   */
  public Object call(final String componentId, final String methodId, final Type returnType, final Map<String, Object> arguments) throws AdminException {

    Proxy proxy = getProxy();

    Map<String, Component> components = getComponents(proxy);

    final Component component = components.get(componentId);
    if (null == component)
      throw new CallableServiceException("service_offline", serviceId);
    final Method method = (Method) component.getMethods().get(methodId);
    if (null == method)
      throw new CallableServiceException("incompartible_service_version", serviceId);

    if (method.equals(component.getMethods().get(method.getName()))) {
      final Response r = proxy.runCommand(new CommandCall(serviceId, component.getName(), method.getName(), returnType, arguments));
      if (Response.StatusOk != r.getStatus())
        throw new CallableServiceException("error_returned", r.getDataAsString());
      final Element resultElem = (Element) r.getData().getElementsByTagName("variant").item(0);
      final Type resultType = Type.getInstance(resultElem.getAttribute("type"));
      switch (resultType.getId()) {
        case Type.StringType:
          return XmlUtils.getNodeText(resultElem);
        case Type.IntType:
          return Long.decode(XmlUtils.getNodeText(resultElem));
        case Type.BooleanType:
          return Boolean.valueOf(XmlUtils.getNodeText(resultElem));
        case Type.StringListType:
          return translateStringList(XmlUtils.getNodeText(resultElem));
        default:
          throw new CallableServiceException("invalid_response", serviceId);
      }
    } else {
      logger.error("Incorrect method \"" + method.getName() + "\" signature");

      // for debug purposes
      try {
          logger.debug("Component: " + component.getName());
          logger.debug("Called method:" + "\n  name: " + method.getName() + "\n  type: " + method.getType().getName() + "\n  params: " + method.getParams());
        if ( null != component.getMethods().get(method.getName())) {
          final Method foundMethod = (Method) component.getMethods().get(method.getName());
          logger.debug("Found method:" + "\n  name: " + foundMethod.getName() + "\n  type: " + foundMethod.getType().getName() + "\n  params: "
              + foundMethod.getParams());
        }
      } catch (Exception e) {
      }

      throw new CallableServiceException("incompartible_service_version", serviceId);
    }
  }

  private static List<String> translateStringList(final String listStr) {
    if (null == listStr || 0 == listStr.length())
      return new ArrayList<String>();

    final StringBuilder buffer = new StringBuilder(listStr.length());

    final List<String> result = new ArrayList<String>();
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