include(import_doctest.cmake)

toylib_add_test(NAME "SeqList"
        LEVEL 1
        SOURCES "seqlist/seqlist_l1_test.cc"
        TIMEOUT 30)

toylib_add_test(NAME "SeqList"
        LEVEL 2
        SOURCES "seqlist/seqlist_l2_test.cc"
        TIMEOUT 60)
