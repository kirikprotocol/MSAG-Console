<div class=content>
<div class=page_subtitle><%=getLocString("aliases.infoSubTitle")%></div>
<table class=list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="85%">
<tr class=row0>
	<td><%=getLocString("common.util.alias")%>:</td>
	<td><input class=txt name=alias value="<%=bean.getAlias()%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row1>
	<td><%=getLocString("common.util.address")%>:</td>
	<td><input class=txt name=address value="<%=bean.getAddress()%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row0>
	<td>&nbsp;</td>
	<td><input class=check type=checkbox id=hide name=hide <%=bean.isHide() ? "checked" : ""%>>&nbsp;<label for=hide><%=getLocString("common.util.hide")%></label></td>
</tr>
</table>
</div>