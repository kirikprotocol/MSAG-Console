<%@ include file="/common/header.jsp"%>
<%@ include file="menu.jsp" %>
<%@ include file="/common/list_selector.jsp"%>
<form method="post" action="route_filter_2.jsp">
	<table class="list" cellspacing="0">
		<tbody>
			<tr class="list">
				<th class="list" colspan="2">Route list options</th>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Sources</th>
				<td class="list">
					<table cellspacing="0" width="100%" border="1">
						<tbody>
							<tr>
								<td><%=createSelector("available_src", routeManager.getSubjects().getNames(), "selected_src", new HashSet(), "source_selected")%></td>
							</tr>
							<tr>
								<td colspan>
									<textarea style="WIDTH: 100%" name="src_masks" rows="5"></textarea>
								</td>
							</tr>
						</tbody>
					</table>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Destinations</th>
				<td class="list">
					<table cellspacing="0" width="100%" border="1">
						<tbody>
							<tr>
								<td><%=createSelector("available_dst", routeManager.getSubjects().getNames(), "selected_dst", new HashSet(), "destination_selected")%></td>
							</tr>
							<tr>
								<td colspan>
									<textarea style="WIDTH: 100%" name="dst_masks" rows="5"></textarea>
								</td>
							</tr>
						</tbody>
					</table>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">SME's</th>
				<td class="list">
          <% for (Iterator i=serviceManager.getServiceIds().iterator(); i.hasNext(); ) { 
            String sme_name = (String) i.next();
            %><input type="checkbox" checked value="sme_<%=sme_name%>" name="sme_<%=sme_name%>"><%=sme_name%><br>
          <%}%>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Items per page</th>
				<td class="list"><input value="20" name="pagesize"></td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Show info</th>
				<td class="list">
					<input type="checkbox" name="show_sources" value="1" disabled>Show sources list<br>
					<input type="checkbox" value="1" name="show_destinations" disabled>Show destinations list<br>
					<input type="checkbox" value="1" name="option1">Some other option
				</td>
			</tr>
			<tr class="list">
				<td class="list" colspan="2">
					<input type="submit" value="Clear filter">
					<input type="submit" value="Refresh">
				</td>
			</tr>
		</tbody>
	</table>
</form>
<%@ include file="/common/footer.jsp"%>