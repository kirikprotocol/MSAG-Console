<%@
page pageEncoding="windows-1251"%><%@
page contentType="text/html; charset=windows-1251"%><%@
page import="ru.novosoft.smsc.jsp.smsc.localeResources.localeResourcesResourceFile"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.localeResources.localeResourcesResourceFile"
/><jsp:setProperty name="bean" property="*"
/><%
ru.novosoft.smsc.jsp.SMSCAppContext appContext = (ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext");
bean.printLocaleText(appContext, out);
%>