	<%@ include file="/sketches/header.jsp" %>
  <%@ include file="menu.jsp"%>
  <h4>Routes</h4>
  <a href="route_filter.jsp">Filter</a> &nbsp;&nbsp;&nbsp; <a href="index.jsp">Hide addresses</a>
	<table class="list" cellspacing="0">
		<tbody>
			<tr class="list">
				<th class="list" width="50%">	Route ID
				</th>
				<th class="list" nowrap>	&nbsp;
				</th>
				<th class="list" nowrap><a href="add_route.jsp">Add</a>
				</th>
			</tr>
			<tr class="list">
				<td class="list">
          Email SME outbound route<br>
          <font size="-2">
            Sources: <b>A, C, F</b><br>
            Destinations: <b>Destination1, destination2, +7902918*</b>
          </font>
        </td>
				<td class="list" nowrap><font color="green"><b>--&gt;</b></font>
				</td>
				<td class="list" nowrap><a href="edit_route.jsp?id=1">Edit</a> 
            <a href="delete_route.jsp?id=1">Delete</a>
				</td>
			</tr>
			<tr class="list">
				<td class="list">
          Email SME in<br>
          <font size="-2">
            Sources: <b>Mobile abonents, , Some other address, +123456789????</b><br>
            Destinations: <b>+7902916*, +5789987????</b>
          </font>
				</td>
				<td class="list" nowrap><font color="red"><b>X<b></font></b></b>
				</td>
				<td class="list" nowrap><a href="edit_route.jsp?id=1">Edit</a> 
            <a href="delete_route.jsp?id=1">Delete</a>
				</td>
			</tr>
		</tbody>
	</table>
	<%@ include file="/sketches/footer.jsp" %>