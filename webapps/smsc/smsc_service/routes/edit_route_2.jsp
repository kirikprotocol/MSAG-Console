<%@ include file="/common/header.jsp"%>
<%@ include file="/common/list_selector.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
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

String showDestinations(Route route, Collection smes)
{
  String result = "<table border=\"0\" cellspacing=\"0\">";
  for (Iterator i = route.getDestinations().iterator(); i.hasNext();)
  {
    Destination d = (Destination) i.next();
    result += "<tr><td>" + (d.isSubject() ? "<font size=\"-2\">subj</font> " : "<font size=\"-2\">mask</font> ") + d.getName()
           +  "</td><td>" + show_set_combo(d.getName(), smes, d.getDefaultSmeId())+"</td></tr>";
  }
  result += "</table>";
  return result;
}
%><%
Collection serviceIDs = serviceManager.getSmeIds();
if (serviceIDs.isEmpty())
  throw new NullPointerException("SME list is empty");

SME defaultSme = smsc.getSmes().get((String)serviceIDs.iterator().next());

String old_route_name = request.getParameter("old_name");
String new_route_name = request.getParameter("name");
String source_selected_string = request.getParameter("source_selected");
String destination_selected_string = request.getParameter("destination_selected");
String source_masks = request.getParameter("source_masks");
String destination_masks = request.getParameter("destination_masks");
String isEnabling = request.getParameter("isEnabling");
String isBilling = request.getParameter("isBilling");
String isArchiving = request.getParameter("isArchiving");
RouteList routeList = smsc.getRoutes();
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

route.updateSources(parseSelected(source_selected_string), source_masks, smsc.getSubjects());
route.updateDestinations(parseSelected(destination_selected_string), destination_masks, smsc.getSubjects(), defaultSme);
route.setEnabling(isEnabling != null && isEnabling.equals("true"));
route.setBilling(isBilling != null && isBilling.equals("true"));
route.setArchiving(isArchiving != null && isArchiving.equals("true"));
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
  			<td class="list" width="50%"><%=showDestinations(route, serviceManager.getSmeIds())%></td>
  		</tr>
  	</tbody>
  </table>
  <input type="Submit">
</form>
<%@ include file="/common/footer.jsp"%>