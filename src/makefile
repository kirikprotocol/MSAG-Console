MODULES=test

all deploy:
	-for name in ${MODULES} ; do (echo Processing: $$name && cd modules/$$name && make $@;) done

