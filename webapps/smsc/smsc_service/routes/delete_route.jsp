<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%@ include file="utils.jsp" %>
<%
String name = request.getParameter("name"); 
if (name == null) 
  throw new NullPointerException("Route not defined");
Route route = smsc.getRoutes().get(name);
if (route == null) 
  throw new NullPointerException("Route not found");

smsc.getRoutes().remove(name);
%>
<h4>Route <i><%=StringEncoderDecoder.encode(name)%></i> deleted.</h4>
<%@ include file="/common/footer.jsp"%>