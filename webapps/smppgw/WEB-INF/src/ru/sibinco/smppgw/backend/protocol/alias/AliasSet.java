package ru.sibinco.smppgw.backend.protocol.alias;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 16:28:35
 * To change this template use File | Settings | File Templates.
 */

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.smppgw.backend.SmppGWAppContext;
import ru.sibinco.smppgw.backend.protocol.journal.SubjectTypes;
import ru.sibinco.smppgw.backend.protocol.journal.Actions;
import ru.sibinco.smppgw.backend.protocol.journal.Action;
import ru.sibinco.smppgw.backend.protocol.tables.impl.alias.AliasDataSource;
import ru.sibinco.smppgw.backend.protocol.tables.impl.alias.AliasQuery;
import ru.sibinco.smppgw.backend.protocol.tables.QueryResultSet;

import java.io.PrintWriter;
import java.util.*;


public class AliasSet
{
  private Set aliases = new HashSet();
  private AliasDataSource dataSource = new AliasDataSource();
  private Category logger = Category.getInstance(this.getClass());
  private final SmppGWAppContext smppGWAppContext;

  public AliasSet(final Element aliasesElem, final SmppGWAppContext smppGWAppContext)
  {
    this.smppGWAppContext = smppGWAppContext;
    final NodeList aliasNodes = aliasesElem.getElementsByTagName("record");
    for (int i = 0; i < aliasNodes.getLength(); i++) {
      final Element aliasElem = (Element) aliasNodes.item(i);
      try {
        add(new Alias(new Mask(aliasElem.getAttribute("addr")),
                      new Mask(aliasElem.getAttribute("alias")),
                      "true".equalsIgnoreCase(aliasElem.getAttribute("hide"))));
      } catch (SibincoException e) {
        logger.error("Couldn't load alias \"" + aliasElem.getAttribute("alias") + "\"-->\"" + aliasElem.getAttribute("addr") + "\", skipped", e);
      }
    }
  }

  public PrintWriter store(final PrintWriter out)
  {
    for (Iterator i = iterator(); i.hasNext();) {
      final Alias a = (Alias) i.next();
      out.println("  <record addr=\"" + StringEncoderDecoder.encode(a.getAddress().getMask()) + "\" alias=\""
                  + StringEncoderDecoder.encode(a.getAlias().getMask())
                  + "\" hide=\""
                  + (a.isHide() ? "true" : "false")
                  + "\"/>");
    }
    return out;
  }

  public boolean add(final Alias newAlias, final Action action)
  {
    if (add(newAlias)) {
      action.setAction(Actions.ACTION_ADD);
      action.setSubjectType(SubjectTypes.TYPE_alias);
      action.setSubjectId(newAlias.getAlias().getMask());
      smppGWAppContext.getJournal().append(action);
      smppGWAppContext.getStatuses().setAliasesChanged(true);
      return true;
    } else
      return false;
  }

  private boolean add(final Alias new_alias)
  {
    if (aliases.contains(new_alias))
      return false;

    if (new_alias.isHide())
      for (Iterator i = aliases.iterator(); i.hasNext();) {
        final Alias alias = (Alias) i.next();
        if (alias.isHide() && alias.getAddress().equals(new_alias.getAddress()))
          return false;
      }

    dataSource.add(new_alias);
    return aliases.add(new_alias);
  }

  public Iterator iterator()
  {
    return aliases.iterator();
  }

  public boolean remove(final Alias a)
  {
    dataSource.remove(a);
    return aliases.remove(a);
  }

  public boolean remove(final String alias)
  {
    try {
      final Alias a = new Alias(new Mask(alias), new Mask(alias), false);
      return remove(a);
    } catch (SibincoException e) {
      logger.error("Couldn't remove alias \"" + alias + '"', e);
      return false;
    }
  }

  public boolean modify(final Alias oldAlias, final Alias newAlias, final Action action)
  {
    assert null != oldAlias && null != newAlias && null != action;
    action.setAction(Actions.ACTION_MODIFY);
    action.setSubjectType(SubjectTypes.TYPE_alias);
    action.setSubjectId(newAlias.getAlias().getMask());
    if (remove(oldAlias) && !oldAlias.getAlias().equals(newAlias.getAlias()))
      action.setAdditionalValue("old alias", oldAlias.getAlias().getMask());
    if (add(newAlias)) {
      smppGWAppContext.getJournal().append(action);
      smppGWAppContext.getStatuses().setAliasesChanged(true);
      return true;
    } else
      return false;
  }
/*
  public QueryResultSet query(final AliasQuery query)
  {
    dataSource.clear();
    for (Iterator i = aliases.iterator(); i.hasNext();) {
      final Alias alias = (Alias) i.next();
      dataSource.add(alias);
    }
    return dataSource.query(query);
  }
    */
  public boolean contains(final Alias a)
  {
    return aliases.contains(a);
  }

  public Alias get(final String aliasString)
  {
    for (Iterator i = aliases.iterator(); i.hasNext();) {
      final Alias alias = (Alias) i.next();
      if (alias.getAlias().getMask().equals(aliasString))
        return alias;
    }
    return null;
  }

  public int size()
  {
    return aliases.size();
  }

  public boolean isContainsAlias(final Mask aliasMask)
  {
    for (Iterator i = aliases.iterator(); i.hasNext();) {
      final Alias alias = (Alias) i.next();
      if (alias.getAlias().addressConfirm(aliasMask))
        return true;
    }
    return false;
  }

  public Alias getAliasByAddress(final Mask address)
  {
    Alias result = null;
    for (Iterator i = aliases.iterator(); i.hasNext();) {
      final Alias alias = (Alias) i.next();
      if (alias.isHide()) {
        if (alias.getAddress().addressConfirm(address)) {
          if (null == result)
            result = alias;
          else if (alias.getAddress().getQuestionsCount() < result.getAddress().getQuestionsCount())
            result = alias;
        }
      }
    }
    return result;
  }

  public Alias getAddressByAlias(final Mask aliasToSearch)
  {
    Alias result = null;
    for (Iterator i = aliases.iterator(); i.hasNext();) {
      final Alias alias = (Alias) i.next();
      if (alias.getAlias().addressConfirm(aliasToSearch)) {
        if (null == result)
          result = alias;
        else if (alias.getAlias().getQuestionsCount() < result.getAlias().getQuestionsCount())
          result = alias;
      }
    }
    return result;
  }

  public Mask dealias(final Mask alias) throws SibincoException
  {
    final Alias addressCandidat = getAddressByAlias(alias);
    if (null != addressCandidat) {
      final Mask result = addressCandidat.getAddress();
      final int questionsCount = result.getQuestionsCount();
      if (0 < questionsCount) {
        final String mask = result.getMask();
        final String sourceMask = alias.getMask();
        return new Mask(mask.substring(0, mask.length() - questionsCount) + sourceMask.substring(sourceMask.length() - questionsCount));
      } else
        return result;
    } else
      return null;
  }

  public boolean isAddressExistsAndHide(final Mask address, final Alias except)
  {
    for (Iterator i = iterator(); i.hasNext();) {
      final Alias alias = (Alias) i.next();
      if (alias.isHide() && alias.getAddress().equals(address) && null != except && !except.equals(alias))
        return true;
    }
    return false;
  }
}
