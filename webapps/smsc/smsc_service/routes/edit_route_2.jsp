<%@ include file="/common/header.jsp"%>
<%@ include file="menu.jsp" %>
<%@ include file="utils.jsp"%>
<%! 
String showSources(Route route)
{
  String result = "";
  for (Iterator i = route.getSources().iterator(); i.hasNext();)
  {
    Source s = (Source) i.next();
    result += (s.isSubject() ? "<font size=\"-2\">subj</font> " : "<font size=\"-2\">mask</font> ") + s.getName() + "<br>";
  }
  
  return result;
}

String showDestinations(Route route, Set smes)
{
  String result = "";
  for (Iterator i = route.getDestinations().iterator(); i.hasNext();)
  {
    Destination d = (Destination) i.next();
    result += (d.isSubject() ? "<font size=\"-2\">subj</font> " : "<font size=\"-2\">mask</font> ") + d.getName()
           +  show_set_combo(d.getName(), smes)+"<br>";
  }
  return result;
}
%><%
Set serviceIDs = serviceManager.getServiceIds();
if (serviceIDs.isEmpty())
  throw new NullPointerException("SME list is empty");

SME defaultSme = new SME((String)serviceIDs.iterator().next());

String old_route_name = request.getParameter("old_name");
String new_route_name = request.getParameter("name");
String source_selected_string = request.getParameter("source_selected");
String destination_selected_string = request.getParameter("destination_selected");
String source_masks = request.getParameter("source_masks");
String destination_masks = request.getParameter("destination_masks");
RouteList routeList = routeManager.getRoutes();
Route route = routeList.get(new_route_name);

if (!old_route_name.equals(new_route_name))
{
  route = routeList.remove(old_route_name);
  if (route == null)
    route = new Route(new_route_name);
  else
    route.setName(new_route_name);
  routeList.put(route);
}

route.updateSources(source_selected_string, source_masks, routeManager.getSubjects());
route.updateDestinations(destination_selected_string, destination_masks, routeManager.getSubjects(), defaultSme);

%>
<form method="post" action="edit_route_3.jsp">
  <h4>Edit Route</h4>
  <table class="list" cellspacing="0">
  	<thead>
  		<tr class="list">
  			<th class="list" width="50%">Route Name</th>
  			<td class="list" width="50%"><input type="text" name="name" value="<%=StringEncoderDecoder.encode(new_route_name)%>" readonly style="WIDTH: 100%"></td>
  		</tr>
  		<tr class="list">
  			<th class="list" nowrap width="50%">Sources</th>
  			<th class="list" nowrap width="50%">Destinations</th>
  		</tr>
    </thead>
    <tbody>
  		<tr class="list">
  			<td class="list" width="50%"><%=showSources(route)%></td>
  			<td class="list" width="50%"><%=showDestinations(route, serviceManager.getServiceIds())%></td>
  		</tr>
  	</tbody>
  </table>
  <input type="Submit">
</form>
<%@ include file="/common/footer.jsp"%>