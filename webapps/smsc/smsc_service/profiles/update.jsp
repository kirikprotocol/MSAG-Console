<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%@ page import = "ru.novosoft.smsc.admin.profiler.Profile"%>
<%
	String addressString = request.getParameter("mask");
	if (addressString == null || addressString.length() == 0)
		throw new AdminException("Mask parameter not defined");
	
	Mask address = new Mask(addressString);
	byte codePage = Byte.decode(request.getParameter("codepage")).byteValue();
	byte reportOptions = Byte.decode(request.getParameter("report")).byteValue();
	
	Profile profile = new Profile(address, codePage, reportOptions);
	smsc.updateProfile(address, profile);
%>
<h4>Profile: <%=profile.getMask().getMask()%></h4>
Changes applied
<table class="list" cellspacing="0">
	<tr class="list">
		<th class="list" width="33%">Mask</th>
		<td class="list" width="33%"><%=profile.getMask().getMask()%></td>
	</tr>
	<tr class="list">
		<th class="list" width="33%">Report mode</th>
		<td class="list" width="33%"><%=profile.getReportOptionsString()%></td>
	</tr>
	<tr class="list">
		<th class="list">CodePage</th>
		<td class="list"><%=profile.getCodepageString()%></td>
	</tr>
</table>
<%@ include file="/common/footer.jsp"%>