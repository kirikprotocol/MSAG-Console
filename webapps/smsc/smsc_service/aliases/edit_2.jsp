<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<%
String address = request.getParameter("address");
String alias = request.getParameter("alias");
String old_address = request.getParameter("old_address");
String old_alias = request.getParameter("old_alias");
boolean old_hide = request.getParameter("old_hide") != null && request.getParameter("old_hide").equalsIgnoreCase("true");
boolean hide = request.getParameter("hide") != null && request.getParameter("hide").length() > 0;
boolean isEdit = old_address.length() > 0 && old_alias.length() > 0;
if (isEdit)
{
  smsc.getAliases().remove(new Alias(new Mask(old_address), new Mask(old_alias), old_hide));
}
smsc.getAliases().add(new Alias(new Mask(address), new Mask(alias), hide));
%>
<h4>Aliases</h4>
<%
if (isEdit)
{%>
<h5>Edit alias:</h5>
Alias "<%=old_alias%>&nbsp;-&gt;&nbsp;<%=old_address%>" 
changed to "<%=alias%>&nbsp;-&gt;&nbsp;<%=address%>" sucessfully.
<%} else {%>
<h5>Add alias:</h5>
Alias "<%=alias%>&nbsp;-&gt;&nbsp;<%=address%>" added sucessfully.
<%}%>
<%@ include file="/sketches/footer.jsp"%>