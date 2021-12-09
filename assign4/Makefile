.PHONY: all clean
all: compile run

compile: dberror.c storage_mgr.c buffer_mgr_stat.c buffer_mgr.c rm_serializer.c expr.c get_attr.c set_attr.c create_table.c record_mgr.c helpers.c insert.c delete.c btree_mgr.c test_assign4_1.c
	gcc -o test_assign4 test_assign4_1.c dberror.c storage_mgr.c buffer_mgr_stat.c buffer_mgr.c expr.c get_attr.c set_attr.c create_table.c rm_serializer.c record_mgr.c helpers.c insert.c delete.c btree_mgr.c

run:
	./test_assign4

clean:
	rm test_assign4
