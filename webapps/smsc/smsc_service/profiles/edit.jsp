<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%@ page import = "ru.novosoft.smsc.admin.profiler.Profile"%>
<h4>Profiles</h4>
<%
	String addressString = request.getParameter("Mask");
	boolean isAdd = false;
	if (addressString == null || addressString.length() == 0)
	{
		addressString = ".0.0.0";
		isAdd = true;
	}
	
	Mask address = new Mask(addressString);
	Profile profile = smsc.lookupProfile(address);
%>
<form method="post" action="update.jsp">
	<h2>Profile: <%=profile.getMask().getMask()%></h2>
	<table class="list" cellspacing="0">
		<tr class="list">
			<th class="list" width="33%">Mask</th>
			<td class="list" width="33%"><input type="text" name="mask" style="width:100%" value="<%=profile.getMask().getMask()%>"<%=isAdd ? "" : " readonly"%>></td>
		</tr>
		<tr class="list">
			<th class="list" width="33%">Report mode</th>
			<td class="list">
				<select name="report" style="width: 100%;">
					<option value="<%=Profile.REPORT_OPTION_None%>" <%=profile.getReportOptions() == Profile.REPORT_OPTION_None ? "SELECTED" : ""%>>none</option>
					<option value="<%=Profile.REPORT_OPTION_Full%>" <%=profile.getReportOptions() == Profile.REPORT_OPTION_Full ? "SELECTED" : ""%>>full</option>
				</select>
			</td>
			</tr>
		<tr class="list">
			<th class="list">CodePage</th>
			<td class="list">
				<select name="codepage" style="width: 100%;">
					<option value="<%=Profile.CODEPAGE_Default%>" <%=profile.getCodepage() == Profile.CODEPAGE_Default ? "SELECTED" : ""%>>Default</option>
					<option value="<%=Profile.CODEPAGE_UCS2   %>" <%=profile.getCodepage() == Profile.CODEPAGE_UCS2    ? "SELECTED" : ""%>>UCS2</option>
				</select>
			</td>
		</tr>
	</table>
	<input type="Submit" value="Apply changes">
</form>

<%@ include file="/common/footer.jsp"%>