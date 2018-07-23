void kernel vkcl(global int* inbuf, global int* outbuf)
{
    int gid = get_global_id(0);
    outbuf[gid] = inbuf[gid];
}
