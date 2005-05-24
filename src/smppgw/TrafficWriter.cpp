#include <stdio.h>
#include <string>
#include <time.h>
#include "TrafficWriter.h"
#include <netinet/in.h>
#include <memory>
#include "util/Exception.hpp"
#include "logger/Logger.h"
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <errno.h>
#include <core/buffers/TmpBuf.hpp>
#include <sys/stat.h>

namespace smsc {
namespace smppgw {

using namespace smsc::sms;
using smsc::util::Exception;
using smsc::core::buffers::TmpBuf;

void twdump(TrafficWriter::routData rd){
    smsc::logger::Logger *log=smsc::logger::Logger::getInstance("tw.dump");

    uint16_t netYear;
    memcpy((void*)&netYear, (const void*)rd.date, 2);
	uint16_t year = ntohs(netYear);
							    
    int32_t of;
	memcpy((void*)&of, (const void*)rd.offset_, 4);
	int32_t offset = ntohl(of);
	char tmp[512];
	sprintf(tmp, "rout: %s, offset: %d, y: %d, m: %d, d: %d, h: %d", rd.routId, offset, year, rd.date[2], rd.date[3], rd.date[4]);
	smsc_log_info(log, tmp );
														
	int32_t cnt = 0;
	memcpy((void*)&cnt, (const void*)rd.acceptedCnt, 4);
	int32_t counter = ntohl(cnt);
	sprintf(tmp, "counter1: %d", counter);
	smsc_log_info(log, tmp );
																					
	memcpy((void*)&cnt, (const void*)(rd.acceptedCnt + 4), 4);
	counter = ntohl(cnt);
	sprintf(tmp, "counter2: %d", counter);
	smsc_log_info(log, tmp );
																									    
	memcpy((void*)&cnt, (const void*)(rd.acceptedCnt + 8), 4);
	counter = ntohl(cnt);
	sprintf(tmp, "counter3: %d", counter);
	smsc_log_info(log, tmp );
																														
	memcpy((void*)&cnt, (const void*)(rd.acceptedCnt + 719*4), 4);
	counter = ntohl(cnt);
	sprintf(tmp, "counter720: %d", counter);
	smsc_log_info(log, tmp );
																																		    
	memcpy((void*)&cnt, (const void*)(rd.acceptedCnt + 718*4), 4);
	counter = ntohl(cnt);
	sprintf(tmp, "counter719: %d", counter);
	smsc_log_info(log, tmp );
																																							
	memcpy((void*)&cnt, (const void*)(rd.acceptedCnt + 717*4), 4);
	counter = ntohl(cnt);
	sprintf(tmp, "counter718: %d", counter);
	smsc_log_info(log, tmp );
}

TrafficWriter::TrafficWriter(const char* location_):
	location(location_),
	cfPtr(NULL)
{
	filename = "traffic.dat";
}

TrafficWriter::TrafficWriter() :
	location(""),
	cfPtr(NULL)
{
	filename = "traffic.dat";
}

TrafficWriter::~TrafficWriter()
{
}

void TrafficWriter::newRec(const char* routId, int& offset, time_t dt)
{
	routData rd(routId, dt);
	
	std::string pathname = location; pathname += '/'; pathname += filename;
	if( ( cfPtr = fopen(pathname.c_str(), "ab+") ) == NULL)
		throw Exception("Can't open traffic file.");
	else {	
		offset = 0;
		if(fseek(cfPtr, 0, SEEK_END) != 0){
            int err = ferror(cfPtr);
            throw Exception("Can't seek offset position in traffic file. Details: %s", strerror(err));
        }
		fpos_t pos;
		if(fgetpos(cfPtr, &pos) != 0){
            int err = ferror(cfPtr);
            throw Exception("Can't get current position of traffic file. Details: %s", strerror(err));
        }

        #ifdef linux
            long long tmp = pos.__pos;
        #else
            long long tmp = pos;
        #endif
        offset = tmp;
		
		offset += (MAX_ROUTE_ID_TYPE_LENGTH + 9);
		rd.offset(offset);
		
		if(fwrite(&rd, sizeof(struct routData), 1, cfPtr) != 1){
            int err = ferror(cfPtr);
            throw Exception("Can't write into traffic file. Details: %s", strerror(err));
        }
		fclose(cfPtr);
	}
	cfPtr = NULL;	
}

void TrafficWriter::updateRec(int offset, int delay, int value, time_t dt)
{

	int hour = offset/2921;
	hour = (offset - hour*2921 - 41)/4;

	std::string pathname = location; pathname += '/'; pathname += filename;
	if( ( cfPtr = fopen(pathname.c_str(), "rb+") ) == NULL)
		throw Exception("Can't open traffic file.");
	else {
	
		// Writes offset and date.
		int sz = 4;	sz += delay > 0 ? 5 : 0;
		int skip = delay > 0 ? 5 : 0;
		std::auto_ptr<char> head_buff( new char[sz] );
		int32_t val = 0;
		val = htonl(offset);	memcpy((void*)(head_buff.get() + skip), (const void*)&val, 4);
		
		if(delay > 0){
			char date[5];
			tm tmDate;
			localtime_r(&dt, &tmDate);
			uint16_t year = tmDate.tm_year + 1900;
			char month = tmDate.tm_mon + 1;
			char day = tmDate.tm_mday;
			char hour = tmDate.tm_hour;
			uint16_t netYear = htons(year);
									
			memcpy((void*)date, (const void*)&netYear, 2);
			date[2] = month;
			date[3] = day;
			date[4] = hour;
			
			memcpy((void*)head_buff.get(), (const void*)date, 5);
		}
		
		int head_offset = offset - 4*hour - 9;	head_offset += delay == 0 ? 5 : 0;
		if(fseek(cfPtr, head_offset, SEEK_SET) != 0){
            int err = ferror(cfPtr);
            throw Exception("Can't seek offset position in traffic file. Details: %s", strerror(err));
        }
		if(fwrite(head_buff.get(), sz, 1, cfPtr) != 1){
            int err = ferror(cfPtr);
            throw Exception("Can't write into traffic file. Details: %s", strerror(err));
        }
		
		// Start hour's counters update
		if(delay - 1 > hour){
			int sz = 4*(hour + 1),	SZ = hour + 1;
			std::auto_ptr<int32_t> cnt_buff( new int32_t[SZ] );
			
			int reset = 0; val = htonl(reset);
			for(int i=0; i <= hour-1; i++)
				memcpy((void*)(cnt_buff.get() + i), (const void*)&val, 4);
			
			val = htonl(value);	memcpy((void*)(cnt_buff.get() + hour), (const void*)&val, 4);
			
			int cnt_offset = offset - 4*hour;
			if(fseek(cfPtr, cnt_offset, SEEK_SET) != 0){
                int err = ferror(cfPtr);
                throw Exception("Can't seek offset position in traffic file. Details: %s", strerror(err));
            }
			if(fwrite((const void*)cnt_buff.get(), sz, 1, cfPtr) != 1){
                int err = ferror(cfPtr);
                throw Exception("Can't write into traffic file. Details: %s", strerror(err));
            }
			
		}else{
			int sz = 4;	sz += delay > 1 ? 4*(delay - 1) : 0; 
			int SZ = sz/4;
			std::auto_ptr<int32_t> cnt_buff( new int32_t[SZ] );
			if(delay > 1){
				int reset = 0; val = htonl(reset);
				for(int i=0; i <= delay-2; i++ )
					memcpy((void*)(cnt_buff.get() + i), (const void*)&val, 4);
			}
			int skip = delay > 1 ? delay - 1 : 0;
			val = htonl(value);     memcpy((void*)(cnt_buff.get() + skip), (const void*)&val, 4);
			
			int cnt_offset = offset; cnt_offset -= delay > 1 ? 4*(delay - 1) : 0;
			if(fseek(cfPtr, cnt_offset, SEEK_SET) != 0){
                int err = ferror(cfPtr);
                throw Exception("Can't seek offset position in traffic file. Details: %s", strerror(err));
            }
			if(fwrite((const void*)cnt_buff.get(), sz, 1, cfPtr) != 1){
                int err = ferror(cfPtr);
                throw Exception("Can't write into traffic file. Details: %s", strerror(err));
            }
		}
		
		// Last hour's counters update
		if(delay - 1 > hour){
			if(720 < delay - 1){
				if(hour < 719){
					int sz = 4*(719 - hour),	SZ = (719 - hour);
					std::auto_ptr<int32_t> cnt_buff( new int32_t[SZ] );
					int reset = 0;	val = htonl(reset);
					for(int i=0; i <= SZ-1; i++)
						memcpy((void*)(cnt_buff.get() + i), (const void*)&val, 4);
					
					int cnt_offset = offset + 4;
					if(fseek(cfPtr, cnt_offset, SEEK_SET) != 0){
                        int err = ferror(cfPtr);
                        throw Exception("Can't seek offset position in traffic file. Details: %s", strerror(err));
                    }
					if(fwrite((const void*)cnt_buff.get(), sz, 1, cfPtr) != 1){
                        int err = ferror(cfPtr);
                        throw Exception("Can't write into traffic file. Details: %s", strerror(err));
                    }
				}
			}else{
				int sz = 4*(delay - 1 - hour),	SZ = delay - 1 - hour; int skip = delay == 721 ? 1 : 0;
				sz -= skip*4;			SZ -= skip;
				if(SZ > 0){
					std::auto_ptr<int32_t> cnt_buff( new int32_t[SZ] );
					int reset =0; val = htonl(reset);
					for(int i=0; i<=SZ-1; i++)
						memcpy((void*)(cnt_buff.get() + i), (const void*)&val, 4);
					
					int cnt_offset = offset + 4*(721 - delay); cnt_offset += delay == 721 ? 4 : 0;
					if(fseek(cfPtr, cnt_offset, SEEK_SET) != 0){
                        int err = ferror(cfPtr);
                        throw Exception("Can't seek offset position in traffic file. Details: %s", strerror(err));
                    }
					if(fwrite((const void*)cnt_buff.get(), sz, 1, cfPtr) != 1){
                        int err = ferror(cfPtr);
                        throw Exception("Can't write into traffic file. Details: %s", strerror(err));
                    }
				}
			}
		}

		fclose(cfPtr);
	}
	cfPtr = NULL;
}

int TrafficWriter::readAt(int pos, routData* rd){

    if(cfPtr){
		if(fseek(cfPtr, pos, SEEK_SET) != 0){
            int err = ferror(cfPtr);
            throw Exception("Can't seek offset position in traffic file. Details: %s", strerror(err));
        }
		int sz = sizeof(struct routData);
        int read_size = fread(rd, sz, 1, cfPtr);
		if( ( read_size != 1 ) && ( read_size != 0) ){
            int err = ferror(cfPtr);
            throw Exception("Can't read route data from traffic file. Details: %s", strerror(err));
        }
		if(feof(cfPtr)){
			fclose(cfPtr);
		    cfPtr = 0;
			return -1;
		}
		return pos + sz;
		
	}	

	std::string pathname = location; pathname += '/'; pathname += filename;
	if( ( cfPtr = fopen(pathname.c_str(), "ab+") ) == NULL)
		throw Exception("Can't open traffic file.");
	else {
		if(fseek(cfPtr, pos, SEEK_SET) != 0){
            int err = ferror(cfPtr);
            throw Exception("Can't seek offset position in traffic file. Details: %s", strerror(err));
        }
		int sz = sizeof(struct routData);
        size_t read_size = fread(rd, sz, 1, cfPtr);
		if( ( read_size != 1 ) && ( read_size != 0 ) ){
            int err = ferror(cfPtr);
            throw Exception("Can't read route data from traffic file. Details: %s", strerror(err));
        }
		if(feof(cfPtr)){
			fclose(cfPtr);
			cfPtr = 0;
			return -1;
		}
		return pos + sz;
	}
}

void TrafficWriter::setLocation(const char* loc)
{
	location = loc;

    if(!createTrafficDir())
        throw Exception("Can't create dirrectory: '%s'", location.c_str());
}

void TrafficWriter::updateRec(int offset, const struct routData rd){

    std::string pathname = location; pathname += '/'; pathname += filename;
	if( ( cfPtr = fopen(pathname.c_str(), "rb+") ) == NULL){
		throw Exception("Can't open traffic file.");
	}else {
        if(fseek(cfPtr, offset, SEEK_SET) != 0){
            int err = ferror(cfPtr);
            throw Exception("Can't seek offset position in traffic file. Details: %s", strerror(err));
        }
        if(fwrite(&rd, sizeof(struct routData), 1, cfPtr) != 1){
            int err = ferror(cfPtr);
            throw Exception("Can't write into traffic file. Details: %s", strerror(err));
        }

        struct routData routeData;
        if(fseek(cfPtr, offset, SEEK_SET) != 0){
            int err = ferror(cfPtr);
            throw Exception("Can't seek offset position in traffic file. Details: %s", strerror(err));
        }
        if(fread(&routeData, sizeof(struct routData), 1, cfPtr) != 1){
            int err = ferror(cfPtr);
            throw Exception("Can't read from traffic file. Details: %s", strerror(err));
        }
        fclose(cfPtr);
    }

}

bool TrafficWriter::createTrafficDir()
{
    int len = strlen(location.c_str());
    if(len == 0)
        return false;

    if(strcmp(location.c_str(), "/") == 0)
        return true;

    ++len;
    TmpBuf<char, 512> tmpBuff(len);
    char* buff = tmpBuff.get();
    memcpy(buff, location.c_str(), len);
    if(buff[len-2] == '/'){
       buff[len-2] = 0;
       if(len > 2){
          if(buff[len-3] == '/'){
              return false;
           }
       }
    }

    std::vector<char*> dirs(0);

    char* p1 = buff+1;
    int dirlen = 0;
    char* p2 = strchr(p1, '/');
    int pos = p2 - buff;
    while(p2){
       int len = p2 - p1;
       dirlen += len + 1;
       if(len == 0)
           return false;

       int direclen = dirlen + 1;
       TmpBuf<char, 512> tmpBuff(direclen);
       char * dir = tmpBuff.get();
       memcpy(dir, buff, dirlen);
       dir[dirlen] = 0;
       dirs.push_back(dir);

       p1 = p1 + len + 1;
       p2 = strchr(p1, '/');
    }
    dirs.push_back(buff);

    std::vector<char*>::iterator it = dirs.begin();
    for(it = dirs.begin(); it != dirs.end(); it++){

        DIR* dirp = opendir(*it);
        if(dirp){
            closedir(dirp);            
        }else{
            try{
                createDir(std::string(*it));
            }catch(...){
                return false;
            }
        }
    }
    
    return true;
}

bool TrafficWriter::createDir(const std::string& dir)
{
    if (mkdir(dir.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0) {
        if (errno == EEXIST) return false;
        throw Exception("Failed to create directory '%s'. Details: %s", 
                        dir.c_str(), strerror(errno));
    }
    return true;
}

void timeToCharPtr(time_t dt, char* date){
	tm tmDate;
	localtime_r(&dt, &tmDate);
	uint16_t year = tmDate.tm_year + 1900;
	char month = tmDate.tm_mon + 1;
	char day = tmDate.tm_mday;
	char hour = tmDate.tm_hour;
	uint16_t netYear = htonl(year);
	
	date[0] = (netYear >> 8) & 0xFF;
	date[1] = netYear & 0xFF;
	date[2] = month;
	date[3] = day;
	date[4] = hour;
}

}
}
