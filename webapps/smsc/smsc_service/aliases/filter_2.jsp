<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.AliasFilter"%>
<%@ include file="/common/header_begin.jsp"%>
<%
String action = request.getParameter("action");
if (action == null)
  throw new NullPointerException("Action not specified - Error in " + aliasesPrefix + "/route_filter.jsp");
if (action.equals("Clear filter"))
{
  session.setAttribute("alias_filter", new AliasFilter());
  pageContext.forward(aliasesPrefix + "/filter.jsp");
} 
else if (action.equals("Refresh"))
{
  pageContext.forward(aliasesPrefix + "/filter.jsp");
} 
else if (action.equals("Apply filter"))
{
  session.setAttribute("alias_filter",
                       new AliasFilter(request.getParameter("aliases"),
                                       request.getParameter("addresses"))
                       );
  session.setAttribute("alias_page_size", Integer.decode(request.getParameter("pagesize")));
  response.sendRedirect(urlPrefix + aliasesPrefix + "/index.jsp");
} 
else
  throw new Exception("Unknown action - Error in " + aliasesPrefix + "/route_filter.jsp");
%>