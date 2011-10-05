<%@include file="/WEB-INF/inc/header.jspf"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>

<sm:page>
<jsp:attribute name="title">
    <c:choose>
        <c:when test="${param.add}">subjects.add.title</c:when>
        <c:otherwise>subjects.edit.title</c:otherwise>
    </c:choose>
</jsp:attribute>


<jsp:attribute name="menu">
</jsp:attribute>

<jsp:body>
      <script src="content/scripts/http_routes.jsp" type="text/javascript"></script>
      <script type="text/javascript">

        var global_subj_counter = 0;

        function removeRow(tblName, rowId) {
            var tbl = getElementByIdUni( tblName );
            var rowElem = tbl.rows[rowId];
            tbl.deleteRow(rowElem.rowIndex);
        }

        function addpElement(elemName, pname){
            var valueElem = document.getElementById( elemName );
            if( getElementByIdUni("maskRow_"+valueElem.value)!=null){
                alert( "Such mask allready exists");
                return false;
            }
            if( validateField(valueElem) ){
                var tbl = getElementByIdUni( 'sources_table' );
                var newRow = tbl.insertRow(tbl.rows.length);
                newRow.className = "row" + ((tbl.rows.length + 1) & 1);
                newRow.id = "srcRow_" + (global_subj_counter++);
                newCell = document.createElement("td");
                newCell.innerHTML = '<img src="content/images/mask.gif">';
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.innerHTML = valueElem.value + '<input type="hidden" + name="' + pname + '" value="' + valueElem.value + '">';
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(\'sources_table\', \'' + newRow.id + '\')" style="cursor: pointer;">';
                newRow.appendChild(newCell);
                valueElem.value = "";
                valueElem.focus();
                return true;
            }else{
                return false;
            }
        }

        function addPathElement(valueElem, pname){
            if(trimStr(valueElem.value).length > 0){
//                var tbl = opForm.all.path_table;
                var tbl = document.getElementById( 'path_table' );
                var newRow = tbl.insertRow(tbl.rows.length);
                newRow.className = "row" + ((tbl.rows.length + 1) & 1);
                newRow.id = "srcRow_" + (global_subj_counter++);
                newCell = document.createElement("td");
                newCell.innerHTML = '<img src="content/images/mask.gif">';
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.innerHTML = valueElem.value + '<input type="hidden" + name="' + pname + '" value="' + valueElem.value + '">';
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(\'path_table\', \'' + newRow.id + '\')" style="cursor: pointer;">';
                newRow.appendChild(newCell);
                valueElem.value = "";
                valueElem.focus();
                return true;
            }else{
                return false;
            }
        }

        function trimStr(sString) {
            while (sString.substring(0, 1) == ' '){
                sString = sString.substring(1, sString.length);
            }
            while (sString.substring(sString.length - 1, sString.length) == ' '){
                sString = sString.substring(0, sString.length - 1);
            }
            return sString;
        }

      </script>
      <c:choose>
      <c:when test="${bean.transportId == 1}">
          <c:set var="smes" value="${bean.ungroupedSmeIds}"/>
          <table cellpadding="2">
              <col width="50%">
              <col width="50%">
              <tr>
                  <td valign="top">
                      <sm-ep:properties title="subjects.edit.properties.smpp_info">
                          <sm-ep:txt title="subjects.edit.txt.name" name="name" maxlength="60" readonly="${!bean.add}"
                                     validation="nonEmpty"/>
                          <sm-ep:list title="subjects.edit.list.defaultsme" name="defaultSme" values="${smes}"
                                      valueTitles="${smes}"/>
                          <sm-ep:txtBox title="subjects.edit.txtbox.description" cols="0" rows="0" name="description"/>
                      </sm-ep:properties>
                  </td>
                  <td valign="top">&nbsp;</td>
                  <tr><td colspan="2">&nbsp;</td></tr>
          </table>

          <table cellpadding="2">
              <col width="50%">
              <col width="50%">
              <tr>
                  <td valign="top">
                      <table cellspacing="0" width="50%" cellpadding="0">
                          <col width="10%" align="left">
                          <col width="100%" align="right">
                          <col width="100%" align="left">
                          <tr>
                              <td><fmt:message>subjects.edit.label.mask</fmt:message></td>
                              <td><input id="newSrcMask" class="txt" name="masks" validation="routeMask"
                                         onkeyup="resetValidation(this)"></td>
                              <td><img src="content/images/but_add.gif"
                                       onclick="addpElement('newSrcMask', 'masks')"
                                       style="cursor:pointer;"></td>
                          </tr>
                      </table>
                      <hr width="100%" align="left">

                      <c:set var="rowN" value="0"/>
                      <table id="sources_table" width="50%" class="properties_list" cellspacing="0"
                             cellpadding="0">
                          <col width="1%">
                          <col width="100%">
                          <col width="100%">
                          <c:forEach items="${bean.masks}" var="i">
                              <c:set var="emask" value="${fn:escapeXml(i)}"/>
                              <tr class="row${rowN%2}" id="maskRow_${emask}">
                                  <td><img src="content/images/mask.gif"></td>
                                  <td>${emask}<input type="hidden" name="masks" value="${emask}"></td>
                                  <td><img src="content/images/but_del.gif"
                                           onClick="removeRow('sources_table', 'maskRow_${emask}')"
                                           style="cursor: pointer;"></td>
                              </tr>
                              <c:set var="rowN" value="${rowN+1}"/>
                          </c:forEach>
                      </table>
                  </td>
                  <td valign="top">&nbsp;</td>
                  <tr><td colspan="2">&nbsp;</td></tr>
          </table>
          <sm-pm:menu>
              <sm-pm:item name="mbSave" value="subjects.edit.item.mbsave.value" title="subjects.edit.item.mbsave.title"/>
              <sm-pm:item name="mbCancel" value="subjects.edit.item.mbcancel.value" title="subjects.edit.item.mbcancel.title" onclick="clickCancel()"/>
              <sm-pm:space/>
          </sm-pm:menu>
      </c:when>
      <c:when test="${bean.transportId == 2}">
      <c:choose>
      <c:when test="${bean.subjectType == 0}">
          <table cellpadding="2">
              <col width="30%">
              <col width="30%">
              <tr>
                  <td valign="top">
                      <sm-ep:properties title="subjects.edit.properties.http_info">
                          <br>
                          <sm-ep:txt title="subjects.edit.txt.name" name="name" maxlength="60" readonly="${!bean.add}"
                                     validation="nonEmpty"/>
                      </sm-ep:properties>
                  </td>
                  <td valign="top">&nbsp;</td>
                  <tr><td colspan="2">&nbsp;</td></tr>
          </table>

          <table cellpadding="2">
              <col width="30%">
              <col width="30%">
              <tr>
                  <td valign="top">
                      <table cellspacing="0" width="30%" cellpadding="0">
                          <col width="10%" align="left">
                          <col width="100%" align="right">
                          <col width="100%" align="left">
                          <tr>
                              <td><fmt:message>subjects.edit.label.address</fmt:message>&nbsp;</td>
                              <td><input id="newAddress" class="txt" name="address" validation="routeMask"
                                         onkeyup="resetValidation(this)"></td>
                              <td><img src="content/images/but_add.gif"
                                       onclick="addpElement('newAddress', 'address')"
                                       style="cursor:pointer;"></td>
                          </tr>
                      </table>
                      <hr width="100%" align="left">

                      <c:set var="rowN" value="0"/>
                      <table id="sources_table" width="50%" class="properties_list" cellspacing="0"
                             cellpadding="0">
                          <col width="1%">
                          <col width="100%">
                          <col width="100%">
                          <c:forEach items="${bean.address}" var="i">
                              <c:set var="eaddress" value="${fn:escapeXml(i)}"/>
                              <tr class="row${rowN%2}" id="addressRow_${eaddress}">
                                  <td><img src="content/images/mask.gif"></td>
                                  <td>${eaddress}<input type="hidden" name="address" value="${eaddress}"></td>
                                  <td><img src="content/images/but_del.gif"
                                           onClick="removeRow('sources_table', 'addressRow_${eaddress}')"
                                           style="cursor: pointer;"></td>
                              </tr>
                              <c:set var="rowN" value="${rowN+1}"/>
                          </c:forEach>
                      </table>
                  </td>
                  <td valign="top">&nbsp;</td>
                  <tr><td colspan="2">&nbsp;</td></tr>
          </table>
          <input type="hidden" name="transportId" value="${bean.transportId}">
          <input type="hidden" name="subjectType" value="${bean.subjectType}">
          <sm-pm:menu>
              <sm-pm:item name="mbSave" value="subjects.edit.item.mbsave.value" title="subjects.edit.item.mbsave.title" onclick="return validateHttpSubj();"/>
              <sm-pm:item name="mbCancel" value="subjects.edit.item.mbcancel.value" title="subjects.edit.item.mbcancel.title" onclick="clickCancel()"/>
              <sm-pm:space/>
          </sm-pm:menu>
      </c:when>
      <c:when test="${bean.subjectType == 1}">
          <table cellpadding="2">
              <col width="50%">
              <col width="50%">
              <tr>
                  <td valign="top">
                      <sm-ep:properties title="subjects.edit.properties.site_info">
                          <br>
                          <sm-ep:txt title="subjects.edit.txt.name" name="name" maxlength="60" readonly="${!bean.add}"
                                     validation="nonEmpty"/>
                      </sm-ep:properties>
                  </td>
                  <td valign="top">&nbsp;</td>
                  <tr><td colspan="2">&nbsp;</td></tr>
          </table>
          <table cellpadding="0" cellspacing="0">
              <col width="10%" align="left">
              <col width="20%" align="left">
              <col width="0%" align="left">
              <col width="0%" align="left">
              <tr>
                  <td><fmt:message>routes.edit.site.port</fmt:message></td>

                  <td nowrap="true">
                        <input id="newSite" class="midtxt" name="newSite"><b>:</b>
                        <input id="newPort" class="mintxt" name="newPort" value="80" validation="port" onkeyup="resetValidation(this)">
                  </td>
                  <td><img src="content/images/but_add.gif" alt="Add new Site"
<%--                           onclick="addSite(opForm.all.newSite,opForm.all.newPort,'<fmt:message>routes.edit.add.new.path</fmt:message>','<fmt:message>scripts.site.already.exist</fmt:message>')"--%>
                           onclick="addSite('newSite','newPort','<fmt:message>routes.edit.add.new.path</fmt:message>','<fmt:message>scripts.site.already.exist</fmt:message>')"
                           style="cursor: pointer;"></td>
              </tr>
          </table>
            <hr>
          <sm:sites_list type="subj" width="40%"/>
          <br>
          <sm-pm:menu>
              <sm-pm:item name="mbSave" value="subjects.edit.item.mbsave.value" title="subjects.edit.item.mbsave.title" onclick="return validateHttpSite();"/>
              <sm-pm:item name="mbCancel" value="subjects.edit.item.mbcancel.value" title="subjects.edit.item.mbcancel.title"
                          onclick="clickCancel()"/>
              <sm-pm:space/>
          </sm-pm:menu>
          <input type="hidden" name="transportId" value="${bean.transportId}">
          <input type="hidden" name="subjectType" value="${bean.subjectType}">
      </c:when>
      </c:choose>
      </c:when>
      </c:choose>
  </jsp:body>
</sm:page>