DEBUG = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations  \
-Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wconversion 						\
-Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers 			\
-Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel 			\
-Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE -fsanitize=address

CPP = g++

start: tree.o tree_dump.o frontend.o backend.o common.o
	$(CPP) tree.o tree_dump.o common.o frontend.o -o frontend $(DEBUG)
	$(CPP) tree.o tree_dump.o common.o backend.o  -o backend  $(DEBUG)

run:
	./frontend test_code/assign_back_test.tnkff
	./backend ast.ast

frontend_run:
	./frontend test_code/factorial.tnkff

backend_run:
	./backend ast.ast

tree.o : src/tree/tree.*
	$(CPP) src/tree/tree.cpp -c

tree_dump.o : src/tree/tree_dump/tree_dump.*
	$(CPP) src/tree/tree_dump/tree_dump.cpp -c

frontend.o : src/frontend/frontend.*
	$(CPP) src/frontend/frontend.cpp -c

backend.o : src/backend/backend.*
	$(CPP) src/backend/backend.cpp -c

common.o: src/common/common.cpp
	$(CPP) src/common/common.cpp -c

clean:
	rm -f *.out
	rm -f *.exe
	rm -f *.o

clean_all:
	rm -f *.o
	rm -f *.ast
	rm -f *.tree
	rm -f *.log
	rm -f src/tree/tree_dump/dumps/png/*.*
	rm -f src/tree/tree_dump/dumps/dot/*.*
	rm -f src/tree/tree_dump/dumps/dumps/*.*
	rm -f src/tree/tree_dump/dumps/tex/*.*
	rm -f *.out
	rm -f *.exe
	rm -f src/tree/tree_dump/dumps/pdf/*.aux
	rm -f src/tree/tree_dump/dumps/pdf/*.log
	rm -f src/tree/tree_dump/dumps/pdf/*.pdf
