<%@ page import="ru.novosoft.smsc.jsp.SMSCAppContext,
                 java.net.URLEncoder,
                 ru.novosoft.util.jsp.MultipartServletRequest,
                 ru.novosoft.util.jsp.MultipartDataSource"%>
<%
MultipartServletRequest multi = (MultipartServletRequest)request.getAttribute("multipart.request");
if (multi == null)
  throw new NullPointerException("File not attached");
request = (HttpServletRequest)multi;

String systemId = "";
MultipartDataSource dataFile = null;
try {
  dataFile = multi.getMultipartDataSource("distribute");
  if (dataFile == null)
    throw new Exception("Service distributive not attached");
  if( dataFile.getContentType().equals("application/x-zip-compressed") ) {
    java.io.InputStream is = dataFile.getInputStream();
    systemId = ((SMSCAppContext)request.getAttribute("appContext")).getServiceManager().receiveNewService(is);
    dataFile.close();
    dataFile = null;
  }
  else
    throw new Exception("Distributive file must be zip compressed");
} finally {
  if( dataFile != null ) dataFile.close();
}

//String service = request.getParameter("service") == null ? "" : request.getParameter("service");;
String host = request.getParameter("host") == null ? "" : request.getParameter("host");
String portString = request.getParameter("port") == null ? "" : request.getParameter("port");
String args = request.getParameter("args") == null ? "" : request.getParameter("args");

if (/*service.equals("") || */host.equals("") || portString.equals("") || systemId.equals(""))
  throw new Exception("Not all parameters defined");

int port = 0;
try {
  port = Integer.decode(portString).intValue();
} catch (NumberFormatException e) {
  throw new Exception("Port number misformatted: "+e.getMessage());
}
if (port == 0)
  throw new Exception("Port number not defined");
%>


<%@ include file="/common/header.jsp"%>
<form action="<%=urlPrefix+servicesPrefix%>/add_service_3.jsp" method="post">
	<--input type="hidden" name="service" value="<%=/*StringEncoderDecoder.encode(service)*/""%>"-->
	<input type="hidden" name="host" value="<%=StringEncoderDecoder.encode(host)%>">
	<input type="hidden" name="port" value="<%=String.valueOf(port)%>">
  <input type="hidden" name="args" value="<%=StringEncoderDecoder.encode(args)%>">
  <h2>Add service:</h2>
  <h3>service parameters:</h3>
  <table class="list">
		<tr class="list">
			<th class="list">System Id</th>
			<td class="list"><input type="text" name="system id" value="<%=systemId%>" readonly style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">System Type</th>
			<td class="list"><input type="text" name="system type" value="" style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">Type of number</th>
			<td class="list">
				<select name="type of number" style="width: 100%;">
					<option value="0" selected>Unknown</option>
					<option value="1">International</option>
					<option value="2">National</option>
					<option value="3">Network Specific</option>
					<option value="4">Subscriber Number</option>
					<option value="5">Alphanumeric</option>
					<option value="6">Abbreviated</option>
				</select>
			</td>
		</tr>
		<tr class="list">
			<th class="list">Numbering plan</th>
			<td class="list">
				<select name="numbering plan" style="width: 100%;">
					<option value="0" selected>Unknown</option>
					<option value="1">ISDN (E163/E164)</option>
					<option value="3">Data (X.121)</option>
					<option value="4">Telex (F.69)</option>
					<option value="6">Land Mobile (E.212)</option>
					<option value="8">National</option>
					<option value="9">Private</option>
					<option value="10">ERMES</option>
					<option value="14">Internet (IP)</option>
					<option value="18">WAP Client Id (to be defined by WAP Forum)</option>
				</select>
			</td>
		</tr>
		<tr class="list">
			<th class="list">Interface version</th>
			<td class="list"><input type="text" name="interface version" value="3.4" readonly style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">Range of Address</th>
			<td class="list"><input type="text" name="range of address" value="" style="width: 100%;"></td>
		</tr>
		<!--tr class="list">
			<th class="list">Password</th>
			<td class="list"><input type="password" name="password" value="" style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">retype Password</th>
			<td class="list"><input type="password" name="password" value="" style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">another Port</th>
			<td class="list"><input type="text" name="sme port" value="" style="width: 100%;"></td>
		</tr-->
		<tr class="list">
			<th class="list">SME NType</th>
			<td class="list"><input type="text" name="sme ntype" value="????" style="width: 100%; color: Red;"></td>
		</tr>
	</table>
	<div align="right">
		<INPUT type=submit value="Next page">
	</div>
</form>
<%@ include file="/common/footer.jsp"%>