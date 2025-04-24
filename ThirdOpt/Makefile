CC=g++

OUT_O_DIR?=build

DED32_FLAGS=-ggdb3 -std=c++17 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations \
-Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts \
-Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal \
-Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op \
-Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self \
-Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 \
-Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override \
-Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast \
-Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast \
-Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -pie -fPIE -Werror=vla

SANITIZER_FLAGS=-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr 

CC?=clang

TARGET=debug

ifeq ($(TARGET), release)
	CFLAGS=-O0 -g -no-pie -z noexecstack -mavx2
else ifeq ($(TARGET), debug)
	CFLAGS=-ggdb3 -g -z noexecstack -std=c++17 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -no-pie -mavx2\
	-Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts \
	-Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal \
	-Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op \
	-Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self \
	-Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 \
	-Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override \
	-Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast \
	-Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast \
	-Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -pie -fPIE -Werror=vla \
	-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr 
endif


SOURCES=main.cpp FileBufferizer.cpp HashTable.cpp List/List.cpp ErrorParser.cpp HashTableBench.cpp
SOURCES_ASM=ListSearch.asm
OBJECTS:=$(addprefix $(OUT_O_DIR)/,$(SOURCES:.cpp=.o))
OBJECTS_ASM:=$(addprefix $(OUT_O_DIR)/,$(SOURCES_ASM:.asm=.o))
LISTINGS:=$(addprefix $(OUT_O_DIR)/,$(SOURCES_ASM:.cpp=.lst))
DEPS=$(OBJECTS:.o=.d)

EXECUTABLE=Hash


.PHONY: clean
.PHONY: all

all: $(SOURCES) $(SOURCES_ASM) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(OBJECTS_ASM)
	@$(CC) $(LDFLAGS) $(CFLAGS) $(LFLAGS) $(OBJECTS_ASM) $(OBJECTS) -o $@

$(OBJECTS_ASM) : $(OUT_O_DIR)/%.o : %.asm
	@mkdir -p $(@D)
	@nasm -f elf64 $< -g -o $@

$(OBJECTS) : $(OUT_O_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LFLAGS) -c $< -o $@

$(DEPS) : $(OUT_O_DIR)/%.d : %.cpp
	@mkdir -p $(@D)
	@$(CC) -E $(CFLAGS) $< -MM -MT $(@:.d=.o) > $@

include $(DEPS)

.PHONY clean:
	rm -rf *.o l build