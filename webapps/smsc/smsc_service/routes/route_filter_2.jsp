<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.route.RouteFilter"%>
<%@ include file="/common/header_begin.jsp"%>
<%@ include file="/common/list_selector.jsp"%>
<%@ include file="utils.jsp"%><%
String action = request.getParameter("action");
if (action == null)
  throw new NullPointerException("Action not specified - Error in " + routesPrefix + "/route_filter.jsp");
if (action.equals("Clear filter"))
{
  session.setAttribute("route_filter", new RouteFilter());
  pageContext.forward(routesPrefix + "/route_filter.jsp");
}
else if (action.equals("Refresh"))
{
  pageContext.forward(routesPrefix + "/route_filter.jsp");
}
else if (action.equals("Apply filter"))
{
  set_trigger_by_request_param("route_show_source_adresses"     , false, session, request);
  set_trigger_by_request_param("route_show_destination_adresses", false, session, request);
  session.setAttribute("route_filter",
                       new RouteFilter(Boolean.valueOf(request.getParameter("intersection")).booleanValue(),
                                       parseSelected(request.getParameter("source_selected")),
                                       request.getParameter("src_masks"),
                                       parseSelected(request.getParameter("destination_selected")),
                                       request.getParameter("dst_masks"),
                                       request.getParameterValues("smes") == null ? new Vector() : Arrays.asList(request.getParameterValues("smes")),
                                       smsc.getSubjects()
                                       )
                       );
  session.setAttribute("route_page_size", Integer.decode(request.getParameter("pagesize")));
  response.sendRedirect(urlPrefix + routesPrefix + "/index.jsp");
}
else
  throw new Exception("Unknown action - Error in " + routesPrefix + "/route_filter.jsp");
%>