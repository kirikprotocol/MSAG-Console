package ru.novosoft.smsc.admin.route;

/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:03:51
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.util.Collection;
import java.util.Iterator;


public class Subject
{
  private String name = null;
  private SME defaultSme = null;
  private MaskList masks = null;
  private String notes = "";
  private ChildSubjectsList childSubjects = null;

  public Subject(String name, String[] masksStrings, SME defaultSME, String notes) throws AdminException
  {
    this.notes = notes;
    if (name == null)
      throw new NullPointerException("Name is null");
    if (masksStrings == null)
      throw new NullPointerException("Masks is null");
    if (defaultSME == null)
      throw new NullPointerException("DefaultSME is null");

    this.name = name;
    this.childSubjects = new ChildSubjectsList();
    masks = new MaskList(masksStrings);
    if (masks.size() == 0)
      throw new AdminException("Masks is empty");
    this.defaultSme = defaultSME;
  }

  public Subject(String name, String[] masksStrings, SME defaultSME, String notes, String[] childSubjects) throws AdminException
  {
    this.notes = notes;
    this.childSubjects = new ChildSubjectsList(childSubjects);
    if (name == null)
      throw new NullPointerException("Name is null");
    if (masksStrings == null)
      throw new NullPointerException("Masks is null");
    if (defaultSME == null)
      throw new NullPointerException("DefaultSME is null");

    this.name = name;
    masks = new MaskList(masksStrings);
    if (masks.size() == 0 && (childSubjects == null || childSubjects.length ==0))
      throw new AdminException("Masks is empty");
    this.defaultSme = defaultSME;
  }

  public Subject(String name, Collection masksStrings, SME defaultSME, String notes)
  {
    this.notes = notes;
    this.childSubjects = new ChildSubjectsList();
    if (name == null)
      throw new NullPointerException("Name is null");
    if (masksStrings == null)
      throw new NullPointerException("Masks is null");
    if (defaultSME == null)
      throw new NullPointerException("DefaultSME is null");

    this.name = name;
    masks = new MaskList(masksStrings);
    if (masks.size() == 0)
      throw new NullPointerException("Masks is empty");
    this.defaultSme = defaultSME;
  }

  public String getName()
  {
    return name;
  }

  public MaskList getMasks()
  {
    return masks;
  }

  public SME getDefaultSme()
  {
    return defaultSme;
  }

  public void setDefaultSme(SME defaultSme)
  {
    this.defaultSme = defaultSme;
  }

  public void setMasks(MaskList masks)
  {
    this.masks = masks;
  }

  public void setChildSubjects(ChildSubjectsList list) {
    this.childSubjects = list;
  }

  public ChildSubjectsList getChildSubjects() {
    return childSubjects;
  }

  public PrintWriter store(PrintWriter out)
  {
    out.println("  <subject_def id=\"" + StringEncoderDecoder.encode(getName()) + "\" defSme=\"" + StringEncoderDecoder.encode(getDefaultSme().getId()) + "\">");
    if (notes != null)
      out.println("    <notes>" + notes + "</notes>");
    getMasks().store(out);
    getChildSubjects().store(out);
    out.println("  </subject_def>");
    return out;
  }

  public String getNotes()
  {
    return notes;
  }

  public void setNotes(String notes)
  {
    this.notes = notes;
  }
}
