/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.novosoft.smsc.admin.profiler;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.profile.ProfileQuery;
import ru.novosoft.smsc.jsp.util.tables.impl.profile.ProfileDataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsview.archive.Message;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.WebAppFolders;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.io.EOFException;
import java.io.File;
import java.util.Date;
import java.util.Collections;

/**
 * Created by IntelliJ IDEA.
 * User: igor
 * Date: May 23, 2005
 * Time: 1:06:10 PM
 * To change this template use File | Settings | File Templates.
 */
public class ProfileDataFile {

    Category logger = Category.getInstance(ProfileDataFile.class);
    private String profilerStorePath;
    private static final String[] columnNames = {"mask", "codeset", "reportinfo", "locale", "alias_hide", "Hide modifiable", "divert", "divert_act", "divert_mod", "ussd7bit"};

    private static final String PARAM_NAME_FILE_NAME = "profiler.storeFile";

    public void init(Config config, String configPath) throws AdminException {

        try {
            profilerStorePath = config.getString(PARAM_NAME_FILE_NAME);
            if (profilerStorePath == null || profilerStorePath.length() <= 0)
                throw new AdminException("store path is empty");
        } catch (Exception e) {
            throw new AdminException("Failed to obtain " + PARAM_NAME_FILE_NAME + " Details: " + e.getMessage());
        }
        File statPath = new File(profilerStorePath);
        if (statPath == null || !statPath.isAbsolute()) {
            File smscConfFile = WebAppFolders.getSmscConfFolder();
            String absolutePath = smscConfFile.getParent();
            statPath = new File(absolutePath, profilerStorePath);
            logger.debug("Sms store path: by smsc conf '" + statPath.getAbsolutePath() + "'");
        } else {
            logger.debug("Sms store path: is absolute '" + statPath.getAbsolutePath() + "'");
        }
        profilerStorePath = statPath.getAbsolutePath();

        FileInputStream input = null;
        try {
            input = new FileInputStream(profilerStorePath);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            throw new AdminException(e.getMessage());
        }
    }

    public QueryResultSet query(ProfileQuery query_to_run) throws AdminException {
        String sort = query_to_run.getSortOrder();
        QueryResultSetImpl results = null;
        byte show = query_to_run.getShow();
        String queryFilter = null;
        int totalCount = 0;
        boolean isLast = true;
        queryFilter = getFilterQuery(show, query_to_run);
        results = new QueryResultSetImpl(columnNames, query_to_run.getSortOrder());
        FileInputStream fis = null;
        System.out.println("start reading File in: " + new Date());
        long tm = System.currentTimeMillis();
        try {
            fis = new FileInputStream(profilerStorePath);
            byte buf[] = new byte[256 * 1024];

            String FileName = Message.readString(fis, 8);
            int version = (int) Message.readUInt32(fis);
            while (true) {
                int msgSize1 = 117;
                readBuffer(fis, buf, msgSize1);
                InputStream bis = new ByteArrayInputStream(buf, 0, msgSize1);

                int used = (int) Message.readUInt8(bis);
                String profMagic = Message.readString(bis, 8);
                byte type = (byte) Message.readUInt8(bis); //f.WriteByte(addr.type);
                byte plan = (byte) Message.readUInt8(bis); //f.WriteByte(addr.plan);

                String address = Message.readString(bis, 21);//f.Write(buf,sizeof(buf)); sizeof(AddressValue)==21
                Mask mask = new Mask(type, plan, address);
                int codepage = (int) Message.readUInt32(bis);//f.WriteNetInt32(codepage);
                int reportoptions = (int) Message.readUInt32(bis);//f.WriteNetInt32(reportoptions);
                int hide = (int) Message.readUInt32(bis);//f.WriteNetInt32(hide);

                String locale = getCleanString(Message.readString(bis, 32));
                short hideModifiable = (short) Message.readUInt8(bis); //f.WriteByte(hideModifiable);

                String divert = Message.readString(bis, 32);

                short divertActive = (short) Message.readUInt8(bis);// f.WriteByte(divertActive);
                short divertActiveAbsent = (short) Message.readUInt8(bis);//f.WriteByte(divertActiveAbsent);
                short divertActiveBlocked = (short) Message.readUInt8(bis);//f.WriteByte(divertActiveBlocked);
                short divertActiveBarred = (short) Message.readUInt8(bis);//f.WriteByte(divertActiveBarred);
                short divertActiveCapacity = (short) Message.readUInt8(bis);//f.WriteByte(divertActiveCapacity);
                short divertModifiable = (short) Message.readUInt8(bis);//f.WriteByte(divertModifiable);
                short udhconcat = (short) Message.readUInt8(bis);//f.WriteByte(udhconcat);
                short translit = (short) Message.readUInt8(bis);//f.WriteByte(translit);

                if (used == 1 && isAddProfile(mask, queryFilter, show)) {
                    String divertActiveStr = String.valueOf(divertActive != 0);
                    String divertActiveAbsentStr = String.valueOf(divertActiveAbsent != 0);
                    String divertActiveBlockedStr = String.valueOf(divertActiveBlocked != 0);
                    String divertActiveBarredStr = String.valueOf(divertActiveBarred != 0);
                    String divertActiveCapacityStr = String.valueOf(divertActiveCapacity != 0);

                    String divertResult = getDivertActive(new Boolean(divertActiveStr).booleanValue(),
                            new Boolean(divertActiveAbsentStr).booleanValue(),
                            new Boolean(divertActiveBlockedStr).booleanValue(),
                            new Boolean(divertActiveBarredStr).booleanValue(),
                            new Boolean(divertActiveCapacityStr).booleanValue());

                    Profile profile = setProfile(mask, codepage, reportoptions,
                            locale, hide, hideModifiable, divert, divertResult,
                            divertModifiable, udhconcat, translit);

                    results.add(new ProfileDataItem(profile));
                    totalCount++;
                    isLast = false;
                }
                bis.close();
            }
        } catch (EOFException e) {
        } catch (Exception e) {
            logger.error("Unexpected exception occured reading operative store file", e);
        } finally {
            if (fis != null)
                try {
                    fis.close();
                } catch (IOException e) {
                    logger.warn("can't close file");
                }
        }

        results.setTotalSize(totalCount);
        results.setLast(isLast);
        Collections.sort(results.getSortOrder());
        System.out.println("end reading File in: " + new Date() + " spent: " + ((System.currentTimeMillis() - tm) / 1000) + " sec. ");

        if (results != null)
            return results;
        else
            throw new AdminException("Couldn't retrieve data from profiles file");
    }

