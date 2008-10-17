package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import org.apache.log4j.Logger;

import java.io.File;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.util.List;
import java.util.LinkedList;

import com.eyeline.utils.tree.radix.StringsRTree;
import com.eyeline.utils.tree.radix.Serializer;
import com.eyeline.utils.tree.radix.FileBasedStringsRTree;
import com.eyeline.utils.tree.radix.UnmodifiableRTree;
import com.eyeline.utils.IOUtils;

/**
 * author: alkhal
 */
@SuppressWarnings({"unchecked"})
class ReplyTreeHandler {

    private static Logger logger = Logger.getLogger(ReplyTreeHandler.class);
    private StringsRTree<List<Long>> stringsRTree;
    private String treeFileName;     

    public ReplyTreeHandler(StringsRTree<List<Long>> stringsRTree, String treeFileName) throws FileStatsException{
        if((stringsRTree==null)||(treeFileName==null)) {
            logger.error("Some arguments are null");
            throw new FileStatsException("Some arguments are null", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        this.stringsRTree = stringsRTree;
        this.treeFileName = treeFileName;

    }

    public void put(String key, long value) throws FileStatsException{
        if(logger.isInfoEnabled()) {
            logger.info("Put into tree entry: "+key+" "+value);
        }
        if(key==null) {
            logger.error("Some arguments are null");
            throw new FileStatsException("Some arguments are null", FileStatsException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        List<Long> previousValue = null;
        List<Long> nextValue = null;
        if((previousValue = stringsRTree.get(key))!=null) {
            nextValue = new LinkedList<Long>(previousValue);
        } else {
            nextValue = new LinkedList<Long>();
        }
        nextValue.add(value);
        stringsRTree.put(key, nextValue);
    }

    public void saveTree() {
        File file = new File(treeFileName);
        file.delete();
        FileBasedStringsRTree.createRTree(stringsRTree,treeFileName, new LongListSerializer());
        logger.info("Tree saved");
    }


    public List<Long> getValue(String key) {
        return stringsRTree.get(key);
    }


    FileBasedStringsRTree<List<Long>> creatFileBasedStringsRTree(String fileName) {
        return new FileBasedStringsRTree<List<Long>>(fileName, new LongListSerializer());
    }
    
    private void print() {
        System.out.println("Tree:");
        stringsRTree.printTree();
        System.out.println("");
    }

    private class LongListSerializer implements Serializer<List<Long>>{

        public void serialize(List<Long> list, byte[] bytes, int i) {
            byte len = (byte)list.size();
            byte[] b = new byte[8*len];
            int j=0;
            for(Long l:list) {
                IOUtils.writeLong(l,b,8*j);
                j++;
            }
            bytes[i]=len;
            System.arraycopy(b,0,bytes,i+1,8*len);

        }

        public List<Long> deserialize(byte[] bytes, int i) {
            List<Long> list = new LinkedList<Long>();
            int len = bytes[i];
            for(int j=0;j<len;j++) {
                list.add(IOUtils.readLong(bytes,8*j+i+1));
            }
            return list;
        }

        public int getSerializationSize(List<Long> list) {
            return 8*list.size()+1;

        }
    }
}
