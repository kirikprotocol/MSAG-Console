<%@ page import="ru.novosoft.smsc.admin.route.SME"%>
<col width="15%" align=right>
<col width="85%">
<tr class=row<%=(rowN++)&1%>>
	<th>priority:</th>
	<td><input class=txt type="text" name=priority maxlength=5 value="<%=bean.getPriority()%>" validation="priority" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>system type:</th>
	<td><input class=txt type="text" name="systemType" value="<%=StringEncoderDecoder.encode(bean.getSystemType())%>" maxlength="13"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>type of number:</th>
	<td><div class=select>
		<select name="typeOfNumber">
		<option value="0"<%=bean.getTypeOfNumberInt() == 0 ? " selected" : ""%>>Unknown</option>
		<option value="1"<%=bean.getTypeOfNumberInt() == 1 ? " selected" : ""%>>International</option>
		<option value="2"<%=bean.getTypeOfNumberInt() == 2 ? " selected" : ""%>>National</option>
		<option value="3"<%=bean.getTypeOfNumberInt() == 3 ? " selected" : ""%>>Network Specific</option>
		<option value="4"<%=bean.getTypeOfNumberInt() == 4 ? " selected" : ""%>>Subscriber Number</option>
		<option value="5"<%=bean.getTypeOfNumberInt() == 5 ? " selected" : ""%>>Alphanumeric</option>
		<option value="6"<%=bean.getTypeOfNumberInt() == 6 ? " selected" : ""%>>Abbreviated</option>
		</select></div>
	</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>numbering plan:</th>
	<td><div class=select>
		<select name="numberingPlan">
		<option value="0" <%=bean.getNumberingPlanInt() == 0  ? " selected" : ""%>>Unknown</option>
		<option value="1" <%=bean.getNumberingPlanInt() == 1  ? " selected" : ""%>>ISDN (E163/E164)</option>
		<option value="3" <%=bean.getNumberingPlanInt() == 3  ? " selected" : ""%>>Data (X.121)</option>
		<option value="4" <%=bean.getNumberingPlanInt() == 4  ? " selected" : ""%>>Telex (F.69)</option>
		<option value="6" <%=bean.getNumberingPlanInt() == 6  ? " selected" : ""%>>Land Mobile (E.212)</option>
		<option value="8" <%=bean.getNumberingPlanInt() == 8  ? " selected" : ""%>>National</option>
		<option value="9" <%=bean.getNumberingPlanInt() == 9  ? " selected" : ""%>>Private</option>
		<option value="10"<%=bean.getNumberingPlanInt() == 10 ? " selected" : ""%>>ERMES</option>
		<option value="14"<%=bean.getNumberingPlanInt() == 14 ? " selected" : ""%>>Internet (IP)</option>
		<option value="18"<%=bean.getNumberingPlanInt() == 18 ? " selected" : ""%>>WAP Client Id (to be defined by WAP Forum)</option>
		</select></div>
	</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>interface version:</th>
	<td><input class=txt type="text" name="interfaceVersion" value="3.4" readonly></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>range of address:</th>
	<td><input class=txt maxlength="41" type="text" name="rangeOfAddress" value="<%=StringEncoderDecoder.encode(bean.getRangeOfAddress())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><label for="wantAlias_check_id">want alias:</label></th>
	<td><input class=check type="checkbox" name="wantAlias" id="wantAlias_check_id" <%=bean.isWantAlias() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><label for="forceDC_check_id">force data coding:</label></th>
	<td><input class=check type="checkbox" name="forceDC" id="forceDC_check_id" <%=bean.isForceDC() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>timeout:</th>
	<td><input class=txt type="text" name="timeout" value="<%=bean.getTimeout()%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>password:</th>
	<td><input class=txt maxlength="41" type="text" name="password" value="<%=StringEncoderDecoder.encode(bean.getPassword())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>receipt scheme name:</th>
	<td><input class=txt maxlength="32" type="text" name="receiptSchemeName" value="<%=StringEncoderDecoder.encode(bean.getReceiptSchemeName() == null ? "default" : bean.getReceiptSchemeName())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><label for="disabled_check_id">disabled:</label></th>
	<td><input class=check type="checkbox" name="disabled" id="disabled_check_id" <%=bean.isDisabled() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>mode:</th>
	<td><div class=select>
		<select name="mode">
		<option value="<%=SME.MODE_TX%>"  <%=bean.getModeByte() == SME.MODE_TX  ? " selected" : ""%>>TX</option>
		<option value="<%=SME.MODE_RX%>"  <%=bean.getModeByte() == SME.MODE_RX  ? " selected" : ""%>>RX</option>
		<option value="<%=SME.MODE_TRX%>" <%=bean.getModeByte() == SME.MODE_TRX ? " selected" : ""%>>TRX</option>
		</select></div></td>
</tr>