<%! final static String urlPrefix = "/smsc/sketches/";%><html>
<head>
	<title>SMSC Admin. <%=request.getParameter("page.title")!=null?request.getParameter("page.title"):""%></title>
	<link rel="stylesheet" type="text/css" href="<%=urlPrefix%>smsc.css">
</head>

<body>
<TABLE class="main" cellspacing="0" height="100%">
  <TR class="main">
    <td class="main" colspan="2" height="1%" bgcolor="#d3dbe4"><h1>SMSC Administration by Novosoft (c)</h1></td></TR>
  <TR class="main">
    <TD class="main" bgcolor="#fffee1" width="20%">
			<%@ include file="left_menu.jsp"%>
    </TD>
    <TD class="main">