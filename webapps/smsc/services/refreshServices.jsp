<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="header.jsp"%>
<% 
	serviceManager.refreshServices();
%>
<h2>Services lists refreshed</h2>
<%@include file="footer.jsp"%>