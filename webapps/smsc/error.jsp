<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
%><%@ include file="/WEB-INF/inc/html_1_header.jsp"
%><%@page isErrorPage="TRUE"%><%
%><div class=content><div class=error><div class=error_header>Internal error occured</div><pre><%
  if (exception != null) {
    java.io.PrintWriter pw = new java.io.PrintWriter( out );
    exception.printStackTrace( pw );
  }
%></pre></div></div><%
%><%@ include file="/WEB-INF/inc/html_1_footer.jsp"%><%
%><%@ include file="/WEB-INF/inc/code_footer.jsp"%>