<%@ include file="/common/header.jsp"%>
<%@ include file="menu.jsp" %>
<%
Boolean show_source_adresses = (Boolean) session.getAttribute("show_source_adresses");
if (show_source_adresses == null)
  show_source_adresses = new Boolean(false);
String show_source_adresses_string = request.getParameter("show_source_adresses");
if (show_source_adresses_string != null)
  show_source_adresses = Boolean.valueOf(show_source_adresses_string);

session.setAttribute("show_source_adresses", show_source_adresses);
%>
  <h4>Routes</h4>
  <a href="route_filter.jsp">Filter</a> &nbsp;&nbsp;&nbsp; <a href="index.jsp?show_source_adresses=<%=show_source_adresses.booleanValue() ? "false\">Hide" : "true\">Show"%> sources</a>
	<table class="list" cellspacing="0">
    <thead>
			<tr class="list">
				<th class="list" width="50%">Route ID</th>
				<th class="list" nowrap>&nbsp;</th>
				<th class="list" nowrap><a href="edit_route.jsp">Add</a></th>
			</tr>
    </thead>
		<tbody>
      <% for (Iterator i = routeManager.getRoutes().iterator(); i.hasNext();) {
        Route route = (Route) i.next(); %>
  			<tr class="list">
  				<td class="list"><%=route.getName()%></td>
  				<td class="list" nowrap><font color="green"><b>--&gt;</b></font></td>
  				<td class="list" nowrap><a href="edit_route.jsp?name=<%=URLEncoder.encode(route.getName())%>">Edit</a> 
              <a href="delete_route.jsp?name=<%=URLEncoder.encode(route.getName())%>">Delete</a>
  				</td>
  			</tr>
      <%}%>
		</tbody>
	</table>
<%@ include file="/common/footer.jsp"%>