<div class=content>
<div class=page_subtitle>Alias info</div>
<table class=list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="85%">
<tr class=row0>
	<td class=label>alias:</td>
	<td><input class=txtW name=alias value="<%=bean.getAlias()%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row1>
	<td class=label>address:</td>
	<td><input class=txtW name=address value="<%=bean.getAddress()%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row0>
	<td class=label>&nbsp;</td>
	<td><input class=check type=checkbox id=hide name=hide <%=bean.isHide() ? "checked" : ""%>>&nbsp;<label for=hide>hide</label></td>
</tr>
</table>
</div>