<%@ page isErrorPage="false"%>
<%@ page errorPage="/common/error.jsp"%>
<%@ include file="/common/header_begin.jsp"%>
<%
AddAdmServiceWizard wizard = (AddAdmServiceWizard) session.getAttribute("AddAdmServiceWizard");
if (wizard == null)
{
	%><meta http-equiv="Refresh" content="0;url=<%=urlPrefix+servicesPrefix%>/add_adm_service.jsp"/></head></html><%
} else {
%>
<%@ include file="/common/header_end.jsp"%>
<h2>Add service <i><%=wizard.getSystemId()%></i> to host <i><%=wizard.getHost()%></i>: </h2>
<%
serviceManager.addAdmService(wizard);
session.removeAttribute("AddAdmServiceWizard");
%>
Service <i><%=wizard.getSystemId()%></i> added to host <i><%=wizard.getHost()%></i> sucessfully.
<%@ include file="/common/footer.jsp"%>
<%}%>