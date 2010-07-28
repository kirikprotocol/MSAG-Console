<%@
 tag body-content="empty" %><%@
taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 attribute name="errorMessages" required="true"%>
<c:set var="errorMessage" value="${bean.traceMessage}"/><%

   if (errorMessages.size() > 0)
   {
     SMSCAppContext appContext = (SMSCAppContext) request.getAttribute("appContext");
   	%><div class=content><%
   	for(java.util.Iterator it = errorMessages.iterator(); it.hasNext();)
   	{
   		MessageException exc = (MessageException) it.next();
       exc.printStackTrace(new PrintStream(System.err));
   		String code = exc.getMessage();
      String nested = exc.getCause() == null ? "" : "<br>Nested: \"" + exc.getCause().getMessage() + "\" (" + exc.getCause().getClass().getName() + ")";
      String param = exc.getParam() == null ? "" : ": " + exc.getParam();
   		String msg = appContext.getLocaleString(appContext.getUserPreferences(request.getUserPrincipal()).getLocale(), code);
       if (msg == null) msg = code;

       String text = msg + param + nested;
       switch (exc.getErrorClass())
       {
         case SMSCJspException.ERROR_CLASS_ERROR:
           {%><div class=error><div class=error_header>Error:</div><%=text%></div><%}
           break;
         case SMSCJspException.ERROR_CLASS_WARNING:
           {%><div class=warning><div class=warning_header>Warning:</div><%=text%></div><%}
           break;
         case SMSCJspException.ERROR_CLASS_MESSAGE:
           {%><div class=message><div class=message_header>Attention!</div><%=text%></div><%}
           break;
         default:
           if(code.startsWith("error."))
             {%><div class=error><div class=error_header>Error:</div><%=text%></div><%}
           else if(code.startsWith("warning."))
             {%><div class=warning><div class=warning_header>Warning:</div><%=text%></div><%}
           else
             {%><div class=message><div class=message_header>Attention!</div><%=text%></div><%}
           break;
       }
   	}
   	%></div><%
   }


   if (exception != null && exception instanceof JspException) {
    JspException e = (JspException) exception;
    final Throwable root = e.getRootCause();
    if (root != null)
      exception = root;
  }
  if (exception != null && exception.getCause() != null)
    pageContext.setAttribute("cause", exception.getCause());

  if (exception != null) {
    //System.out.println("SMPP GW error page catch an exception:");
    final PrintWriter printWriter = new PrintWriter(System.out);
    exception.printStackTrace(printWriter);
    printWriter.flush();
  }

  if (exception instanceof SCAGJspException) {
    SCAGJspException e = (SCAGJspException) exception;
    pageContext.setAttribute("exception", e);
    String message = "Unknown exception";
    if (e.getCode() != null)
      message = LocaleMessages.getInstance().getMessage(request.getLocale(), e.getCode().getId());
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