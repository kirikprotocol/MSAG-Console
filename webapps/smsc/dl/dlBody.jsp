<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Common values ~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<table class=list cellspacing=0 cellspadding=1 width="100%">
<thead>
<col width="15%" align=right>
<col width="85%">
<tr><th colspan=2 class=secInner><div class=secView>Distribution list</div></td></tr>
</thead>
<tr class=row0>
	<th class=label>name:</th>
	<td><input class=txt name=name value="<%=StringEncoderDecoder.encode(bean.getName())%>" <%=isEditing ? "readonly" : ""%>></td>
</tr>
<tr class=row1>
	<th class=label>owner:</th>
	<td>system</td>
</tr>
<tr class=row0>
	<th class=label>maximum elements:</th>
	<td><input class=txt name=maxElements value="<%=bean.getMaxElements()%>" <%=isEditing ? "readonly" : ""%>  validation="positive" onkeyup="resetValidation(this)"></td>
</tr>
</table>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Submitters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<table class=list cellspacing=0 cellpadding=1 width="100%">
<thead>
<col width="15%" align=right>
<col width="85%">
<tr><th colspan=2 class=secInner><div class=secView>Submitters</div></td></tr>
</thead>
<%
for (int i = 0; i<bean.getSubmitters().length; i++)
{
	%><tr class=row0>
		<th class=label>&nbsp;</th>
		<td><input class=txt name=submitters value="<%=StringEncoderDecoder.encode(bean.getSubmitters()[i])%>" validation="address" onkeyup="resetValidation(this)"></td>
	</tr>
	<%
}
%>
<tr class=row0>
	<th class=label><input class=btn type=submit name=mbAdd value="Add" title="Add new submitter"></th>
	<td><input class=txt name=submitters validation="address" onkeyup="resetValidation(this)"></td>
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
<table class=list cellspacing=0 cellpadding=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<thead>
<tr>
	<th class=secInner><div class=secView>Members</div></th>
	<th class=secInner style="border-bottom: 1px solid #888888;"><a href="#" <%=bean.getSort().endsWith("address") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by address" onclick='return setSort("address")'>Addresses</a></th>
</tr>
</thead>
<%
for (int i = 0; i<bean.getMembers().length; i++)
{
	%><tr class=row0>
		<th class=label>&nbsp;</th>
		<td><input class=txt name=members value="<%=StringEncoderDecoder.encode(bean.getMembers()[i])%>" validation="address" onkeyup="resetValidation(this)"></td>
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
	<th class=label><input class=btn type=submit name=mbAdd value="Add" title="Add new member"></th>
	<td><input class=txt name=members validation="address" onkeyup="resetValidation(this)"></td>
</tr>
</table>
