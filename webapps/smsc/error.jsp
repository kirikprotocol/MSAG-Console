<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
%><%@ include file="/WEB-INF/inc/html_1_header.jsp"
%><%@page isErrorPage="TRUE"%><%
%>
<span class=error>Internal error occured</span>
<pre>
<% 
java.io.PrintWriter pw = new java.io.PrintWriter( out );
exception.printStackTrace( pw ); 
%>
</pre><%
%><%@ include file="/WEB-INF/inc/html_1_footer.jsp"%><%
%><%@ include file="/WEB-INF/inc/code_footer.jsp"%>