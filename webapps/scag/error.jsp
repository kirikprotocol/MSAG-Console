<%@ page import="java.io.PrintWriter,
                 java.io.StringWriter,
                 ru.sibinco.lib.SibincoException,
                 ru.sibinco.scag.beans.SCAGJspException,
                 ru.sibinco.scag.util.LocaleMessages,
                 java.util.Locale,
                 java.security.Principal"%>
<%@page isErrorPage="true"%>
<%@include file="/WEB-INF/inc/header.jspf"%>
<%
  if (exception != null && exception instanceof JspException) {
    JspException e = (JspException) exception;
    final Throwable root = e.getRootCause();
    if (root != null)
      exception = root;
  }
  if (exception != null && exception.getCause() != null)
    pageContext.setAttribute("cause", exception.getCause());

  if (exception != null) {
    //System.out.println("SCA GW error page catch an exception:");
    final PrintWriter printWriter = new PrintWriter(System.out);
    exception.printStackTrace(printWriter);
    printWriter.flush();
  }

  if (exception instanceof SCAGJspException) {
    SCAGJspException e = (SCAGJspException) exception;
    pageContext.setAttribute("exception", e);
    String message = "Unknown exception";
    if (e.getCode() != null)
      message = LocaleMessages.getInstance().getMessage(session, e.getCode().getId());
    if (message == null)
      message = e.getMessage();
    pageContext.setAttribute("e_message", message);
    pageContext.setAttribute("e_info", e.getAdditionalInfo());
    %>
    <sm:page title="Error">
      <div class=error>
        <div class=header>${e_message}</div>
        ${e_info}
        <c:if test="${!empty cause}"><div class=header>Nested exception:</div>${cause.message}</c:if>
      </div>
    </sm:page><%
  } else if (exception instanceof SibincoException) {
    %>
    <sm:page title="Error">
      <div class=error><div class=header>Internal exception occured</div>
        ${exception.message}
        <c:if test="${!empty cause}"><div class=header>Nested exception:</div>${cause.message}</c:if>
      </div>
    </sm:page><%
  } else {
    StringWriter trace = new StringWriter();
    exception.printStackTrace(new PrintWriter(trace));
    pageContext.setAttribute("trace", trace);
    if (exception.getCause() != null) {
      StringWriter cause = new StringWriter();
      exception.getCause().printStackTrace(new PrintWriter(cause));
      pageContext.setAttribute("causeTrace", cause.toString());
    }
    %>
    <sm:page title="Error">
      <div class=error><div class=header>Unexpected exception occured</div>
        ${exception.message}
        <pre>${trace}</pre>
        <c:if test="${!empty cause}"><div class=header>Nested exception:</div>
          ${cause.message}
          <pre>${causeTrace}</pre>
        </c:if>
      </div>
    </sm:page><%
  }
%>