libdbcompat_sources = [
    'btree/bt_close.c',
    'btree/bt_conv.c',
    'btree/bt_debug.c',
    'btree/bt_delete.c',
    'btree/bt_get.c',
    'btree/bt_open.c',
    'btree/bt_overflow.c',
    'btree/bt_page.c',
    'btree/bt_put.c',
    'btree/bt_search.c',
    'btree/bt_seq.c',
    'btree/bt_split.c',
    'btree/bt_utils.c',
    'recno/rec_close.c',
    'recno/rec_delete.c',
    'recno/rec_get.c',
    'recno/rec_open.c',
    'recno/rec_put.c',
    'recno/rec_search.c',
    'recno/rec_seq.c',
    'recno/rec_utils.c',
    'db.c',
    'mpool.c',

]

libdbcompat = static_library(
    'dbcompat',
    libdbcompat_sources,
    include_directories: [inc, cinc, dbinc],
    c_args: ['-D__DBINTERFACE_PRIVATE'],
)
