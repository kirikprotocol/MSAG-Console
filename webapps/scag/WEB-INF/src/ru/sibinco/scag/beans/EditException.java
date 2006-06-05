package ru.sibinco.scag.beans;


/**
 * Created by igork
 * Date: 15.03.2004
 * Time: 16:12:48
 */
public class EditException extends SCAGJspException {
    private final String editId;
    private String param;

    public EditException(String editId) {
        this.editId = editId;
    }

    public EditException(String editId, String param) {
        this.editId = editId;
        this.param = param;
    }

    public String getParam() {
        return param;
    }

    public String getEditId() {
        return editId;
    }
}
