<%@ page 
	import="ru.novosoft.smsc.jsp.smsc.localeResources.localeResourcesResourceFile"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.localeResources.localeResourcesResourceFile"
/><jsp:setProperty name="bean" property="*"
/><%bean.printLocaleText(out);%>