<%@include file="header.jsp"%>
<%! static final protected String paramHeader = "SMSC_METHOD_PARAMETER_";%>
<%
	String host = request.getParameter("host");
	String service = request.getParameter("service");
	String component = request.getParameter("component");
	String method = request.getParameter("method");
	String returnTypeName = request.getParameter("returnTypeName");

   Map arguments = new HashMap();
   for (Enumeration i = request.getParameterNames(); i.hasMoreElements(); )
   {
     String paramName = (String) i.nextElement();
     if (paramName.startsWith(paramHeader))
     {
       String param = request.getParameter(paramName);
       arguments.put(paramName.substring(paramHeader.length()), param);
     }
   }

	Object result = serviceManager.callServiceMethod(host, service, component,
                                  method, returnTypeName, arguments);

	%>
	<%=link("<h1>Host: \"" + host +"\"</h1>", "viewHost.jsp", "host", host)%>
	<%=link("<h2>Service: \"" + service + "\"</h2>", "viewService.jsp", "host", host, "service", service)%>
Result:<br><%
	if (result instanceof String) {
		out.print((String)result);
	} else if (result instanceof Long) {
		out.print(String.valueOf(((Long)result).longValue()));
	} else if (result instanceof Boolean) {
		out.print(String.valueOf(((Boolean)result).booleanValue()));
	} else {
		%><div style="color:Red">unknown type</div><%
	}
%>
<%@include file="footer.jsp"%>