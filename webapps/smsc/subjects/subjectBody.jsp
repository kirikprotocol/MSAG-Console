<%int rowN = 0;%>
<%
//if (beanResult == bean.RESULT_OK)
{
%>
<table class=secRep cellspacing=0 cellpadding=2 width="100%">
<col width="15%" align=right>
<col width="85%">
<tr><td colspan=2 class=secInner><div class=secView>Subject information</div></td></tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>name:</th>
	<td><input class=txtW name=name value="<%=bean.getName()%>" <%=isNew?"":"readonly"%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>default SME:</th>
	<td><div class=select><select name=defSme>
		<%for (Iterator i = bean.getPossibleSmes().iterator(); i.hasNext(); )
		{
			String smeId = (String) i.next();
			String encSmeId = StringEncoderDecoder.encode(smeId);
			%><option value="<%=encSmeId%>" <%=smeId.equals(bean.getDefSme()) ? "selected" : ""%>><%=encSmeId%></option><%
		}%>
	</select></div></td>
</tr>
<tr><td colspan=2 class=secInner><div class=secList>Masks</div></td></tr>
<%
for (int i=0; i<bean.getMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>&nbsp;</th>
	<td><input class=txtW name=masks value="<%=bean.getMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to subject"></th>
	<td><input class=txtW name=masks validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
</table>
<%}%>