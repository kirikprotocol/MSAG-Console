package ru.sibinco.scag.beans.gw.logging;

import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.EditBean;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Map;
import java.util.HashMap;


/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 03.03.2004
 * Time: 18:39:37
 */
public class Index extends EditBean {

    private Map params = new HashMap();
    private Map requestParams = null;
    private static final String COLLAPSING_TREE_PARAM_PREFIX = "collapsing_tree_param.";


    public String getId() {
        return null;
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
        requestParams = request.getParameterMap();
    }

    protected void load(String loadId) throws SCAGJspException {
        //To change body of implemented methods use File | Settings | File Templates.
    }

    protected void save() throws SCAGJspException {
        //To change body of implemented methods use File | Settings | File Templates.
    }

    public Map getParams() {
        return params;
    }

    public void setParams(Map params) {
        this.params = params;
    }

}
