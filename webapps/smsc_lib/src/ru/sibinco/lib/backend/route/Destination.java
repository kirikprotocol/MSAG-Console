/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:45:30
 */
package ru.sibinco.lib.backend.route;

import org.w3c.dom.Element;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.util.Map;


public class Destination extends Source
{
  private Sme sme = null;

  public Destination(Subject subj)
      throws SibincoException
  {
    this(subj, subj.getDefaultSme());
  }

  public Destination(Subject subj, Sme sme)
      throws SibincoException
  {
    super(subj);

    if ((this.sme = sme) == null)
      throw new SibincoException("Sme is null");
  }

  public Destination(Mask mask, Sme sme)
      throws SibincoException
  {
    super(mask);
    if ((this.sme = sme) == null)
      throw new SibincoException("Sme is null");
  }

  public Destination(Element dstElem, Map subjects, SmeManager smeManager)
      throws SibincoException
  {
    super(dstElem, subjects);
    sme = (Sme) smeManager.getSmes().get(dstElem.getAttribute("sme"));
    if (sme == null)
      throw new SibincoException("Unknown Sme \"" + dstElem.getAttribute("sme") + '"');
  }

  public Sme getSme()
  {
    return sme;
  }

  public void setSme(Sme sme)
  {
    this.sme = sme;
  }

  public PrintWriter store(PrintWriter out)
  {
    out.println("    <destination sme=\"" + StringEncoderDecoder.encode(getSme().getId()) + "\">");
    if (isSubject())
      out.println("      <subject id=\"" + StringEncoderDecoder.encode(subj.getName()) + "\"/>");
    else
      out.println("      <mask value=\"" + StringEncoderDecoder.encode(mask.getNormalizedMask()) + "\"/>");
    out.println("    </destination>");
    return out;
  }
}