    private String getFilterQuery(byte show, ProfileQuery query_to_run) {
        String queryFilter;
        switch (show) {
            case ProfileQuery.SHOW_ADDRESSES:
                queryFilter = query_to_run.getFilter();
                break;
            case ProfileQuery.SHOW_MASKS:
                queryFilter = "?";
                break;
            default:
                queryFilter = query_to_run.getFilter();
                System.out.println("Profile filtered: show all");
        }
        return queryFilter;
    }

    private boolean isAddProfile(Mask mask, String queryFilter, byte show) {
        boolean filter;
        if (show == ProfileQuery.SHOW_ADDRESSES) {
            filter = mask.getNormalizedMask().startsWith(queryFilter);

        } else if (show == ProfileQuery.SHOW_MASKS) {
            filter = mask.getNormalizedMask().indexOf(queryFilter) != -1;
        } else {
            filter = true;
        }
        return filter;
    }


    public String getProfilerStorePath() {
        return profilerStorePath;
    }


    private static void readBuffer(InputStream is, byte buffer[], int size) throws IOException {
        int read = 0;
        while (read < size) {
            int result = is.read(buffer, read, size - read);
            if (result < 0) throw new EOFException("Failed to read " + size + " bytes, read failed at " + read);
            read += result;
        }
    }


    private static String getCleanString(String str) {
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

    private String getDivertActive(boolean divertActiveUnconditional,
                                   boolean divertActiveAbsent,
                                   boolean divertActiveBlocked,
                                   boolean divertActiveBarred,
                                   boolean divertActiveCapacity) {
        final StringBuffer result = new StringBuffer(5);
        result.append(divertActiveUnconditional ? 'Y' : 'N');
        result.append(divertActiveAbsent ? 'Y' : 'N');
        result.append(divertActiveBlocked ? 'Y' : 'N');
        result.append(divertActiveBarred ? 'Y' : 'N');
        result.append(divertActiveCapacity ? 'Y' : 'N');
        return result.toString();
    }

    private Profile setProfile(Mask mask, int codepage,
                               int reportoptions,
                               String locale, int hide,
                               short hideModifiable, String divert,
                               String divertResult, short divertModifiable,
                               short udhconcat, short translit) throws AdminException {
        Profile profile = new Profile(mask,
                Profile.getCodepageString((byte) ((byte) codepage & 0x7F)),
                String.valueOf((codepage & 0x80) != 0),
                Profile.getReportOptionsString((byte) reportoptions),
                locale,
                String.valueOf((hide) != 0),
                String.valueOf((hideModifiable) != 0),
                divert,
                divertResult,
                String.valueOf((divertModifiable) != 0),
                String.valueOf((udhconcat) != 0),
                String.valueOf((translit) != 0));
        return profile;
    }


}
