<%@ include file="/common/header.jsp"%>
<%@ include file="menu.jsp" %>
<%@ include file="/common/list_selector.jsp"%>
<% 
String old_name = request.getParameter("name"); 
if (old_name == null) old_name = "";
Route route = routeManager.getRoutes().get(old_name);
if (route == null) route = new Route(old_name);

Set available_src = new HashSet();
Set available_dst = new HashSet();
available_src.addAll(routeManager.getSubjects().getNames());
available_dst.addAll(routeManager.getSubjects().getNames());

SourceList selected_src = route.getSources();
Set masks_src = selected_src.getMaskNames();
available_src.removeAll(selected_src.getNames());

DestinationList selected_dst = route.getDestinations();
Set masks_dst = selected_dst.getMaskNames();
available_dst.removeAll(selected_dst.getNames());

%>
<form method="post" action="edit_route_2.jsp">
  <input type="Hidden" name="old_name" value="<%=StringEncoderDecoder.encode(old_name)%>">
  <h4>Edit Route</h4>
  <table class="list" cellspacing="0">
  	<thead>
  		<tr class="list">
  			<th class="list" width="50%">Route Name</th>
  			<td class="list" width="50%"><input name="name" style="WIDTH: 100%" value = "<%=StringEncoderDecoder.encode(old_name)%>"></td>
  		</tr>
  		<tr class="list">
  			<th class="list" nowrap width="50%">Sources</th>
  			<th class="list" nowrap width="50%">Destinations</th>
  		</tr>
    </thead>
    <tbody>
  		<tr class="list">
  			<td class="list" width="50%"><%=createSelector("available_src", available_src, "selected_src", selected_src.getSubjectNames(), "source_selected")%></td>
  			<td class="list" width="50%"><%=createSelector("available_dst", available_dst, "selected_dst", selected_dst.getSubjectNames(), "destination_selected")%></td>
  		</tr>
  		<tr class="list">
  			<td class="list" width="50%"><textarea rows="5" name="source_masks" style="width: 100%;"><%
          for (Iterator i = masks_src.iterator(); i.hasNext(); )
          {
            %><%=StringEncoderDecoder.encode((String)i.next())+'\n'%><%
          }
        %></textarea></td>
  			<td class="list" width="50%"><textarea rows="5" name="destination_masks" style="width: 100%;"><%
          for (Iterator i = masks_dst.iterator(); i.hasNext(); )
          {
            %><%=StringEncoderDecoder.encode((String)i.next())+'\n'%><%
          }
        %></textarea></td>
  		</tr>
  	</tbody>
  </table>
  <input type="Submit">
</form>
<%@ include file="/common/footer.jsp"%>