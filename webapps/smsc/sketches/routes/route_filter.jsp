
	<%@ include file="/sketches/header.jsp" %>
	<table class="list" cellspacing="0">
		<tbody>
			<tr class="list">
				<th class="list" colspan="2">	Route list options
				</th>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">	Sources
				</th>
				<td class="list">
					<table cellspacing="0" width="100%" border="1">
						<tbody>
							<tr>
								<th>	Subjects
								</th>
							</tr>
							<tr>
								<td>
                  <% for (int i=1; i<=5; i++) { %>
                  <input type="checkbox" checked value="Subject <%=i%>" name="src_subj">Subject <%=i%><br>
                  <%}%>
								</td>
							</tr>
							<tr>
								<td colspan="2">
									<textarea style="WIDTH: 100%" name="src_masks" rows="5">asdasd</textarea>
								</td>
							</tr>
						</tbody>
					</table>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">	Destinations
				</th>
				<td class="list">
					<table cellspacing="0" width="100%" border="1">
						<tbody>
							<tr>
								<th>	Subjects
								</th>
							</tr>
							<tr>
								<td>
                  <% for (int i=1; i<=5; i++) { %>
									<input type="checkbox" checked value="Subject <%=i%>" name="dst_subj">Subject <%=i%><br>
                  <%}%>
								</td>
							</tr>
							<tr>
								<td colspan="2">
									<textarea style="WIDTH: 100%" name="dst_masks" rows="5">asdasd</textarea>
								</td>
							</tr>
						</tbody>
					</table>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">	SME's
				</th>
				<td class="list">
          <% for (int i=1; i<=3; i++) { %>
					<input type="checkbox" checked value="SME <%=i%>" name="sme">SME <%=i%><br>
          <%}%>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">	Items per page
				</th>
				<td class="list">
					<input value="20" name="pagesize">
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">	Show info
				</th>
				<td class="list">
					<input type="checkbox" value="1" name="show_sources">Show 
            sources list<br>
					<input type="checkbox" value="1" name="show_destinations">Show destinations list<br>
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
	<%@ include file="/sketches/footer.jsp" %>