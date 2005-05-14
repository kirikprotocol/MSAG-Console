package ru.sibinco.smppgw.beans.routing.subjects;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.*;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.beans.*;

import java.util.*;


/**
 * Created by igork Date: 20.04.2004 Time: 15:51:47
 */
public class Edit extends EditBean
{
  private String name;
  private String defaultSme;
  private String[] masks;
  private String notes;

  public String getId()
  {
    return name;
  }

  protected void load(final String loadId) throws SmppgwJspException
  {
    final Subject subject = (Subject) appContext.getGwRoutingManager().getSubjects().get(loadId);
    if (null != subject) {
      name = subject.getName();
      defaultSme = null != subject.getDefaultSme() ? subject.getDefaultSme().getId() : null;
      masks = new String[subject.getMasks().size()];
      int counter = 0;
      for (Iterator i = subject.getMasks().iterator(); i.hasNext();) {
        final Mask mask = (Mask) i.next();
        masks[counter++] = mask.getMask();
      }
      notes = subject.getNotes();
    }
  }

  protected void save() throws SmppgwJspException
  {
    masks = Functions.trimStrings(masks);
    final Map subjects = appContext.getGwRoutingManager().getSubjects();
    final Sme defSme = (Sme) appContext.getGwSmeManager().getSmes().get(defaultSme);
    if (null == defSme)
      throw new SmppgwJspException(Constants.errors.routing.subjects.DEFAULT_SME_NOT_FOUND, defaultSme);

    if (isAdd()) {
      if (subjects.containsKey(name))
        throw new SmppgwJspException(Constants.errors.routing.subjects.SUBJECT_ALREADY_EXISTS, name);
      try {
        subjects.put(name, new Subject(name, masks, defSme, notes));
      } catch (SibincoException e) {
        logger.debug("Could not create new subject", e);
        throw new SmppgwJspException(Constants.errors.routing.subjects.COULD_NOT_CREATE, e);
      }
    } else {
      if (!getEditId().equals(name)) {
        if (subjects.containsKey(name))
          throw new SmppgwJspException(Constants.errors.routing.subjects.SUBJECT_ALREADY_EXISTS, name);
        subjects.remove(getEditId());
        try {
          subjects.put(name, new Subject(name, masks, defSme, notes));
        } catch (SibincoException e) {
          logger.debug("Could not create new subject", e);
          throw new SmppgwJspException(Constants.errors.routing.subjects.COULD_NOT_CREATE, e);
        }
      } else {
        final Subject subject = (Subject) subjects.get(name);
        subject.setDefaultSme(defSme);
        try {
          subject.setMasks(new MaskList(masks));
        } catch (SibincoException e) {
          logger.debug("Could not set masks list for subject", e);
          throw new SmppgwJspException(Constants.errors.routing.subjects.COULD_NOT_SET_MASKS, e);
        }
        subject.setNotes(notes);
      }
    }
    appContext.getStatuses().setRoutesChanged(true);
    throw new DoneException();
  }

  public String getSmeIds()
  {
    final StringBuffer result = new StringBuffer();
    final List smes = new SortedList(appContext.getGwSmeManager().getSmes().keySet());
    for (Iterator i = smes.iterator(); i.hasNext();) {
      result.append((String) i.next());
      if (i.hasNext())
        result.append(",");
    }
    return result.toString();
  }

  public String getName()
  {
    return name;
  }

  public void setName(final String name)
  {
    this.name = name;
  }

  public String getDefaultSme()
  {
    return defaultSme;
  }

  public void setDefaultSme(final String defaultSme)
  {
    this.defaultSme = defaultSme;
  }

  public String[] getMasks()
  {
    return masks;
  }

  public void setMasks(final String[] masks)
  {
    this.masks = masks;
  }

  public String getNotes()
  {
    return notes;
  }

  public void setNotes(final String notes)
  {
    this.notes = notes;
  }
}
