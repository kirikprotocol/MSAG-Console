<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder"%><div class=content>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Common values ~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle><%=getLocString("dl.subTitle")%></div>
<table class=properties_list cellspacing=0>
<thead>
<col width="15%" align=right>
<col width="85%">
</thead>
<tr class=row0>
	<th><%=getLocString("dl.name")%>:</th>
	<td><input class=txt name=name value="<%=StringEncoderDecoder.encode(bean.getName())%>" <%=isEditing ? "readonly" : ""%>></td>
</tr>
<tr class=row1>
	<th><%=getLocString("dl.owner")%>:</th>
	<td><%
      if (bean.isCreate()) {
      %><input class=radio type=radio name=system id=systemTrue  value=true  onClick="checkSystemRadio();" <%=bean.isSystem() ? "checked" : ""%>><label for=systemTrue >&nbsp;<%=getLocString("dl.owner.system")%></label>
        <input class=radio type=radio name=system id=systemFalse value=false onClick="checkSystemRadio();" <%=bean.isSystem() ? "" : "checked"%>><label for=systemFalse>&nbsp;<input class=txt name=owner id=ownerAddress value="<%=StringEncoderDecoder.encode(bean.getOwner())%>" validation="address" onkeyup="resetValidation(this)"></label>
        <script>function checkSystemRadio() {opForm.all.ownerAddress.disabled = opForm.all.systemTrue.checked;}
        checkSystemRadio();</script><%
      } else {
        %><%=bean.isSystem() ? "SYSTEM" : StringEncoderDecoder.encode(bean.getOwner())%><%
      }
  %></td>
</tr>
<tr class=row0>
	<th><%=getLocString("dl.maxMembersCount")%>:</th>
	<td><input class=txt name=maxElements value="<%=bean.getMaxElements()%>" validation="positive" onkeyup="resetValidation(this)"></td>
</tr>
</table>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Submitters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle><%=getLocString("dl.submittersTitle")%></div>
<table class=list cellspacing=0 cellpadding=1 width="100%">
<thead>
<col width="15%">
<col width="85%">
</thead>
<%
for (int i = 0; i<bean.getSubmitters().length; i++)
{
	%><tr class=row0>
		<td colspan=2><input class=txt name=submitters value="<%=StringEncoderDecoder.encode(bean.getSubmitters()[i])%>" validation="address" onkeyup="resetValidation(this)"
        <%=isEditing && !bean.isSystem() && bean.getOwner().equals(bean.getSubmitters()[i]) ? "readonly" : ""%>></td>
	</tr>
	<%
}
%>
<tr class=row0>
	<td><input class=txt name=submitters validation="address" onkeyup="resetValidation(this)"></td>
	<td><%addButton(out, "mbAdd", "Add", "dl.addSubmitterHint");%></td>
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
<div class=page_subtitle><%=getLocString("dl.membersTitle")%></div>
<table class=list cellspacing=0 cellpadding=1 width="100%">
<col width="15%">
<col width="85%">
<thead>
<tr>
	<th class=secInner colspan=2 style="border-bottom: 1px solid #888888;">
    <a href="#" <%=bean.getSort().endsWith("address") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
     title="<%=getLocString("common.sortmodes.addressHint")%>" onclick='return setSort("address")'>
     <%=getLocString("common.sortmodes.address")%></a></th>
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
	<td><%addButton(out, "mbAdd", "Add", "dl.addMemberHint");%></td>
</tr>
</table>
</div>