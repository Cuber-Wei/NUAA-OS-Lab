#include "utils/std.h"
#include "utils/typer.h"
#include "disk.h"
#include "fs.h"
#include "inode.h" 
#include "dir.h"
#include "dump.h"

void dump_regular(inode_t *inode)
{
    char buf[101] = {0};
    int len = inode_read(inode, 0, buf, 100);
    buf[len > 0 ? len : 0] = '\0';
    typer_dump("reg %s\n", buf);
}

extern void dump_dir_entry(dir_entry_t *dir_entry);

void dump_dir(inode_t *dir)
{
    typer_dump("dir\n");
    typer_push();

    dir_entry_t *dir_entry;
    dir_cursor_t dir_cursor;
    dir_cursor_init(&dir_cursor, dir);

    while ((dir_entry = dir_cursor_next(&dir_cursor)) != NULL) {
        dump_dir_entry(dir_entry);
    }

    dir_cursor_destroy(&dir_cursor);
    typer_pop();
}

static int is_dot(char *name)
{
    return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

void dump_dir_entry(dir_entry_t *dir_entry)
{
    char *name = dir_entry->name;
    int ino = dir_entry->ino;
    if (ino == FAKE_INO)
        return;

    typer_dump("entry\n");
    typer_push();

    typer_dump("name %s\n", name);
    typer_dump("ino %d\n", ino);

    inode_t *inode = fs_seek_inode(ino);
    if (inode_is_dir(inode)) {
        // 只对非"."和".."递归
        if (!is_dot(name)) {
            dump_dir(inode);
        }
    } else if (inode_is_regular(inode)) {
        dump_regular(inode);
    }

    typer_pop();
}

void dump_uxfs(int argc, char *argv[])
{
    if (argc != 2) {
        puts("Usage: dump.uxfs path");
        return;
    }
    char *path = argv[1];

    fs = disk_open(path);
    fs_dump();

    inode_t *root = fs_seek_inode(ROOT_INO);
    dump_dir(root);

    disk_close();
}
