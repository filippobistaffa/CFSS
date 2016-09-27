.PHONY: all

ifndef OUT
OUT=./cfss
endif

CMP=g++
WARN=-Wall -Wno-unused-result -Wno-deprecated-declarations -Wno-sign-compare -Wno-maybe-uninitialized
OPTIM=-Ofast -march=native -funroll-loops -funsafe-loop-optimizations -falign-functions=16 -falign-loops=16
NOOPTIM=-O0 -march=native
DBG=-g ${NOOPTIM}

COBJSUBDIR=cobj
DEPSUBDIR=dep

ECHOCC=>&2 echo "[\033[01;33m CC \033[0m]"
ECHOLD=>&2 echo "[\033[01;36m LD \033[0m]"

OPT=${OPTIM} # Put desired optimisation level here

define compilec
${ECHOCC} $(notdir $<) ;\
mkdir -p ${DEPSUBDIR} ;\
tmp=`mktemp` ;\
${CMP} ${DEFS} ${INC} -MM ${OPT} $< >> $$tmp ;\
if [ $$? -eq 0 ] ;\
then echo -n "${COBJSUBDIR}/" > ${DEPSUBDIR}/$(notdir $<).d ;\
cat $$tmp >> ${DEPSUBDIR}/$(notdir $<).d ;\
rm $$tmp ;\
mkdir -p ${COBJSUBDIR} ;\
cd ${COBJSUBDIR} ;\
${CMP} ${DEFS} -c ${INC} ${OPT} ${WARN} ../$< ;\
else \
ret=$$? ;\
rm $$tmp ;\
exit $$ret ;\
fi
endef

all: cfss
	@true

-include ${DEPSUBDIR}/*.d

cfss: ${COBJSUBDIR}/cfss.o ${COBJSUBDIR}/random.o
	@${ECHOLD} cfss
	@${CMP} ${OPT} $^ ${LINK} -o ${OUT}

${COBJSUBDIR}/cfss.o: cfss.cpp
	@$(compilec)

${COBJSUBDIR}/random.o: random.c
	@$(compilec)

clean:
	@echo "Removing subdirectories..."
	@rm -rf ${COBJSUBDIR} ${DEPSUBDIR}
