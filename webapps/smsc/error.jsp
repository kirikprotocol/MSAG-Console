<%@
 page isErrorPage="TRUE"%><%@
 include file="/WEB-INF/inc/code_header.jsp"%><%@
 include file="/WEB-INF/inc/html_0_header.jsp"%><%@
 include file="/WEB-INF/inc/html_1_header.jsp"
%><div class=content><div class=error><div class=error_header><%=getLocString("common.errors.internal")%></div><pre><%
  if (exception != null) {
    java.io.PrintWriter pw = new java.io.PrintWriter( out );
    exception.printStackTrace( pw );
    exception.printStackTrace( new java.io.PrintStream(System.err) );
  }
%></pre></div></div><%@
 include file="/WEB-INF/inc/html_1_footer.jsp"%><%@
 include file="/WEB-INF/inc/code_footer.jsp"%>