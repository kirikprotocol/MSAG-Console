	<%@ include file="/sketches/header.jsp" %>
	<%@ include file="menu.jsp" %>
  <h4>Edit Route</h4>
	<table class="list" cellspacing="0">
		<tbody>
			<tr class="list">
				<th class="list">Route Name</th>
				<td class="list"><input style="WIDTH: 100%" value="Email SME outbound route"></td>
			</tr>
			<tr class="list">
				<th class="list" nowrap>Sources <a href="add_recipient.jsp">Edit</a></th>
				<th class="list" nowrap>Destinations <a href="add_recipient.jsp?recipient=dest">Edit</a></th>
			</tr>
			<tr class="list">
				<td class="list">A<br>C<br>F</td>
				<td class="list">B (SME1)<br>D (SME2)<br>E (SME3)<br>+7902985 (SME1)</td>
			</tr>
		</tbody>
	</table>
	<%@ include file="/sketches/footer.jsp" %>