<%@ page isErrorPage="false"%>
<%@ page errorPage="/common/error.jsp"%>
<%@ include file="/common/header_begin.jsp"%>
<%
AddNonAdmServiceWizard wizard = (AddNonAdmServiceWizard) session.getAttribute("AddNonAdmServiceWizard");
if (wizard == null)
{
	%><meta http-equiv="Refresh" content="0;url=<%=urlPrefix+servicesPrefix%>/add_non_service.jsp"/></head></html><%
} else {
%>
<%@ include file="/common/header_end.jsp"%>
<h2>Add service <i><%=wizard.getSystemId()%></i> : </h2>
<%
serviceManager.addNonAdmService(wizard);
session.removeAttribute("AddAdmServiceWizard");
%>
Service <i><%=wizard.getSystemId()%></i> added sucessfully.<br>
<a href="<%=urlPrefix+smscPrefix%>/apply_routes.jsp">Restart SMSC to apply new sme's config now</a> or <a href="<%=urlPrefix%>/">restart SMSC later</a>.
<%@ include file="/common/footer.jsp"%>
<%}%>