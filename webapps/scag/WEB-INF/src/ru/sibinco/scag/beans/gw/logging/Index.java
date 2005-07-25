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

    public String getId() {
        return null;
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
    }

    protected void load(String loadId) throws SCAGJspException {
        //To change body of implemented methods use File | Settings | File Templates.
    }

    protected void save() throws SCAGJspException {
        //To change body of implemented methods use File | Settings | File Templates.
    }
}
