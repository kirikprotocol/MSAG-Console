<%@ page import="java.util.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%><div class=content>
<div class=page_subtitle>Provider info</div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="85%">
<%int rowN = 0;%>
<tr class=row<%=(rowN++)&1%>>
	<th>Name:</th>
	<td><input class=txt name=name value="<%= StringEncoderDecoder.encode(bean.getName())%>"  ></td>
</tr>
</table>
</div>