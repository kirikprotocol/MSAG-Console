<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "javax.servlet.*"%>
<%@ page import = "javax.servlet.http.*"%>
<HTML>
<HEAD>
<link rel="STYLESHEET" type="text/css" href="common.css">
<script src="scripts.js" type="text/javascript"></script>
</HEAD>
<BODY>
<form name=opForm method=post onSubmit="return validateForm(this)">
<table class=list border=0 width=100%>
<col width=1%>
<col width=99%>
	<tr class=row0>
		<th class=label>f1 match ^as.?d$</th>
		<td><input class=txt onkeyup="return resetValidation(this)" type=text name=testfld1 validation="mask"></td>
	</tr>
	<tr class=row1>
		<th class=label>100 &lt;= f2 &lt;= 1000</th>
		<td><input title="100 &lt;= f2 &lt;= 1000" class=txt onkeyup="resetValidation(this)" type=text name=testfld2 validation="port"></td>
	</tr>
	<tr class=row0>
		<th class=label>f3 &lt;= 100</th>
		<td><input htitle="f3 &lt;= 100" class=txt onkeyup="resetValidation(this)" type=text name=testfld3 validation="priority"></td>
	</tr>
	<tr class=row1>
		<th class=label>100 &lt;= f2 &lt;= 1000</th>
		<td><input class=txt onkeyup="resetValidation(this)" type=text name=testfld4></td>
	</tr>
	<tr class=row0>
		<th class=label>f3 &lt;= 100</th>
		<td><div class=select><select class=txt onchange="resetValidation(this)" name=testfld5 validation="select">
		<option value=0>-----
		<option value=1>1
		<option value=2>2</option>
		</select></div>
		</td>
	</tr>
</table>
<div class=secButtons>
<input class=btn type=submit name=testIt value="Test it">
</div>
</form>
</BODY></HTML>