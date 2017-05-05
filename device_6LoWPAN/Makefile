DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

all: main_core

CRYPTO_SHA_DIR = ./sha
CRYPTO_SHA_FILES = sha256.c
PROJECTDIRS += $(CRYPTO_SHA_DIR)
PROJECT_SOURCEFILES += $(CRYPTO_SHA_FILES)

SNMP_RESOURCES_DIR = ./snmpd
SNMP_SOURCEFILES += mibii.c snmp.c snmp_asn1.c

CONTIKI_WITH_IPV6 = 1
REST_RESOURCES_DIR = ./resources ./CoAP
REST_RESOURCES_FILES += res-hello.c res-devices.c

PROJECTDIRS += $(SNMP_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(SNMP_SOURCEFILES)

PROJECTDIRS += $(REST_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)
PROJECT_SOURCEFILES += coap-server.c

# REST Engine shall use Erbium CoAP implementation
# APPS += er-coap servreg-hack
APPS += er-coap
APPS += rest-engine

# Adicionada estas duas linhas de flags para reduzir tamanho do firmware que não cabe no espaço de rom do msp430 que é utilizado na simulação
CFLAGS += -ffunction-sections
LDFLAGS += -Wl,--gc-sections,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__

CONTIKI=/projects/contiki
include $(CONTIKI)/Makefile.include
