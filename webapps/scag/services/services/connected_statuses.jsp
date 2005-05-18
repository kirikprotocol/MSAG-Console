<%@ page import="java.util.Iterator"%>
<%@ page contentType="text/html;charset=UTF-8" language="java" %>
<jsp:useBean id="bean" class="ru.sibinco.scag.beans.services.services.Index"
/><%@ include file = "/WEB-INF/inc/show_sme_status.jsp"
%><%
/*try
{*/
 bean.process(request,response);
 //SCAGAppContext appContext= (SCAGAppContext) request.getAttribute("appContext");
 SCAGAppContext appContext=bean.getAppContext();
//System.out.println("*** connected_statuses.jsp called");
  //out.println();
	SmeStatus status = null;
  System.out.println("connected_statuses.jsp running");
  for (Iterator i = bean.getSmeIds().iterator(); i.hasNext(); )
{
	out.print('"' + (String) i.next() + '"');
	if (i.hasNext())
		out.print(", ");
}
//System.out.println("*** connected_statuses.jsp called");
out.println();
  for (Iterator i = bean.getSmeIds().iterator(); i.hasNext(); )
 {
 	String smeId = (String) i.next();
  status = appContext.getGateway().getSmeStatus(smeId);
  //  status = bean.getSmeStatus(sId);
 	out.print("<span " + showSmeStatus(status));
 	if (i.hasNext())
 		out.print(", ");
 }

/*} catch (Throwable t)
{
	t.printStackTrace(new java.io.PrintWriter(out));
}*/
%>