C_FLAGS = -O2

CPP = g++

start: stack.o onegin.o spu.o asm.o tree.o tree_dump.o frontend.o middleend.o backend.o common.o compiler.o
	$(CPP) obj/tree.o obj/tree_dump.o obj/common.o obj/frontend.o  -o frontend
	$(CPP) obj/tree.o obj/tree_dump.o obj/common.o obj/middleend.o -o middleend
	$(CPP) obj/tree.o obj/tree_dump.o obj/common.o obj/backend.o   -o backend

	$(CPP) obj/onegin.o obj/asm.o -o asm
	$(CPP) obj/my_hash.o obj/stack.o obj/spu.o -o spu

	$(CPP) obj/common.o obj/compiler.o -o compiler

compile:
	./compiler test_code/square.tnkff

run:
	./exec

compiler.o: src/compiler/compiler.*
	$(CPP) $(C_FLAGS) src/compiler/compiler.cpp -c -o obj/compiler.o

tree.o: src/tree/tree.*
	$(CPP) $(C_FLAGS) src/tree/tree.cpp -c -o obj/tree.o

tree_dump.o: src/tree/tree_dump/tree_dump.*
	$(CPP) $(C_FLAGS) src/tree/tree_dump/tree_dump.cpp -c -o obj/tree_dump.o

frontend.o: src/frontend/frontend.*
	$(CPP) $(C_FLAGS) src/frontend/frontend.cpp -c -o obj/frontend.o

middleend.o: src/middleend/middleend.*
	$(CPP) $(C_FLAGS) src/middleend/middleend.cpp -c -o obj/middleend.o

backend.o: src/backend/backend.*
	$(CPP) $(C_FLAGS) src/backend/backend.cpp -c -o obj/backend.o

common.o: src/common/common.*
	$(CPP) $(C_FLAGS) src/common/common.cpp -c -o obj/common.o

processor.o: stack.o onegin.o spu.o asm.o disasm.o
	$(CPP) $(C_FLAGS) obj/onegin.o obj/asm.o -o asm $(C_FLAGS)
	$(CPP) $(C_FLAGS) obj/my_hash.o obj/stack.o obj/spu.o -o spu $(C_FLAGS)

stack.o : src/stack/stack.*
	$(CPP) src/stack/stack.cpp -c -o obj/stack.o
	$(CPP) src/stack/my_hash.cpp -c -o obj/my_hash.o

onegin.o : src/processor/text_processing_lib/text_buf.*
	$(CPP) $(C_FLAGS) src/processor/text_processing_lib/text_buf.cpp -c -o obj/onegin.o

spu.o : src/processor/processor/spu.*
	$(CPP) $(C_FLAGS) src/processor/processor/spu.cpp -c -o obj/spu.o

asm.o : spu.o src/processor/assembler/asm.*
	$(CPP) $(C_FLAGS) src/processor/assembler/asm.cpp -c -o obj/asm.o

clean:
	rm -f *.out
	rm -f *.exe
	rm -f *.o
	rm -f obj/*
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
	rm -f obj/*
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
	rm -f out.s
	rm -f *.tinkov
	rm -f src/tree/tree_dump/dumps/png/*.*
	rm -f src/tree/tree_dump/dumps/dot/*.*
	rm -f src/tree/tree_dump/dumps/dumps/*.*
	rm -f src/tree/tree_dump/dumps/tex/*.*
	rm -f src/tree/tree_dump/dumps/pdf/*.aux
	rm -f src/tree/tree_dump/dumps/pdf/*.log
	rm -f src/tree/tree_dump/dumps/pdf/*.pdf
