<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<tr><td colspan=2 class=secInner><div class=secView>Alias info</div></td></tr>
<tr class=row0>
	<th class=label>alias:</th>
	<td><input class=txtW name=alias value="<%=bean.getAlias()%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row1>
	<th class=label>address:</th>
	<td><input class=txtW name=address value="<%=bean.getAddress()%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row0>
	<th class=label>&nbsp;</th>
	<td><input class=check type=checkbox id=hide name=hide <%=bean.isHide() ? "checked" : ""%>>&nbsp;<label for=hide>hide</label></td>
</tr>
</table>