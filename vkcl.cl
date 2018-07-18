void kernel vkcl(global int* buf)
{
    int gid = get_global_id(0);
    int ofs = get_global_size(0);
    buf[gid+ofs] = buf[gid];
}
