<%@ include file="/common/header.jsp"%>
<% String host = request.getParameter("host") == null ? "" : request.getParameter("host");%>
<form method="post" enctype="multipart/form-data" action="<%=urlPrefix%>/upload">
<input type="hidden" name="jsp" value="<%=urlPrefix + servicesPrefix%>/add_service_2.jsp">
	<h2>Add service:</h2>
	<h3>hosting parameters:</h3>
	<table class="list">
		<tr class="list">
			<th class="list">Service name</th>
			<td class="list"><input name="service" value="" style="width: 100%;"></td></tr>
		<tr class="list">
			<th class="list">Host</th>
			<td class="list">
				<select name="host" style="width: 100%;">
          <%
          boolean isSelected = false;
          for (Iterator i = serviceManager.getHostNames().iterator(); i.hasNext(); )
          {
            String name = StringEncoder.encode((String) i.next());
            %><option value="<%=name%>"<%=isSelected ? "" : " selected"%>><%=name%></option><%
            isSelected = true;
          }
          %>
				</select>
			</td>
			<tr class="list">
				<th class="list">Port</th>
				<td class="list"><input name="port" value="" style="width: 100%;"></td>
			</tr>
			<tr class="list">
				<th class="list">Distribute</th>
				<td class="list"><input type="file" name="distribute" style="width: 100%;"></td>
			</tr>
			<tr class="list" title="maybe, it is not useful">
				<th class="list">Startup Arguments</th>
				<td class="list">
					<textarea cols="" rows="5" name="args" wrap="off" style="width: 100%;"></textarea>
				</td>
			</tr>
		</tr>
	</table>
	<div align="right">
		<input type=submit value="Next page">
	</div>
</form>
<%@ include file="/common/footer.jsp"%>