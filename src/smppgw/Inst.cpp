#include "Inst.h"
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <errno.h>


Inst::Inst() :
run_(false)
{
	filename = "/tmp/smppgw.pid";
	
	std::ifstream fin(filename.c_str());
	char oldpid[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	if(!fin.fail()){
		fin.read(oldpid, 20);
		run_ = !IsProcDir(oldpid);
	}else{
		run_ = true;
	}
	fin.close();
	
	if(run_){
		char _pid[20];
	        std::ofstream fout(filename.c_str());
		pid_t pid = getpid();
	        std::sprintf(_pid, "%d", pid);
		fout.write(_pid, std::strlen(_pid));
		fout.close();
	}
}

Inst::Inst(const char* fname) :
run_(false)
{
	filename = fname;
	
	std::ifstream fin(filename.c_str());
	char oldpid[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	if(!fin.fail()){
		fin.read(oldpid, 20);
		run_ = !IsProcDir(oldpid);
	}else{
		run_ = true;
	}
	fin.close();
	
	if(run_){
		char _pid[20];
	    std::ofstream fout(filename.c_str());
		pid_t pid = getpid();
	    std::sprintf(_pid, "%d", pid);
		fout.write(_pid, std::strlen(_pid));
		fout.close();
	}
}

Inst::~Inst()
{
	if(run_)
		int rt = remove(filename.c_str());
}

bool Inst::IsProcDir(char* oldpid)
{
	DIR* dirp = opendir("/proc");
	dirent *dp;
	
	while(dirp) {
	
		errno = 0;
		
		if((dp = readdir(dirp)) != NULL){
			if(std::strcmp(dp->d_name, oldpid) == 0){
				closedir(dirp);
				return true;
			}
		}else{
			if(errno == 0) {
				closedir(dirp);
				return false;
			}
		}
	}
	
	return false;
}
