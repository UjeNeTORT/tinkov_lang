DEBUG = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations  \
-Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wconversion 						\
-Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers 			\
-Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel 			\
-Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE -fsanitize=address

CPP = g++

start: stack.o onegin.o spu.o asm.o tree.o tree_dump.o frontend.o middleend.o backend.o common.o compiler.o
	$(CPP) tree.o tree_dump.o common.o frontend.o  -o frontend  $(DEBUG)
	$(CPP) tree.o tree_dump.o common.o middleend.o -o middleend $(DEBUG)
	$(CPP) tree.o tree_dump.o common.o backend.o   -o backend   $(DEBUG)

	$(CPP) onegin.o asm.o -o asm $(DEBUG)
	$(CPP) my_hash.o stack.o spu.o -o spu $(DEBUG)

	$(CPP) common.o compiler.o -o compiler

run:
	./compiler test_code/test.tnkff

compiler.o: src/compiler/compiler.*
	$(CPP) src/compiler/compiler.cpp -c

tree.o : src/tree/tree.*
	$(CPP) src/tree/tree.cpp -c

tree_dump.o : src/tree/tree_dump/tree_dump.*
	$(CPP) src/tree/tree_dump/tree_dump.cpp -c

frontend.o : src/frontend/frontend.*
	$(CPP) src/frontend/frontend.cpp -c

middleend.o : src/middleend/middleend.*
	$(CPP) src/middleend/middleend.cpp -c

backend.o : src/backend/backend.*
	$(CPP) src/backend/backend.cpp -c

common.o: src/common/common.*
	$(CPP) src/common/common.cpp -c

processor.o: stack.o onegin.o spu.o asm.o disasm.o
	$(CPP) onegin.o asm.o -o asm $(DEBUG)
	$(CPP) my_hash.o stack.o spu.o -o spu $(DEBUG)

stack.o : src/stack/stack.*
	$(CPP) src/stack/stack.cpp -c -o stack.o
	$(CPP) src/stack/my_hash.cpp -c -o my_hash.o

onegin.o : src/processor/text_processing_lib/text_buf.*
	$(CPP) src/processor/text_processing_lib/text_buf.cpp -c -o onegin.o

spu.o : src/processor/processor/spu.*
	$(CPP) src/processor/processor/spu.cpp -c -o spu.o

asm.o : spu.o src/processor/assembler/asm.*
	$(CPP) src/processor/assembler/asm.cpp -c -o asm.o

clean:
	rm -f *.out
	rm -f *.exe
	rm -f *.o
	rm -f spu
	rm -f asm
	rm -f frontend
	rm -f middleend
	rm -f backend
	rm -f compiler

clean_all:
	rm -f *.out
	rm -f *.exe
	rm -f *.o
	rm -f spu
	rm -f asm
	rm -f frontend
	rm -f middleend
	rm -f backend
	rm -f compiler
	rm -f *.ast
	rm -f *.tree
	rm -f *.log
	rm -f *.bin
	rm -f *.tinkov
	rm -f src/tree/tree_dump/dumps/png/*.*
	rm -f src/tree/tree_dump/dumps/dot/*.*
	rm -f src/tree/tree_dump/dumps/dumps/*.*
	rm -f src/tree/tree_dump/dumps/tex/*.*
	rm -f src/tree/tree_dump/dumps/pdf/*.aux
	rm -f src/tree/tree_dump/dumps/pdf/*.log
	rm -f src/tree/tree_dump/dumps/pdf/*.pdf
