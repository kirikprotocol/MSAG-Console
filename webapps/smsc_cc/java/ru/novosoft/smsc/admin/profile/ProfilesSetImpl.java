package ru.novosoft.smsc.admin.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.IOUtils;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Реализация ProfilesSet
 */
class ProfilesSetImpl implements ProfilesSet {

  private final InputStream is;
  private final byte buf[];
  private final int msgSize1;
  private final int version;
  private final boolean smsx;

  public ProfilesSetImpl(InputStream is, boolean smsx) throws AdminException {
    try {
      this.is = is;
      this.smsx = smsx;
      IOUtils.readString(is, 8); // skip magic
      version = (int) IOUtils.readUInt32(is);
      if (version == 0x00010000)
        msgSize1 = 117;
      else if (version >= 0x00010100)
        msgSize1 = 129 + 4 + 1 + 10;
      else
        msgSize1 = (smsx) ? 129 + 4 + 1 : 129;
      buf = new byte[msgSize1 - 1];
    } catch (IOException e) {
      throw new ProfileException("broken.profiles.file", e);
    }
  }

  public boolean next() {
    try {
      while (IOUtils.readUInt8(is) == 0)
        IOUtils.skip(is, msgSize1 - 1);

      IOUtils.readFully(is, buf);
      return true;

    } catch (Exception e) {
      return false;
    }
  }

  public Profile get() {
    ByteArrayInputStream bis = new ByteArrayInputStream(buf);
    Profile p = new Profile();
    try {
      IOUtils.readString(bis, 8); // skip magic
      byte type = (byte) IOUtils.readUInt8(bis);
      byte plan = (byte) IOUtils.readUInt8(bis);

      String address = IOUtils.readString(bis, 21);
      p.setAddress(new Address(type, plan, address));
      int codepage = (int) IOUtils.readUInt32(bis);
      p.setLatin1((codepage & 1) == 1);
      p.setUcs2((codepage & 8) == 8);
      p.setUssd7bit((codepage & 128) == 128);

      p.setReportOptions(getReportOption((int) IOUtils.readUInt32(bis)));

      p.setAliasHide(getAliasHide((int) IOUtils.readUInt32(bis)));

      p.setLocale(getCleanString(IOUtils.readString(bis, 32)));

      short hideModifiable = (short) IOUtils.readUInt8(bis); //f.WriteByte(hideModifiable);
      p.setAliasModifiable(hideModifiable != 0);

      p.setDivert(IOUtils.readString(bis, 32));

      p.setDivertActive(IOUtils.readUInt8(bis) != 0);
      p.setDivertActiveAbsent(IOUtils.readUInt8(bis) != 0);
      p.setDivertActiveBlocked(IOUtils.readUInt8(bis) != 0);
      p.setDivertActiveBarred(IOUtils.readUInt8(bis) != 0);
      p.setDivertActiveCapacity(IOUtils.readUInt8(bis) != 0);
      p.setDivertModifiable(IOUtils.readUInt8(bis) != 0);

      p.setUdhConcat(IOUtils.readUInt8(bis) != 0);
      p.setTranslit(IOUtils.readUInt8(bis) != 0);

      if (version > 0x00010000) {
        int groupId = (int) IOUtils.readUInt32(bis);
        if (groupId > 0)
          p.setGroupId(groupId);
        p.setInputAccessMask((int) IOUtils.readUInt32(bis));
        p.setOutputAccessMask((int) IOUtils.readUInt32(bis));
        if (smsx) {
          p.setSubscription((int) IOUtils.readUInt32(bis));          
          p.setSponsored((byte) IOUtils.readUInt8(bis));
        }
      }
      if (version >= 0x00010100 && smsx) {
        p.setNick(getCleanString(IOUtils.readString(bis, 10)));
      }

      return p;
    } catch (IOException e) {
      return null;
    } catch (AdminException e) {
      return null; // todo
    }
  }

  private String getCleanString(String str) {
    String result = "";
    if (str != null) {
      int len = str.indexOf("\u0000");
      if (len != -1) {
        result = str.substring(0, len);
      } else {
        result = str;
      }
    }
    return result;
  }

  private ReportOption getReportOption(final int reportOptions) {
    switch (reportOptions) {
      case 1:
        return ReportOption.FULL;
      case 3:
        return ReportOption.FINAL;
      default:
        return ReportOption.NONE;
    }
  }

  private AliasHide getAliasHide(int hide) {
    switch (hide) {
      case 0:
        return AliasHide.FALSE;
      case 1:
        return AliasHide.TRUE;
      default:
        return AliasHide.SUBSTITUTE;
    }
  }

  public void close() throws AdminException {
    try {
      is.close();
    } catch (IOException e) {
    }
  }
}
