# Сборка компонентов-калькуляторов курса (Eco.Education, 001.InsideACOM, уроки 02-07)
# из исходников без изменений в статические библиотеки (StaticRelease, Linux x86_64).
#
# Запуск: make -f Makefile COMP=A        (COMP = A, B, C, D, E)
#
# Исходники написаны под предыдущую версию Eco.Framework, совместимость обеспечивается флагами:
#   -std=gnu89                            в исходниках есть комментарии вида //
#   -DERR_ECO_SUCCESES=ERR_ECO_SUCCESS    в предыдущей версии фреймворка код успеха назывался ERR_ECO_SUCCESES
# Компонент C - вторая версия компонента A: тот же CID и те же глобальные символы фабрики.
# Чтобы A и C можно было скомпоновать в один исполняемый файл (юнит-тест подменяет A на C),
# эти три символа у C получают суффикс _C.

COMP ?= A
PLATFORM = Linux
ARCH_TARGET = x86_64

ifeq ($(COMP), A)
    CID = 4828F6552E4540E78121EBD220DC360E
else ifeq ($(COMP), B)
    CID = AE202E543CE54550899603BD70C62565
else ifeq ($(COMP), C)
    CID = 4828F6552E4540E78121EBD220DC360E
    RENAME = -DGetIEcoComponentFactoryPtr_$(CID)=GetIEcoComponentFactoryPtr_$(CID)_C \
             -Dg_x$(CID)Factory=g_x$(CID)Factory_C \
             -Dg_x$(CID)FactoryVTbl=g_x$(CID)FactoryVTbl_C
else ifeq ($(COMP), D)
    CID = 3A8E44677E82475CB4A3719ED8397E61
else ifeq ($(COMP), E)
    CID = 872FEF1DE3314B87AD44D1E7C232C2F0
else
    $(error Unknown COMP: $(COMP). Use A, B, C, D or E)
endif

ROOT = ../../..
SRC_DIR = $(ROOT)/Eco.Calculator$(COMP)
OUT_DIR = $(SRC_DIR)/BuildFiles/$(PLATFORM)/$(ARCH_TARGET)/StaticRelease
OUT = $(OUT_DIR)/lib$(CID).a

# Заголовки фреймворка, самого калькулятора, а также A и B (D включает B или A, E агрегирует B)
INC = -I "$(ECO_FRAMEWORK)/Eco.Core1/SharedFiles" \
      -I "$(ECO_FRAMEWORK)/Eco.InterfaceBus1/SharedFiles" \
      -I "$(ECO_FRAMEWORK)/Eco.MemoryManager1/SharedFiles" \
      -I "$(ECO_FRAMEWORK)/Eco.System1/SharedFiles" \
      -I $(SRC_DIR)/HeaderFiles \
      -I $(SRC_DIR)/SharedFiles \
      -I $(ROOT)/Eco.CalculatorA/SharedFiles \
      -I $(ROOT)/Eco.CalculatorB/SharedFiles

CC = gcc
AR = gcc-ar
CCFLAGS = -std=gnu89 -Wall -O2 -fno-stack-protector -DECO_LIB -DECO_X86_64 -DUGUID_UTILITY -DECO_LINUX \
          -DERR_ECO_SUCCESES=ERR_ECO_SUCCESS $(RENAME) $(INC)

SRC = $(SRC_DIR)/SourceFiles/CEcoCalculator$(COMP).c \
      $(SRC_DIR)/SourceFiles/CEcoCalculator$(COMP)Factory.c

# Объектные файлы создаются в каталоге результата, а не рядом с исходниками курса
OBJ = $(OUT_DIR)/CEcoCalculator$(COMP).o \
      $(OUT_DIR)/CEcoCalculator$(COMP)Factory.o

.PHONY: all clean

all: $(OUT)

$(OUT_DIR)/%.o: $(SRC_DIR)/SourceFiles/%.c
	@mkdir -p $(OUT_DIR)
	$(CC) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(AR) rcs $@ $(OBJ)
	@rm -f $(OBJ)

clean:
	rm -f $(OBJ) $(OUT)
