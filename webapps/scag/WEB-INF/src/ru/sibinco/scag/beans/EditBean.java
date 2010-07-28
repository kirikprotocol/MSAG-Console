package ru.sibinco.scag.beans;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;


/**
 * Created by igork
 * Date: 18.03.2004
 * Time: 17:53:33
 */
public abstract class EditBean extends SCAGBean
{
  private String editId = null;
  private boolean add = false;
  private String mbSave = null;
  private String mbCancel = null;

  public abstract String getId();

  public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException
  {
    super.process(request, response);

    if (getId() == null && !isAdd())
      load(getEditId());

    if (getMbCancel() != null)
      throw new CancelException();

    if (getMbSave() != null)
      save();
  }

  protected abstract void load(final String loadId) throws SCAGJspException;

  protected abstract void save() throws SCAGJspException;


  public String getEditId()
  {
    return editId;
  }

  public void setEditId(String editId)
  {
    this.editId = editId;
  }

  public boolean isAdd()
  {
    return add;
  }

  public void setAdd(boolean add)
  {
    this.add = add;
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }
}
