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
      <script src="content/scripts/http_routes.js" type="text/javascript"></script>
      <script type="text/javascript">

        var global_subj_counter = 0;

        function removeRow(tbl, rowId) {
            var rowElem = tbl.rows(rowId);
            tbl.deleteRow(rowElem.rowIndex);
        }

        function addpElement(valueElem, pname){
            if(validateField(valueElem)){
                var tbl = opForm.all.sources_table;
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
                newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(opForm.all.sources_table, \'' + newRow.id + '\')" style="cursor: hand;">';
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
                var tbl = opForm.all.path_table;
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
                newCell.innerHTML = '<img src="content/images/but_del.gif" onClick="removeRow(opForm.all.path_table, \'' + newRow.id + '\')" style="cursor: hand;">';
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
          <c:set var="smes" value="${bean.smeIds}"/>
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
                                       onclick="addpElement(opForm.all.newSrcMask, 'masks')"
                                       style="cursor:hand;"></td>
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
                                           onClick="removeRow(opForm.all.sources_table, 'maskRow_${emask}')"
                                           style="cursor: hand;"></td>
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
                                       onclick="addpElement(opForm.all.newAddress, 'address')"
                                       style="cursor:hand;"></td>
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
                                           onClick="removeRow(opForm.all.sources_table, 'addressRow_${eaddress}')"
                                           style="cursor: hand;"></td>
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
              <sm-pm:item name="mbSave" value="subjects.edit.item.mbsave.value" title="subjects.edit.item.mbsave.title"/>
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

                  <td nowrap="true"><input id="newSite" class="midtxt" name="newSite"><b>:</b><input id="newPort"
                                                                                                     class="mintxt"
                                                                                                     name="newPort"
                                                                                                     value="80"
                                                                                                     validation="port"
                                                                                                     onkeyup="resetValidation(this)">
                  </td>
                  <td><img src="content/images/but_add.gif" alt="Add new Site"
                           onclick="addSite(opForm.all.newSite,opForm.all.newPort,'<fmt:message>routes.edit.add.new.path</fmt:message>','<fmt:message>scripts.site.already.exist</fmt:message>')"
                           style="cursor: hand;"></td>
              </tr>
          </table>
            <hr>
          <sm-et:section title="Sites List" opened="true"  name="allsites" width="40%">
                <table id="div_site_table" cellpadding="0" cellspacing="0" class="properties_list">
                    <tr><td>
                        <c:forEach items="${bean.sites}" var="i">
                            <c:set var="esite" value="${fn:escapeXml(i.host)}"/>
                            <c:set var="eport" value="${fn:escapeXml(i.port)}"/>
                            <c:set var="esite_sub" value="${fn:substringBefore(esite,'.')}"/>
                            <c:set var="sectHeader" value="sectionHeader_${esite_sub}"/>
                            <c:set var="esite_table" value="sitesp_table_${esite_sub}"/>
                            <c:set var="defaultSiteObjId" value="${fn:escapeXml(i.defaultSite)}"/>

                            <div class="collapsing_tree_opened" id="${sectHeader}" style="width:100%;">
                                 <%--onclick="collasping_tree_showhide_section('${esite_sub}')">--%>

                                <table id="${esite_table}" cellpadding="0" cellspacing="0" class="properties_list">
                                    <col width="1%">
                                    <col width="99%">
                                    <tr>
                                        <td width="100%"><input type="radio" name="defaultSiteObjId" value="${esite}" <c:if test="${defaultSiteObjId}">checked</c:if>>&nbsp;${fn:escapeXml(i.host)}</td>
                                        <td align="left" nowrap="true"><input type="hidden" name="sitesHost"  id="${esite}"
                                                                              value="${esite}">
                                            <input type="hidden" name="sitesPort" value="${esite}_${eport}"></td>
                                        <td><img src="content/images/but_del.gif"
                                                 onClick="removeSection('${esite_sub}')"
                                                 style="cursor:hand;"></td>
                                    </tr>

                                    <tr>
                                        <td nowrap="true"  align="right"><fmt:message>routes.edit.add.new.path</fmt:message>&nbsp;</td>
                                        <td align="right"><input id="${esite_sub}_newPath" class="txt"
                                                                 name="${esite_sub}_newPath"></td>
                                        <td><img src="content/images/but_add.gif" alt="Add new path"
                                                 onclick="addPath(opForm.all.${esite_sub}_newPath, '${esite}', opForm.all.sitesp_table_${esite_sub}, '${esite_sub}')"
                                                 style="cursor:hand;"></td>
                                    </tr>
                                    <c:set var="rowN" value="0"/>
                                    <c:forEach items="${i.pathLinks}" var="j">
                                        <c:set var="epath" value="${fn:escapeXml(j)}"/>
                                        <tr class="row${rowN%2}" id="pathRow_${esite_sub}_${epath}">
                                            <td></td>
                                            <td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;${epath}<input type="hidden"
                                                                                             name="pathLinks"
                                                                                             value="${esite}_${epath}">
                                            </td>
                                            <td><img src="content/images/but_del.gif"
                                                     onClick="removeRow(opForm.all.sitesp_table_${esite_sub}, 'pathRow_${esite_sub}_${epath}')"
                                                     style="cursor:hand;"></td>
                                        </tr>
                                        <c:set var="rowN" value="${rowN+1}"/>
                                    </c:forEach>
                                </table>
                            </div>
                        </c:forEach>
                    </td></tr>
                    <tr><td valign="top">&nbsp;</td></tr>
                  <tr><td colspan="2">&nbsp;</td></tr>
                </table>
            </sm-et:section>
          <br>
          <sm-pm:menu>
              <sm-pm:item name="mbSave" value="subjects.edit.item.mbsave.value" title="subjects.edit.item.mbsave.title"/>
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