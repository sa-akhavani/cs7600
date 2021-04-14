CC=gcc
CFLAGS=-g3 -O0 -fno-stack-protector

# ============================================
# ================Added Parts=================
# run make hw5 then make check to test the code.

hw5: checkpoint restart readckpt hello ckpt libckpt.so

testsave: testsave.c
	${CC} ${CFLAGS} -DSTANDALONE -o $@ $<

checkpoint: checkpoint.c
	${CC} ${CFLAGS} -o $@ $<

readckpt: readckpt.c
	${CC} ${CFLAGS} -o $@ $<

restart: restart.c
	${CC} -static -Wl,-Ttext-segment=5000000 -Wl,-Tdata=5100000 -Wl,-Tbss=5200000 -o $@ $<

hello: hello.c
	${CC} ${CFLAGS} -o $@ $<

ckpt: ckpt.c
	${CC} ${CFLAGS} -fPIC -c -o $@ $<
libckpt.so: ckpt
	${CC} ${CFLAGS} -shared -fPIC -o $@ $<

check: checkpoint restart hello libckpt.so
	rm -f myckpt  # Remove any older 'myckpt' files
	./checkpoint ./hello 10 &
	sleep 2
	echo Checkpointing ...
	kill -12 `pgrep -n hello`
	sleep 2
	echo Restarting ...
	pkill -9 hello  # Kill your 'hello' program
	./restart

# ============================================
# =============End of Added Parts=============



all: primes-test counting-test test1 test2 test3 test4 test5 \
     proc-self-maps save-restore-memory hw5

#========================

# https://stackoverflow.com/questions/36692315/what-exactly-does-rdynamic-do-and-when-exactly-is-it-needed
primes-test: primes-test.c
	${CC} ${CFLAGS} -rdynamic -o $@ $<

counting-test: counting-test.c 
	${CC} ${CFLAGS} -rdynamic -o $@ $<

test: test.c
	${CC} ${CFLAGS} -o $@ $<

test%: test libconstructor%.so
	cp $< $@

# -fPIC required for shared libraries (.so)
libconstructor%.so: constructor%.o
	${CC} ${CFLAGS} -shared -fPIC -o $@ $<
constructor%.o: constructor%.c
	${CC} ${CFLAGS} -fPIC -c $<

#========================
proc-self-maps: proc-self-maps.c
	${CC} ${CFLAGS} -DSTANDALONE -o $@ $<

#========================
save-restore-memory: save-restore-memory.c
	${CC} ${CFLAGS} -o $@ $<

#========================

clean:
	rm -f a.out primes-test counting-test
	rm -f libconstructor?.so constructor?.o test? test
	rm -f proc-self-maps save-restore-memory save-restore.dat
	rm -f checkpoint ckpt hello readckpt restart libckpt.so
	rm -f myckpt myckpt.header
	rm -f while1

dist: clean
	dir=`basename $$PWD` && cd .. && tar czvf $$dir.tar.gz ./$$dir
	dir=`basename $$PWD` && ls -l ../$$dir.tar.gz

.PRECIOUS: libconstructor%.so constructor%.o
