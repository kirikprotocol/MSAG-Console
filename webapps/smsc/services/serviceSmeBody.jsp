<col width="15%" align=right>
<col width="85%">
<tr class=row<%=(rowN++)&1%>>
	<th class=label>priority:</th>
	<td><input class=txt type="text" name=priority maxlength=5 value="<%=bean.getPriority()%>" validation="priority" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>system type:</th>
	<td><input class=txt type="text" name="systemType" value="<%=StringEncoderDecoder.encode(bean.getSystemType())%>" maxlength="13"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>type of number:</th>
	<td><div class=select>
		<select name="typeOfNumber">
		<option value="0"<%=bean.getTypeOfNumber() == 0 ? " selected" : ""%>>Unknown</option>
		<option value="1"<%=bean.getTypeOfNumber() == 1 ? " selected" : ""%>>International</option>
		<option value="2"<%=bean.getTypeOfNumber() == 2 ? " selected" : ""%>>National</option>
		<option value="3"<%=bean.getTypeOfNumber() == 3 ? " selected" : ""%>>Network Specific</option>
		<option value="4"<%=bean.getTypeOfNumber() == 4 ? " selected" : ""%>>Subscriber Number</option>
		<option value="5"<%=bean.getTypeOfNumber() == 5 ? " selected" : ""%>>Alphanumeric</option>
		<option value="6"<%=bean.getTypeOfNumber() == 6 ? " selected" : ""%>>Abbreviated</option>
		</select></div>
	</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>numbering plan:</th>
	<td><div class=select>
		<select name="numberingPlan">
		<option value="0" <%=bean.getNumberingPlan() == 0  ? " selected" : ""%>>Unknown</option>
		<option value="1" <%=bean.getNumberingPlan() == 1  ? " selected" : ""%>>ISDN (E163/E164)</option>
		<option value="3" <%=bean.getNumberingPlan() == 3  ? " selected" : ""%>>Data (X.121)</option>
		<option value="4" <%=bean.getNumberingPlan() == 4  ? " selected" : ""%>>Telex (F.69)</option>
		<option value="6" <%=bean.getNumberingPlan() == 6  ? " selected" : ""%>>Land Mobile (E.212)</option>
		<option value="8" <%=bean.getNumberingPlan() == 8  ? " selected" : ""%>>National</option>
		<option value="9" <%=bean.getNumberingPlan() == 9  ? " selected" : ""%>>Private</option>
		<option value="10"<%=bean.getNumberingPlan() == 10 ? " selected" : ""%>>ERMES</option>
		<option value="14"<%=bean.getNumberingPlan() == 14 ? " selected" : ""%>>Internet (IP)</option>
		<option value="18"<%=bean.getNumberingPlan() == 18 ? " selected" : ""%>>WAP Client Id (to be defined by WAP Forum)</option>
		</select></div>
	</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>interface version:</th>
	<td><input class=txt type="text" name="interfaceVersion" value="3.4" readonly></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>range of address:</th>
	<td><input class=txt maxlength="41" type="text" name="rangeOfAddress" value="<%=StringEncoderDecoder.encode(bean.getRangeOfAddress())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>want alias:</th>
	<td><input class=check type="checkbox" name="wantAlias" <%=bean.isWantAlias() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>force data coding:</th>
	<td><input class=check type="checkbox" name="forceDC" <%=bean.isForceDC() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>timeout:</th>
	<td><input class=txt type="text" name="timeout" value="<%=bean.getTimeout()%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>password:</th>
	<td><input class=txt maxlength="41" type="text" name="password" value="<%=StringEncoderDecoder.encode(bean.getPassword())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>receipt scheme name:</th>
	<td><input class=txt maxlength="32" type="text" name="receiptSchemeName" value="<%=StringEncoderDecoder.encode(bean.getReceiptSchemeName() == null ? "default" : bean.getReceiptSchemeName())%>"></td>
</tr>