<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%@ page import = "ru.novosoft.smsc.admin.profiler.Profile"%>
<%
	String addressString = request.getParameter("address");
	if (addressString == null || addressString.length() == 0)
		throw new AdminException("Address parameter not defined");
	
	Mask address = new Mask(addressString);
	byte codePage = Byte.decode(request.getParameter("codepage")).byteValue();
	byte reportOptions = Byte.decode(request.getParameter("report")).byteValue();
	
	Profile profile = new Profile(codePage, reportOptions);
	smsc.updateProfile(address, profile);
%>
<h4>Profile: <%=addressString%></h4>
Changes applied
<table class="list" cellspacing="0">
	<tr class="list">
		<th class="list" width="33%">Report mode</th>
		<td class="list"><%
			switch(reportOptions)
			{
				case Profile.REPORT_OPTION_None:
					%>none<%
					break;
				case Profile.REPORT_OPTION_Full:
					%>full<%
					break;
				default:
					%><font color="#ff0000">unknown</font><%
			}
		%></td>
		</tr>
	<tr class="list">
		<th class="list">CodePage</th>
		<td class="list"><%
			switch(codePage)
			{
				case Profile.CODEPAGE_Default:
					%>Default<%
					break;
				case Profile.CODEPAGE_UCS2:
					%>UCS2<%
					break;
				default:
					%><font color="#ff0000">unknown</font><%
			}
		%></td>
	</tr>
</table>
<%@ include file="/common/footer.jsp"%>