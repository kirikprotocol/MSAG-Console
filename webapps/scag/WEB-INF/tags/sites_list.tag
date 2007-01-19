<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<%@taglib tagdir="/WEB-INF/tags/edit/tree" prefix="sm-et"%>
<%@attribute name="width" required="false"%>
<%@attribute name="type" required="true"%>
            <sm-et:section title="Sites List" opened="true"  name="allsites" width="${width}" >
                <table id="div_site_table" cellpadding="0" cellspacing="0" class="properties_list">
                    <tr><td>

                        <c:forEach items="${bean.sites}" var="i">
                            <c:set var="esite" value="${fn:escapeXml(i.host)}"/>
                            <c:set var="eport" value="${fn:escapeXml(i.port)}"/>
                            <c:set var="esite_sub" value="${fn:replace(esite,'.','_')}"/>
                            <c:set var="sectHeader" value="sectionHeader_${esite_sub}"/>
                            <c:set var="esite_table" value="sitesp_table_${esite_sub}"/>
                            <c:set var="defaultSiteObjId" value="${fn:escapeXml((type=='route')?bean.defaultSiteObjId:i.defaultSite)}"/>

                            <div class="collapsing_tree_opened" id="${sectHeader}" style="width:100%;">
                                 <%--onclick="collasping_tree_showhide_section('${esite_sub}')">--%>

                                <table id="${esite_table}" cellpadding="0" cellspacing="0" class="properties_list">
                                    <col width="1%">
                                    <col width="99%">
                                    <tr>
                                        <td width="100%"><input type="radio" name="defaultSiteObjId" <c:if test="${(type=='route')?defaultSiteObjId==esite:defaultSiteObjId}">checked</c:if> value="${esite}">&nbsp;${fn:escapeXml(i.host)}:${fn:escapeXml(i.port)}</td>
                                        <td align="left" nowrap="true"><input type="hidden" name="sitesHost"  id="${esite}"
                                                                              value="${esite}">
                                            <input type="hidden" name="sitesPort" value="${esite}_${eport}"></td>
                                        <td><img src="content/images/but_del.gif"
                                                 onClick="removeSection('${esite_sub}')"
                                                 style="cursor:hand;"></td>
                                    </tr>

                                    <tr>
                                        <td nowrap="true" align="right"><fmt:message>routes.edit.add.new.path</fmt:message>&nbsp;</td>
                                        <td align="right"><input id="newPath_${esite_sub}" class="txt"
                                                                 name="newPath_${esite_sub}" onkeyup="resetValidation(this)"></td>
                                        <td><img src="content/images/but_add.gif" alt="Add new path"
                                                 onclick="addPath(opForm.all.newPath_${esite_sub}, '${esite}', opForm.all.sitesp_table_${esite_sub}, '${esite_sub}')"
                                                 style="cursor:hand;"></td>
                                    </tr>
                                    <c:set var="rowN" value="0"/>
                                    <c:forEach items="${i.pathLinks}" var="j">
                                        <c:set var="epath" value="${fn:escapeXml(j)}"/>
                                        <tr class="row${rowN%2}" id="pathRow_${esite_sub}_${epath}">
                                            <td></td>
                                            <td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;${epath}<input type="hidden"
                                                                                             name="pathLinks"
                                                                                             value="${esite}^${epath}">
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
                </table>
            </sm-et:section>