<%@ include file="/common/header.jsp"%>
<%@ include file="menu.jsp" %>
<%@ include file="utils.jsp" %>
<%
boolean show_source_adresses = process_trigger("show_source_adresses", session, request);
boolean show_destination_adresses = process_trigger("show_destination_adresses", session, request);
%>
  <h4>Routes</h4>
  <a href="route_filter.jsp">Filter</a> &nbsp;&nbsp;&nbsp; 
  <%=switch_trigger("index.jsp?", "show_source_adresses", "Show sources", "Hide sources", session, request)%>
  <%=switch_trigger("index.jsp?", "show_destination_adresses", "Show destinations", "Hide destinations", session, request)%>
	<table class="list" cellspacing="0">
    <thead>
			<tr class="list">
				<th class="list" width="99%">Route ID</th>
				<th class="list" nowrap width="1%"><a href="edit_route.jsp">Add</a></th>
			</tr>
    </thead>
		<tbody>
      <% for (Iterator i = routeManager.getRoutes().iterator(); i.hasNext();) {
        Route route = (Route) i.next(); %>
  			<tr class="list">
  				<td class="list" width="99%">
            <font color="<%=route.isEnabling() ? "green" : "red"%>"><%=route.getName()%></font>
            <%
            if (show_source_adresses) {
              %><br><font size="-2"><%=show_sources(route)%></font><%
            }
            if (show_destination_adresses) {
              %><br><font size="-2"><%=show_destinations(route)%></font><%
            }
            %>
          </td>
  				<td class="list" nowrap width="1%"><a href="edit_route.jsp?name=<%=URLEncoder.encode(route.getName())%>">Edit</a>&nbsp;<a href="delete_route.jsp?name=<%=URLEncoder.encode(route.getName())%>">Delete</a></td>
  			</tr>
      <%}%>
		</tbody>
	</table>
<%@ include file="/common/footer.jsp"%>