<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page>
  <jsp:attribute name="title">
    <c:choose>
      <c:when test="${param.add}">Create new service center</c:when>
      <c:otherwise>Edit service center "${param.editId}"</c:otherwise>
    </c:choose>
  </jsp:attribute>

  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbSave" value="Save" title="Save provider info"/>
      <sm-pm:item name="mbCancel" value="Cancel" title="Cancel provider editing" onclick="clickCancel()"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
      <script>
          function changeTransportId() {
              var transport = opForm.all.transportId.options[opForm.all.transportId.selectedIndex].value;
              opForm.submit();
              return true;
          }
      </script>
      <sm-ep:properties title="">
          <sm-ep:list title="Transport" onChange="changeTransportId();" name="transportId"
                      values="${fn:join(bean.transpotIds, ',')}" valueTitles="${fn:join(bean.transpotTitles, ',')}"/>
      </sm-ep:properties>
      <br>
      <br>
      <br>
    <c:choose>
        <c:when test="${bean.transportId == 1}">
    <c:set var="smes" value="${fn:join(bean.smes, ',')}"/>
    <sm-ep:properties title="SMPP Service Center info">
      <br>
      <sm-ep:list title="name" name="id" values="${smes}" valueTitles="${smes}" readonly="${!bean.add}"/>
      <sm-ep:txt title="host" name="host" validation="nonEmpty"/>
      <sm-ep:txt title="port" name="port" validation="port"/>
      <sm-ep:txt title="alternate host" name="altHost"/>
      <sm-ep:txt title="alternate port" name="altPort" validation="port"/>
      <sm-ep:txt title="systemId" name="systemId" validation="nonEmpty"/>
      <sm-ep:txt title="password" name="password"/><!--type="password"-->
      <sm-ep:txt title="responseTimeout" name="responseTimeout" validation="positive"/>
      <sm-ep:txt title="uniqueMsgIdPrefix" name="uniqueMsgIdPrefix" validation="nonEmpty"/>
    </sm-ep:properties>
        </c:when>
    <c:when test="${bean.transportId == 2}">
        <sm-ep:properties title="WAP Service Center info">
        <br>
        Place your content here
        </sm-ep:properties>
        </c:when>
        <c:when test="${bean.transportId == 3}">
        <sm-ep:properties title="MMS Service Center info">
        <br>
        Place your content here
        </sm-ep:properties>
        </c:when>
    </c:choose>
  </jsp:body>
</sm:page>