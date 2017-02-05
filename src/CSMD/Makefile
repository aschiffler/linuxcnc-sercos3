################################################
# makefile:
# Compile the CoSeMa objects.
# Build the Sercos III master cosema-library
################################################

include $(PWD_DIR)/Rules.mk


########################
#
# obj list of the library
#
########################
#
OBJS = CSMD_ATOM_SVC.o \
    CSMD_BM_CFG.o \
    CSMD_CALC.o \
    CSMD_CFG_PARA.o \
    CSMD_CONFIG.o \
    CSMD_CP_AUX.o \
    CSMD_CYC_PRIV.o \
    CSMD_CYC_SVC.o \
    CSMD_CYCLIC.o \
    CSMD_DIAG.o \
    CSMD_FRCFG_RX.o \
    CSMD_FRCFG_TX.o \
    CSMD_GET_TIME.o \
    CSMD_HAL.o \
    CSMD_HAL_DMA.o \
    CSMD_HAL_NRT.o \
    CSMD_HOTPLUG.o \
    CSMD_INIT.o \
    CSMD_PARSER.o \
    CSMD_PARSER_AUX.o \
    CSMD_PHASE.o \
    CSMD_PHASEDEV.o \
    CSMD_PRIV_SVC.o \
    CSMD_PUB_SVC.o \
    CSMD_RING_CFG.o \
    CSMD_SERC_MON_CFG.o \
    CSMD_TIME.o \
    CSMD_TOPOLOGY.o


########################
#
# target to build the library
#
########################
#
libcosema.a: $(OBJS)
    @$(AR) $(ARFLAGS) $@ $(OBJS)
    @$(RANLIB) $@



########################
#
# Include the dependencies of the sources
#
########################
#
#
include $(subst .o,.d, $(OBJS))


