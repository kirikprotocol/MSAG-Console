<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=secView>Profile information</div></td></tr>
<tr class=row0>
	<th class=label>mask:</th>
	<td><input class=txtW name="mask" value="<%=bean.getMask()%>" <%=isEdit ? "readonly" : ""%> validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row0>
	<th class=label>codepage:</th>
	<td><div class=select><select class=txt name=codepage>
			<option value="<%=Profile.CODEPAGE_Default%>" <%=bean.getCodepage() == Profile.CODEPAGE_Default ? "SELECTED" : ""%>>Default</option>
			<option value="<%=Profile.CODEPAGE_UCS2   %>" <%=bean.getCodepage() == Profile.CODEPAGE_UCS2    ? "SELECTED" : ""%>>UCS2</option>
		</select></div></td>
</tr>
<tr class=row1>
	<th class=label>report mode:</th>
	<td><div class=select><select class=txt name="report">
			<option value="<%=Profile.REPORT_OPTION_None%>" <%=bean.getReport() == Profile.REPORT_OPTION_None ? "SELECTED" : ""%>>none</option>
			<option value="<%=Profile.REPORT_OPTION_Full%>" <%=bean.getReport() == Profile.REPORT_OPTION_Full ? "SELECTED" : ""%>>full</option>
		</select></div></td>
</tr>
</table>