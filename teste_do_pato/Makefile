#---------------------------------------------------------------------------------------------------------------------
# Makefile genérico para projetos Butano (https://github.com/GValiente/butano).
#
# Ele localiza o butano automaticamente:
#   1. Usa a variável de ambiente LIBBUTANO / BUTANO, se estiver definida.
#   2. Procura em uma lista de caminhos candidatos relativos ao projeto.
#   3. Como último recurso, sobe na árvore de diretórios procurando um "butano.mak".
#
# Para forçar um caminho:  make LIBBUTANO=/caminho/para/butano/butano
#---------------------------------------------------------------------------------------------------------------------

#---------------------------------------------------------------------------------------------------------------------
# Configuração do projeto (todos os caminhos são relativos à pasta deste Makefile):
#---------------------------------------------------------------------------------------------------------------------
TARGET      	:=  $(notdir $(CURDIR))
BUILD       	:=  build
PYTHON      	:=  $(shell command -v python3 >/dev/null 2>&1 && echo python3 || echo python)
SOURCES     	:=  src
INCLUDES    	:=  include
DATA        	:=
GRAPHICS    	:=  graphics
AUDIO       	:=  audio
AUDIOBACKEND	:=  maxmod
AUDIOTOOL   	:=
DMGAUDIO    	:=  dmg_audio
DMGAUDIOBACKEND	:=  default
ROMTITLE    	:=  MAFIARAT
ROMCODE     	:=  MRAT
USERFLAGS   	:=
USERCXXFLAGS	:=
USERASFLAGS 	:=
USERLDFLAGS 	:=
USERLIBDIRS 	:=
USERLIBS    	:=
DEFAULTLIBS 	:=
STACKTRACE  	:=
USERBUILD   	:=
EXTTOOL     	:=

#---------------------------------------------------------------------------------------------------------------------
# Localização automática do butano.
# Aceita LIBBUTANO ou BUTANO vindo do ambiente/linha de comando.
#---------------------------------------------------------------------------------------------------------------------
ifndef LIBBUTANO
    ifdef BUTANO
        LIBBUTANO := $(BUTANO)
    endif
endif

# Um diretório válido de butano é aquele que contém "butano.mak".
# Lista de candidatos relativos (a pasta interna "butano/butano" contém o butano.mak):
BUTANO_CANDIDATES := \
    ../butano/butano \
    ../../butano/butano \
    ../../../butano/butano \
    ../butano \
    ../../butano \
    $(HOME)/butano/butano \
    $(HOME)/gba/butano/butano \
    /opt/butano/butano

ifndef LIBBUTANO
    LIBBUTANO := $(firstword $(foreach dir,$(BUTANO_CANDIDATES),$(if $(wildcard $(dir)/butano.mak),$(dir))))
endif

# Último recurso: sobe na árvore de diretórios procurando um butano.mak.
ifndef LIBBUTANO
    LIBBUTANO := $(shell dir="$(CURDIR)"; \
        while [ "$$dir" != "/" ]; do \
            found=$$(find "$$dir" -maxdepth 3 -name butano.mak 2>/dev/null | head -n1); \
            if [ -n "$$found" ]; then dirname "$$found"; break; fi; \
            dir=$$(dirname "$$dir"); \
        done)
endif

ifeq ($(strip $(LIBBUTANO)),)
    $(error Nao encontrei o butano. Defina LIBBUTANO manualmente, ex: make LIBBUTANO=/caminho/para/butano/butano)
endif

ifeq ($(wildcard $(LIBBUTANO)/butano.mak),)
    $(error LIBBUTANO=$(LIBBUTANO) nao contem butano.mak. Aponte para a pasta interna "butano/butano")
endif

#---------------------------------------------------------------------------------------------------------------------
# Exporta o caminho absoluto do butano:
#---------------------------------------------------------------------------------------------------------------------
ifndef LIBBUTANOABS
    export LIBBUTANOABS	:=	$(realpath $(LIBBUTANO))
    $(info Usando butano em: $(LIBBUTANOABS))
endif

#---------------------------------------------------------------------------------------------------------------------
# Inclui o makefile principal do butano:
#---------------------------------------------------------------------------------------------------------------------
include $(LIBBUTANOABS)/butano.mak
