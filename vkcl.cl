void kernel __attribute__((reqd_work_group_size(1, 1, 1))) vkcl(global int* inbuf, global int* outbuf)
{
    int gid = get_global_id(0);
    outbuf[gid] = inbuf[gid];
}
