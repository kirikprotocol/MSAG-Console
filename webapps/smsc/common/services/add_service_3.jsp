<%@ page import="ru.novosoft.smsc.jsp.SMSCAppContext"%>
<%@ include file="/common/header.jsp"%>
<%
//  String service = request.getParameter("service") == null ? "" : request.getParameter("service");
  String host = request.getParameter("host") == null ? "" : request.getParameter("host");
  String portString = request.getParameter("port") == null ? "" : request.getParameter("port");
  String args = request.getParameter("args") == null ? "" : request.getParameter("args");
  String systemId = request.getParameter("system id") == null ? "" : request.getParameter("system id");

  String systemType = request.getParameter("system type") == null ? "" : request.getParameter("system type");
  String typeOfNumberString = request.getParameter("type of number") == null ? "" : request.getParameter("type of number");
  String numberingPlanString = request.getParameter("numbering plan") == null ? "" : request.getParameter("numbering plan");
  String interfaceVersionString = request.getParameter("interface version") == null ? "" : request.getParameter("interface version");
  String rangeOfAddress = request.getParameter("range of address") == null ? "" : request.getParameter("range of address");
  String smeNType = request.getParameter("sme ntype") == null ? "" : request.getParameter("sme ntype");

  if (/*service.equals("") || */host.equals("") || portString.equals("") || systemId.equals("") || systemType.equals("")
          || typeOfNumberString.equals("") || numberingPlanString.equals("") || interfaceVersionString.equals("")
          || rangeOfAddress.equals("") || smeNType.equals(""))
    throw new Exception("Not all parameters defined");

  int port = 0;
  try {
    port = Integer.decode(portString).intValue();
  } catch (NumberFormatException e) {
    throw new Exception("Port number misformatted: "+e.getMessage());
  }
  if (port == 0)
    throw new Exception("Port number not defined");

  int typeOfNumber = Integer.decode(typeOfNumberString).intValue();
  if (typeOfNumber < 0 || typeOfNumber > 6)
    throw new Exception("Unknown type of number");
  int numberingPlan = Integer.decode(numberingPlanString).intValue();
  if (numberingPlan != 0 && numberingPlan != 1 && numberingPlan != 3 && numberingPlan != 4 && numberingPlan != 6
          && numberingPlan != 8 && numberingPlan != 9 && numberingPlan != 10 && numberingPlan != 14
          && numberingPlan != 18)
    throw new Exception("Unknown numbering plan");
  if (!interfaceVersionString.equals("3.4"))
    throw new Exception("Incompatible interface version \"" + interfaceVersionString + "\". 3.4 version supported only");
  int interfaceVersion = 0x34;

  serviceManager.addService(/*service, */host, port, args, systemId, systemType, typeOfNumber,
                            numberingPlan, interfaceVersion, rangeOfAddress, smeNType);
%>
<h2>Add service <i><%=service%></i> to host <i><%=host%></i>: </h2>
Service <i><%=service%></i> added to host <i><%=host%></i> sucessfully.
<%@ include file="/common/footer.jsp"%>