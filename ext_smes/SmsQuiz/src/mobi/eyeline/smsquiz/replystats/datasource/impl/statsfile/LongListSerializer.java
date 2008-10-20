package mobi.eyeline.smsquiz.replystats.datasource.impl.statsfile;

import com.eyeline.utils.tree.radix.Serializer;
import com.eyeline.utils.IOUtils;

import java.util.List;
import java.util.LinkedList;

/**
 * author: alkhal
 */
   public class LongListSerializer implements Serializer<List<Long>> {

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

