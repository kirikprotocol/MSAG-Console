include ../../makefile.inc

all: $(OUTDIR)/hello

OBJ=\
  $(OBJDIR)/hello.o


$(OUTDIR)/hello: $(OBJ)
	$(LD) -o $(OUTDIR)/hello $(OBJ)
  
$(OBJDIR)/hello.o: hello.c
	$(CC) -o $(OBJDIR)/hello.o $(CFLAGS) hello.c

