/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:03:51
 */
package ru.sibinco.lib.backend.route;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.backend.util.xml.Utils;

import java.io.PrintWriter;
import java.util.Collection;


public class Subject
{
  private String name = null;
  private Sme defaultSme = null;
  private MaskList masks;
  private String notes = "";

  public Subject(Element subjElem, SmeManager smeManager) throws SibincoException
  {
    name = subjElem.getAttribute("id");
    NodeList masksList = subjElem.getElementsByTagName("mask");
    masks = new MaskList();
    for (int j = 0; j < masksList.getLength(); j++) {
      Element maskElem = (Element) masksList.item(j);
      masks.add(new Mask(maskElem.getAttribute("value").trim()));
    }
    defaultSme = (Sme) smeManager.getSmes().get(subjElem.getAttribute("defSme"));
    if (defaultSme == null)
      throw new SibincoException("Unknown default Sme \"" + subjElem.getAttribute("defSme") + "\" for subject \"" + name + "\"");

    NodeList notesList = subjElem.getElementsByTagName("notes");
    for (int j = 0; j < notesList.getLength(); j++)
      notes += Utils.getNodeText(notesList.item(j));
  }

  public Subject(String name, String[] masksStrings, Sme defaultSme, String notes) throws SibincoException
  {
    this.notes = notes;
    if (name == null)
      throw new NullPointerException("Name is null");
    if (masksStrings == null)
      throw new NullPointerException("Masks is null");
    if (defaultSme == null)
      throw new NullPointerException("DefaultSme is null");

    this.name = name;
    masks = new MaskList(masksStrings);
    if (masks.size() == 0)
      throw new SibincoException("Masks is empty");
    this.defaultSme = defaultSme;
  }

  public Subject(String name, Collection masksStrings, Sme defaultSme, String notes) throws SibincoException
  {
    this.notes = notes;
    if (name == null)
      throw new NullPointerException("Name is null");
    if (masksStrings == null)
      throw new NullPointerException("Masks is null");
    if (defaultSme == null)
      throw new NullPointerException("DefaultSme is null");

    this.name = name;
    masks = new MaskList(masksStrings);
    if (masks.size() == 0)
      throw new NullPointerException("Masks is empty");
    this.defaultSme = defaultSme;
  }

  public String getName()
  {
    return name;
  }

  public String getId()
  {
    return name;
  }

  public MaskList getMasks()
  {
    return masks;
  }

  public Sme getDefaultSme()
  {
    return defaultSme;
  }

  public void setDefaultSme(Sme defaultSme)
  {
    this.defaultSme = defaultSme;
  }

  public void setMasks(MaskList masks)
  {
    this.masks = masks;
  }

  public PrintWriter store(PrintWriter out)
  {
    out.println("  <subject_def id=\"" + StringEncoderDecoder.encode(getName()) + "\" defSme=\"" + StringEncoderDecoder.encode(getDefaultSme().getId()) + "\">");
    if (notes != null)
      out.println("    <notes>" + notes + "</notes>");
    getMasks().store(out);
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
