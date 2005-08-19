<%@include file="/WEB-INF/inc/header.jspf"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<sm:page>
  <jsp:attribute name="title">
    <c:choose>
      <c:when test="${param.add}">Create new Service</c:when>
      <c:otherwise>Edit Service "${param.editId}"</c:otherwise>
    </c:choose>
  </jsp:attribute>

  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbSave" value="Save" title="Save user info"/>
      <sm-pm:item name="mbCancel" value="Cancel" title="Cancel user editing" onclick="clickCancel()"/>
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
     <sm-ep:list title="Transport" onChange="changeTransportId();" name="transportId" values="${fn:join(bean.transpotIds, ',')}" valueTitles="${fn:join(bean.transpotTitles, ',')}"/>
    </sm-ep:properties>
    <br>
    <br>
    <br>
    <c:choose>
        <c:when test="${bean.transportId == 1}">
    <sm-ep:properties title="SMPP Service (SME) info">
    <br>
      <sm-ep:txt title="ID" name="id" readonly="${!bean.add}" validation="nonEmpty"/>
      <c:choose>
        <c:when test="${bean.smsc}"><sm-ep:flag title="is SMSC" name="smsc"/></c:when>
        <c:otherwise>
        <c:choose>
            <c:when test="${bean.administrator}">
                <sm-ep:list  title="provider" name="providerId"
                       values="${fn:join(bean.providerIds, ',')}"
                       valueTitles="${fn:join(bean.providerNames, ',')}"/>
            </c:when>
        <c:otherwise>
          <sm-ep:const title="provider" name="providerName" value="${bean.providerName}"/>
        </c:otherwise>
      </c:choose>
        </c:otherwise>

      </c:choose>
      <sm-ep:txt title="timeout" name="timeout" validation="unsigned"/>
      <sm-ep:txt title="password" name="password"/>
      <sm-ep:check title="enabled" name="enabled"/>
      <sm-ep:list title="mode" name="mode" values="1,2,3" valueTitles="TX,RX,TRX"/>
    </sm-ep:properties>
    </c:when>
    <c:when test="${bean.transportId == 2}">
        <sm-ep:properties title="WAP Service info">
        Place your content here
        </sm-ep:properties>
        </c:when>
        <c:when test="${bean.transportId == 3}">
        <sm-ep:properties title="MMS Service info">
        Place your content here
        </sm-ep:properties>
        </c:when>
    </c:choose>
  </jsp:body>
</sm:page>