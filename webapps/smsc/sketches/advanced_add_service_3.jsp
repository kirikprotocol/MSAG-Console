<%@ include file="header.jsp"%>
<% String host = URLDecoder.decode(request.getParameter("host") == null ? "SMSC Main" : request.getParameter("host")); %>
<% int port = Integer.valueOf(request.getParameter("port") == null ? "0" : request.getParameter("port") ).intValue(); %>
<% String args = URLDecoder.decode(request.getParameter("args") == null ? "" : request.getParameter("args")); %>
<% String systemId = "Mail->SMS SME System Id";%>
<% int typeOfNumber = Integer.valueOf(request.getParameter("type of number") == null ?  "0" : request.getParameter("type of number")).intValue(); %>
<% int numberingPlan = Integer.valueOf(request.getParameter("numbering plan") == null ? "0" : request.getParameter("numbering plan")).intValue(); %>
<% String interfaceVersion = request.getParameter("interface version") == null ? "" : request.getParameter("interface version"); %>
<% String rangeOfAddress = request.getParameter("range of address") == null ? "" : request.getParameter("range of address"); %>
<% String systemType = request.getParameter("system type") == null ? "" : request.getParameter("system type"); %>
<% String smeNType = request.getParameter("sme ntype") == null ? "" : request.getParameter("sme ntype"); %>

	<h2>Service Added:</h2>
	Name: <%=systemId%><br>
	host: <%=host%>:<%=port%><br>
	arguments for executable: <pre><%=args%></pre>
	type of number: <%
		switch (typeOfNumber)
		{
		case 0: out.print("Unknown");
			break;
		case 1: out.print("International");
			break;
		case 2: out.print("National");
			break;
		case 3: out.print("Network Specific");
			break;
		case 4: out.print("Subscriber Number");
			break;
		case 5: out.print("Alphanumeric");
			break;
		case 6: out.print("Abbreviated");
			break;
		default:out.print("wrong: " + typeOfNumber);
		}%><br>
	Numbering plan: <%
		switch (numberingPlan)
		{
		case 0: out.print("Unknown");
			break;
		case 1: out.print("ISDN (E163/E164)");
			break;
		case 3: out.print("Data (X.121)");
			break;
		case 4: out.print("Telex (F.69)");
			break;
		case 6: out.print("Land Mobile (E.212)");
			break;
		case 8: out.print("National");
			break;
		case 9: out.print("Private");
			break;
		case 10: out.print("ERMES");
			break;
		case 14: out.print("Internet (IP)");
			break;
		case 18: out.print("WAP Client Id (to be defined by WAP Forum)");
			break;
		default:out.print("wrong: " + numberingPlan);
		}%><br>
		Interface version: <%= interfaceVersion%><br>
		Range of Address: <%=rangeOfAddress%><br>
		System Type: <%= systemType%><br>
		SME NType: <%= smeNType%>
<%@ include file="footer.jsp"%>