<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.subject.SubjectFilter"%>
<%@ include file="/common/header_begin.jsp"%>
<%@ include file="/common/list_selector.jsp"%>
<%
String action = request.getParameter("action");
if (action == null)
  throw new NullPointerException("Action not specified - Error in " + subjectsPrefix + "/route_filter.jsp");
if (action.equals("Clear filter"))
{
  session.setAttribute("subject_filter", new SubjectFilter());
  pageContext.forward(subjectsPrefix + "/filter.jsp");
}
else if (action.equals("Refresh"))
{
  pageContext.forward(subjectsPrefix + "/filter.jsp");
}
else if (action.equals("Apply filter"))
{
  session.setAttribute("subject_filter",
                       new SubjectFilter(parseSelected(request.getParameter("names_selected")),
                                       request.getParameterValues("smes") == null ? new Vector() : Arrays.asList(request.getParameterValues("smes")),
                                       request.getParameter("masks"))
                       );
  session.setAttribute("subject_page_size", Integer.decode(request.getParameter("pagesize")));
  response.sendRedirect(urlPrefix + subjectsPrefix + "/index.jsp");
}
else
  throw new Exception("Unknown action - Error in " + subjectsPrefix + "/filter.jsp");
%>