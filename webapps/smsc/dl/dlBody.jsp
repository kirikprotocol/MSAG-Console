<div class=content>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Common values ~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>Distribution list</div>
<table class=properties_list cellspacing=0>
<thead>
<col width="15%" align=right>
<col width="85%">
</thead>
<tr class=row0>
	<th>name:</th>
	<td><input class=txt name=name value="<%=StringEncoderDecoder.encode(bean.getName())%>" <%=isEditing ? "readonly" : ""%>></td>
</tr>
<tr class=row1>
	<th>owner:</th>
	<td>system</td>
</tr>
<tr class=row0>
	<th>maximum elements:</th>
	<td><input class=txt name=maxElements value="<%=bean.getMaxElements()%>" <%=isEditing ? "readonly" : ""%>  validation="positive" onkeyup="resetValidation(this)"></td>
</tr>
</table>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Submitters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>Submitters</div>
<table class=list cellspacing=0 cellpadding=1 width="100%">
<thead>
<col width="15%">
<col width="85%">
</thead>
<%
for (int i = 0; i<bean.getSubmitters().length; i++)
{
	%><tr class=row0>
		<td colspan=2><input class=txt name=submitters value="<%=StringEncoderDecoder.encode(bean.getSubmitters()[i])%>" validation="address" onkeyup="resetValidation(this)"></td>
	</tr>
	<%
}
%>
<tr class=row0>
	<td><input class=txt name=submitters validation="address" onkeyup="resetValidation(this)"></td>
	<td><%addButton(out, "mbAdd", "Add", "Add new submitter");%></td>
</tr>
</table>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
<script>
function setSort(sorting)
{
	if (sorting == "<%=bean.getSort()%>")
		opForm.sort.value = "-<%=bean.getSort()%>";
	else
		opForm.sort.value = sorting;
	opForm.submit();
	return false;
}
</script>
<div class=page_subtitle>Members</div>
<table class=list cellspacing=0 cellpadding=1 width="100%">
<col width="15%">
<col width="85%">
<thead>
<tr>
	<th class=secInner colspan=2 style="border-bottom: 1px solid #888888;"><a href="#" <%=bean.getSort().endsWith("address") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by address" onclick='return setSort("address")'>Addresses</a></th>
</tr>
</thead>
<%
for (int i = 0; i<bean.getMembers().length; i++)
{
	%><tr class=row0>
		<td colspan=2><input class=txt name=members value="<%=StringEncoderDecoder.encode(bean.getMembers()[i])%>" validation="address" onkeyup="resetValidation(this)"></td>
	</tr>
	<%
}
%>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
<table class=list cellspacing=0 cellpadding=1 width="100%">
<thead>
<col width="15%" align=right>
<col width="85%">
</thead>
<tr class=row0>
	<td><input class=txt name=members validation="address" onkeyup="resetValidation(this)"></td>
	<td><%addButton(out, "mbAdd", "Add", "Add new member");%></td>
</tr>
</table>
</div>