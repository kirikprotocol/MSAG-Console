<%@include file="/WEB-INF/inc/header.jspf"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<sm:page>
  <jsp:attribute name="title">
    <c:choose>
      <c:when test="${param.add}">Create new Service point</c:when>
      <c:otherwise>Edit Service point "${param.editId}"</c:otherwise>
    </c:choose>
  </jsp:attribute>

  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbSave" value="Save" title="Save Service point info"/>
      <sm-pm:item name="mbCancel" value="Cancel" title="Cancel Service point editing" onclick="clickCancel()"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
  <script>
   function changeTransportId(){
      var transport = opForm.all.transportId.options[opForm.all.transportId.selectedIndex].value;
      opForm.submit();
      return true;
    }
    </script>

    <sm-ep:properties title="">
     <sm-ep:list title="Transport" onChange="changeTransportId();" name="transportId" values="${fn:join(bean.transportIds, ',')}" valueTitles="${fn:join(bean.transportTitles, ',')}"/>
    </sm-ep:properties>
    <br>
    <br>
    <br>
    <c:choose>
        <c:when test="${bean.transportId == 1}">
    <sm-ep:properties title="SMPP Service point (SME) info">
    <br>
      <sm-ep:txt title="ID" name="id" maxlength="60" validation="id"/>
      <sm-ep:txt title="timeout" name="timeout" maxlength="6" validation="unsigned"/>
      <sm-ep:txt title="password" name="password"/>
      <sm-ep:check title="" head="enabled" name="enabled"/>
      <sm-ep:list title="mode" name="mode" values="1,2,3" valueTitles="TX,RX,TRX"/>
    </sm-ep:properties>
    </c:when>
    <c:when test="${bean.transportId == 2}">
        <sm-ep:properties title="HTTP Service point info">
        Place your content here
        </sm-ep:properties>
        </c:when>
        <c:when test="${bean.transportId == 3}">
        <sm-ep:properties title="MMS Service point info">
        Place your content here
        </sm-ep:properties>
        </c:when>
    </c:choose>
  </jsp:body>
</sm:page>