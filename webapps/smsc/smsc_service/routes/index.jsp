<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%@ include file="/common/tables.jsp"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.RouteQuery"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.RouteFilter"%>
<%@ page import = "ru.novosoft.smsc.jsp.util.tables.QueryResultSet"%>
<%@ include file="utils.jsp" %>
<%
boolean show_source_adresses = process_trigger("route_show_source_adresses", session, request);
boolean show_destination_adresses = process_trigger("route_show_destination_adresses", session, request);
String sort = request.getParameter("sort");
if (sort == null)
  sort = "Route ID";
Vector sortOrder = new Vector();
sortOrder.add(sort);
RouteFilter route_filter = (RouteFilter) session.getAttribute("route_filter");
if (route_filter == null)
  route_filter = new RouteFilter();

Integer pagesizeI = (Integer) session.getAttribute("route_page_size");
int pagesize = pagesizeI == null ? 20 : pagesizeI.intValue();

QueryResultSet results = smsc.getRoutes().query(new RouteQuery(pagesize, route_filter, sortOrder, 0));
%>
  <h4>Routes</h4><!--a href="show_smsc_data.jsp">Show SMSC data</a-->
  <a href="route_filter.jsp">Filter</a> &nbsp;&nbsp;&nbsp; 
  <%=switch_trigger("index.jsp?", "route_show_source_adresses", "Show sources", "Hide sources", session, request)%>
  <%=switch_trigger("index.jsp?", "route_show_destination_adresses", "Show destinations", "Hide destinations", session, request)%>
	<table class="list" cellspacing="0">
    <thead>
			<tr class="list">
				<th class="list" width="99%"><a href="?sort=<%=sort.startsWith("-") ? URLEncoder.encode(sort.substring(1)) : "-" + URLEncoder.encode(sort)%>">Route ID</a></th>
				<th class="list" nowrap width="1%"><a href="edit_route.jsp">Add</a></th>
			</tr>
    </thead>
		<tbody>
      <% 
      for (Iterator i = results.iterator(); i.hasNext();) {
        DataItem routeItem = (DataItem) i.next();
        String routeId = (String) routeItem.getValue("Route ID");
        SourceList sources = (SourceList) routeItem.getValue("sources");
        DestinationList destinations = (DestinationList) routeItem.getValue("destinations");
        boolean isEnabling  = ((Boolean)routeItem.getValue("isEnabling" )).booleanValue();
        boolean isBilling   = ((Boolean)routeItem.getValue("isBilling"  )).booleanValue();
        boolean isArchiving = ((Boolean)routeItem.getValue("isArchiving")).booleanValue();

        %>
  			<tr class="list">
  				<td class="list" width="99%">
            <font color="<%=isEnabling ? "green" : "red"%>"><%=routeId%></font>
            <%
            if (show_source_adresses) {
              %><br><font style="font-size: small;"><%=show_sources(sources)%></font><%
            }
            if (show_destination_adresses) {
              %><br><font style="font-size: small;"><%=show_destinations(destinations)%></font><%
            }
            %>
          </td>
  				<td class="list" nowrap width="1%"><a href="edit_route.jsp?name=<%=URLEncoder.encode(routeId)%>">Edit</a>&nbsp;<a href="delete_route.jsp?name=<%=URLEncoder.encode(routeId)%>">Delete</a></td>
  			</tr>
      <%}%>
		</tbody>
	</table>
<%@ include file="/common/footer.jsp"%>