<%@include file="/WEB-INF/inc/header.jspf"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<sm:page>
  <jsp:attribute name="title">
    <c:choose>
      <c:when test="${param.add}">Create new user</c:when>
      <c:otherwise>Edit SME "${param.editId}"</c:otherwise>
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
    <sm-ep:properties title="SME info">
      <sm-ep:txt title="ID" name="id" readonly="${!bean.add}" validation="nonEmpty"/>
      <c:choose>
        <c:when test="${bean.smsc}"><sm-ep:flag title="is SMSC" name="smsc"/></c:when>
        <c:otherwise><sm-ep:list title="provider" name="providerId" values="${fn:join(bean.providerIds, ',')}" valueTitles="${fn:join(bean.providerTitles, ',')}"/></c:otherwise>
      </c:choose>
      <sm-ep:txt title="priority" name="priority" validation="priority"/>
      <sm-ep:txt title="system type" name="systemType" validation="nonEmpty"/>
      <sm-ep:list title="type of number" name="typeOfNumber" values="0,1,2,3,4,5,6" valueTitles="Unknown,International,National,Network Specific,Subscriber Number,Alphanumeric,Abbreviated"/>
      <sm-ep:list title="numbering plan" name="numberingPlan" values="0,1,3,4,6,8,9,10,14,18" valueTitles="Unknown,ISDN (E163/E164),Data (X.121),Telex (F.69),Land Mobile (E.212),National,Private,ERMES,Internet (IP),WAP Client Id (to be defined by WAP Forum)"/>
      <sm-ep:const title="interface version" name="interfaceVersion" value="3.4"/>
      <sm-ep:txt title="range of addres" name="addrRange" validation="nonEmpty"/>
      <sm-ep:check title="is want alias" name="wantAlias"/>
      <sm-ep:check title="force data coding" name="forceDC"/>
      <sm-ep:txt title="timeout" name="timeout" validation="unsigned"/>
      <sm-ep:txt title="password" name="password"/>
      <sm-ep:txt title="receipt scheme name" name="receiptSchemeName"/>
      <sm-ep:check title="disabled" name="disabled"/>
      <sm-ep:list title="mode" name="mode" values="1,2,3" valueTitles="TX,RX,TRX"/>
      <sm-ep:txt title="proclimit" name="proclimit" validation="unsigned"/>
      <sm-ep:txt title="schedlimit" name="schedlimit" validation="unsigned"/>
      <sm-ep:txt title="smeN" name="smeN"/>
    </sm-ep:properties>
  </jsp:body>
</sm:page>