DEBUG = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations  \
-Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wconversion 						\
-Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers 			\
-Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel 			\
-Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE -fsanitize=address

CPP = g++

start: tree.o tree_dump.o frontend.o common.o
	$(CPP) tree.o tree_dump.o common.o frontend.o -o frontend $(DEBUG)
	./frontend

tree.o : src/tree/tree.cpp
	$(CPP) src/tree/tree.cpp -c

tree_dump.o : src/tree/tree_dump/tree_dump.cpp
	$(CPP) src/tree/tree_dump/tree_dump.cpp -c

frontend.o : src/frontend/frontend.cpp
	$(CPP) src/frontend/frontend.cpp -c

common.o: src/common/common.cpp
	$(CPP) src/common/common.cpp -c

clean:
	rm -f $(TARGET)
	rm -f *.o
	rm -f src/tree/tree_dump/dumps/png/*.*
	rm -f src/tree/tree_dump/dumps/dot/*.*
	rm -f src/tree/tree_dump/dumps/dumps/*.*
	rm -f src/tree/tree_dump/dumps/tex/*.*
	rm -f *.out
	rm -f *.exe
	rm -f src/tree/tree_dump/dumps/pdf/*.aux
	rm -f src/tree/tree_dump/dumps/pdf/*.log
	rm -f src/tree/tree_dump/dumps/pdf/*.pdf
