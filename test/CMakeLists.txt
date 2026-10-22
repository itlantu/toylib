include(import_doctest.cmake)

toylib_add_test_dir(NAME "Error"    DIRECTORY "error"       TIMEOUT 30)
toylib_add_test_dir(NAME "SeqList"  DIRECTORY "seqlist"     TIMEOUT 60)
toylib_add_test_dir(NAME "DList"    DIRECTORY "dlist"       TIMEOUT 60)
toylib_add_test_dir(NAME "Stack"    DIRECTORY "stack"       TIMEOUT 60)
