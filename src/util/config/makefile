SRC = $(HOME)/cvs/smsc/src/util/config
OUT = $(HOME)/build/out/util/config

GLOBAL_INC = /usr/local/include
XERCES_INC = $(GLOBAL_INC)/xercesc
LOG4CPP_INC= $(GLOBAL_INC)/log4cpp
SMSC_SRC = $(HOME)/cvs/smsc/src

INCS = -I$(GLOBAL_INC) -I$(XERCES_INC) -I$(LOG4CPP_INC) -I$(SMSC_SRC)
LIBS = -llog4cpp -lxercesc

CFLAGS  = -g -march=pentiumpro -pedantic $(INCS)

OBJECTS =\
	DB.o\
	MAP.o\
	Logger.o\
	Manager.o

build: $(foreach X,$(OBJECTS),$(OUT)/$(X))

$(OUT)/%.o: $(SRC)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@
