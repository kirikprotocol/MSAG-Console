<%@ page import="ru.novosoft.smsc.jsp.SMSCAppContext,
				 java.net.URLEncoder,
				 ru.novosoft.util.jsp.MultipartServletRequest,
				 ru.novosoft.util.jsp.MultipartDataSource,
				 ru.novosoft.smsc.admin.service.*,
				 ru.novosoft.smsc.util.*"%>
<%
MultipartServletRequest multi = (MultipartServletRequest)request.getAttribute("multipart.request");
if (multi == null)
	throw new NullPointerException("File not attached");
request = (HttpServletRequest)multi;

%>
<%@ page isErrorPage="false"%>
<%@ page errorPage="/common/error.jsp"%>
<%@ include file="/common/header_begin.jsp"%>
<%

//ServiceManagerImpl hostsManager = ((SMSCAppContext)request.getAttribute("appContext")).getServiceManager();

AddAdmServiceWizard wizard = null;
MultipartDataSource dataFile = null;
try {
	dataFile = multi.getMultipartDataSource("distribute");
	if (dataFile == null)
		throw new Exception("Service distributive not attached");
	if( dataFile.getContentType().equals("application/x-zip-compressed") ) {
		java.io.InputStream is = dataFile.getInputStream();
		wizard = serviceManager.receiveNewServiceArchive(is);
		dataFile.close();
		dataFile = null;
	}
	else
		throw new Exception("Distributive file must be zip compressed");
} finally {
	if( dataFile != null ) {
		dataFile.close();
		dataFile = null;
	}
}
if (!serviceManager.getSmeIds().contains(wizard.getSystemId()))
{
	session.setAttribute("AddAdmServiceWizard", wizard);
	%><meta http-equiv="Refresh" content="0;url=<%=urlPrefix+servicesPrefix%>/add_adm_service_3.jsp"></head></html><%
} else {
	%><meta http-equiv="Refresh" content="0;url=<%=urlPrefix+servicesPrefix%>/add_adm_service.jsp?error=<%=StringEncoderDecoder.encode("SME with ID \"" + wizard.getSystemId() + "\" already presented in system")%><%
		if (request.getParameter("host") != null)
		{
			%>&host=<%=request.getParameter("host")%><%
		}
	%>"></head></html><%
}%>