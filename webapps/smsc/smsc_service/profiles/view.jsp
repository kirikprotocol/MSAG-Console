<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%@ page import = "ru.novosoft.smsc.admin.profiler.Profile"%>
<h4>Profiles</h4>
<%
	String addressString = request.getParameter("address");
	if (addressString == null || addressString.length() == 0)
		throw new AdminException("Address parameter not defined");
	
	Mask address = new Mask(addressString);
	Profile profile = smsc.lookupProfile(address);
	byte codePage = profile.getCodepage();
	byte reportOptions = profile.getReportOptions();
%>
<form method="post" action="update.jsp">
	<input type="Hidden" name="address" value="<%=addressString%>">
	<h2>Profile: <%=addressString%></h2>
	<table class="list" cellspacing="0">
		<tr class="list">
			<th class="list" width="33%">Report mode</th>
			<td class="list">
				<select name="report" style="width: 100%;">
					<option value="<%=Profile.REPORT_OPTION_None%>" <%=reportOptions == Profile.REPORT_OPTION_None ? "SELECTED" : ""%>>none</option>
					<option value="<%=Profile.REPORT_OPTION_Full%>" <%=reportOptions == Profile.REPORT_OPTION_Full ? "SELECTED" : ""%>>full</option>
				</select>
			</td>
			</tr>
		<tr class="list">
			<th class="list">CodePage</th>
			<td class="list">
				<select name="codepage" style="width: 100%;">
					<option value="<%=Profile.CODEPAGE_Default%>" <%=codePage == Profile.CODEPAGE_Default ? "SELECTED" : ""%>>Default</option>
					<option value="<%=Profile.CODEPAGE_UCS2   %>" <%=codePage == Profile.CODEPAGE_UCS2    ? "SELECTED" : ""%>>UCS2</option>
				</select>
			</td>
		</tr>
	</table>
	<input type="Submit" value="Apply changes">
</form>

<%@ include file="/common/footer.jsp"%>