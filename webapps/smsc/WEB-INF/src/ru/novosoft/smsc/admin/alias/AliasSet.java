/*
 * Author: igork
 * Date: 05.06.2002
 * Time: 16:52:19
 */
package ru.novosoft.smsc.admin.alias;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasQuery;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;


public class AliasSet {
    private Set aliases = new HashSet();
    private AliasDataSource dataSource = new AliasDataSource();
    private Category logger = Category.getInstance(this.getClass());

    private String aliasmanStorePath;
    private static final String PARAM_NAME_FILE_NAME = "aliasman.storeFile";

    private Smsc smsc = null;

    public AliasSet() {
    }

    public void init(Config config, Smsc smsc) throws AdminException {
        try {
            aliasmanStorePath = config.getString(PARAM_NAME_FILE_NAME);
            if (aliasmanStorePath == null || aliasmanStorePath.length() <= 0)
                throw new AdminException("store path is empty");
        } catch (Exception e) {
            throw new AdminException("Failed to obtain " + PARAM_NAME_FILE_NAME + " Details: " + e.getMessage());
        }

        this.smsc = smsc;

        FileInputStream fis = null;
        try {
            fis = new FileInputStream(aliasmanStorePath);
            String FileName = Message.readString(fis, 8);
            int version = (int) Message.readUInt32(fis);
            int msgSizeLength = 48; //1+(1+1+21)+(1+1+21)+1
            byte buf[] = new byte[48];

            while (true) {
                int read = 0;
                while (read < msgSizeLength) {
                    int result = fis.read(buf, read, msgSizeLength - read);
                    if (result < 0) throw new EOFException("Failed to read " + msgSizeLength + " bytes, read failed at " + read);
                    read += result;
                }
                InputStream bis = new ByteArrayInputStream(buf, 0, msgSizeLength);

                byte isInUse = (byte) Message.readUInt8(bis);
                byte addrTon = (byte) Message.readUInt8(bis);
                byte addrNpi = (byte) Message.readUInt8(bis);
                String addrValue = Message.readString(bis, 21);
                byte aliasTon = (byte) Message.readUInt8(bis);
                byte aliasNpi = (byte) Message.readUInt8(bis);
                String aliasValue = Message.readString(bis, 21);
                byte isHide = (byte) Message.readUInt8(bis);

                if (isInUse != 0)
                    add(new Alias(new Mask(addrTon, addrNpi, addrValue), new Mask(aliasTon, aliasNpi, aliasValue), (isHide != 0)));

                bis.close();
            }
        } catch (EOFException e) {
        } catch (Exception e) {
            logger.error("Unexpected exception occured reading alias set file", e);
        } finally {
            if (fis != null)
                try {
                    fis.close();
                } catch (IOException e) {
                    logger.warn("can't close file");
                }
        }
    }

/*    public PrintWriter store(final PrintWriter out) {
        for (Iterator i = iterator(); i.hasNext();) {
            final Alias a = (Alias) i.next();
            out.println("  <record addr=\"" + StringEncoderDecoder.encode(a.getAddress().getMask()) + "\" alias=\""
                    + StringEncoderDecoder.encode(a.getAlias().getMask())
                    + "\" hide=\""
                    + (a.isHide() ? "true" : "false")
                    + "\"/>");
        }
        return out;
    }*/

    public boolean add(final Alias new_alias) {
        if (aliases.contains(new_alias))
            return false;

        if (new_alias.isHide())
            for (Iterator i = aliases.iterator(); i.hasNext();) {
                final Alias alias = (Alias) i.next();
                if (alias.isHide() && alias.getAddress().equals(new_alias.getAddress()))
                    return false;
            }

        try {
            smsc.addAlias(new_alias.getAddress().getMask(), new_alias.getAlias().getMask(), new_alias.isHide());
            dataSource.add(new_alias);
            return aliases.add(new_alias);
        }
        catch (AdminException e) {
            logger.error("Couldn't add alias \"" + new_alias.getAlias().getMask() + '"', e);
            return false;
        }
    }

    public Iterator iterator() {
        return aliases.iterator();
    }

    public boolean remove(final Alias a) {
        try {
            smsc.delAlias(a.getAlias().getMask());
            dataSource.remove(a);
            return aliases.remove(a);
        }
        catch (AdminException e) {
            logger.error("Couldn't remove alias \"" + a.getAlias().getMask() + '"', e);
            return false;
        }
    }

    public boolean remove(final String alias) {
        try {
            final Alias a = new Alias(new Mask(alias), new Mask(alias), false);
            return remove(a);
        } catch (AdminException e) {
            logger.error("Couldn't remove alias \"" + alias + '"', e);
            return false;
        }
    }

    public QueryResultSet query(final AliasQuery query) {
        dataSource.clear();
        for (Iterator i = aliases.iterator(); i.hasNext();) {
            final Alias alias = (Alias) i.next();
            dataSource.add(alias);
        }
        return dataSource.query(query);
    }

    public boolean contains(final Alias a) {
        return aliases.contains(a);
    }

    public Alias get(final String aliasString) {
        for (Iterator i = aliases.iterator(); i.hasNext();) {
            final Alias alias = (Alias) i.next();
            if (alias.getAlias().getMask().equals(aliasString))
                return alias;
        }
        return null;
    }

    public int size() {
        return aliases.size();
    }

    public boolean isContainsAlias(final Mask aliasMask) {
        for (Iterator i = aliases.iterator(); i.hasNext();) {
            final Alias alias = (Alias) i.next();
            if (alias.getAlias().addressConfirm(aliasMask))
                return true;
        }
        return false;
    }

    public Alias getAliasByAddress(final Mask address) {
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

    public Alias getAddressByAlias(final Mask aliasToSearch) {
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

    public Mask dealias(final Mask alias) throws AdminException {
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

    public boolean isAddressExistsAndHide(final Mask address, final Alias except) {
        for (Iterator i = iterator(); i.hasNext();) {
            final Alias alias = (Alias) i.next();
            if (alias.isHide() && alias.getAddress().equals(address) && null != except && !except.equals(alias))
                return true;
        }
        return false;
    }
}
