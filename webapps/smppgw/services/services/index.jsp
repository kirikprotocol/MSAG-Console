<%@include file="/WEB-INF/inc/header.jspf"%>

<sm:page title="Services">

  <jsp:body>
 <OBJECT id="tdcConnStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
  	<PARAM NAME="DataURL" VALUE="services/services/connected_statuses.jsp">
  	<PARAM NAME="UseHeader" VALUE="True">
  	<PARAM NAME="TextQualifier" VALUE='"'>
  </OBJECT>

<script>
  function refreshStatus()
  {
  	document.all.tdcConnStatuses.DataURL = document.all.tdcConnStatuses.DataURL;
  	document.all.tdcConnStatuses.reset();
  	window.setTimeout(refreshStatus, 5000);
  }
  refreshStatus();
  </script>
    <sm:table columns="id,providerName,smsc,status" names="id,provider,is SMSC,status" widths="30,30,30,10" />

 <%--   <table class=list cellspacing=1 width="100%">
    <col align=left>
    <col width="1%" align=center>
    <thead>
   <tr>
    	<th>service</th>
    	<th colspan="1">status</th>
    </tr>
    </thead>
    <tbody>
<c:set var="smes" value="${bean.dataSources}"/>
 <c:forEach items="${bean.smeIds}" var="result" varStatus="counter">
<c:set var="smeId" value="${result}"/>
<c:set var="sme" value="${smes[counter.count]}"/>
<c:set var="smeStatus" value="${sme.status}"/>
<tr class='row${(status.count+1)%2}'>
     <td class=name>${smeId}</td>
     <td class=name>${smf:smeStatus(smeStatus,smeId)}</td>
    </tr>
</c:forEach>
    </tbody>
    </table>
    --%>
  </jsp:body>
</sm:page>