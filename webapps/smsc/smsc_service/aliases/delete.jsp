<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<h4>Aliases</h4>
<h5>Delete alias:</h5>
<%
String address = request.getParameter("Address");
String alias = request.getParameter("Alias");
if (smsc.getAliases().remove(new Alias(new Mask(address), new Mask(alias))))
{
  %>Alias "<%=alias%>&nbsp;-&gt;&nbsp;<%=address%>" deleted sucessfully.<%
} else {
  %>Alias "<%=alias%>&nbsp;-&gt;&nbsp;<%=address%>" not found.<%
}
%>
<%@ include file="/common/footer.jsp"%>