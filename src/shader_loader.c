#include "private_api.h"
#include <stdio.h>

static
const char* gx_str_readln(
    const char *str,
    char* buf,
    unsigned int length)
{
    int c;
    char* ptr;

    if (!str) {
        return 0;
    }

    ptr = buf;

    while ((c = *str)) {
        if ((unsigned int)(ptr - buf) < (length - 1)) {
            *ptr = c;
            ptr++;
        }

        if (c == '\n') {
            break;
        }

        str ++;
    }

    *ptr = '\0';

    if (ptr == buf) {
        return NULL;
    } else {
        return str + 1;
    }
}

static
char* gx_file_readln(
    FILE* file,
    char* buf,
    unsigned int length)
{
    int c;
    char* ptr;

    if (!file) {
        return 0;
    }

    c = EOF;
    ptr = buf;

    while (!feof(file)) {
        c = fgetc(file);
        
        if (c == EOF) {
            break;
        }

        if ((unsigned int)(ptr - buf) < (length - 1)) {
            *ptr = c;
            ptr++;
        }

        if (c == '\n') {
            break;
        }
    }

    *ptr = '\0';

    if (ptr == buf) {
        return NULL;
    } else {
        return buf;
    }
}

static
int gx_shader_parse(
    const char *filename,
    const char *str,
    ecs_strbuf_t *out)
{
    FILE *f = NULL;
    if (filename) {
        f = fopen(filename, "r");
        if (!f) {
            ecs_err("%s: file not found\n", filename);
            return -1;
        }
    }

    char line[1024];
    do {
        if (f) {
            if (gx_file_readln(f, line, 1024) == NULL) {
                break;
            }
        } else {
            str = gx_str_readln(str, line, 1024);
            if (!str) {
                break;
            }
        }

        if (!ecs_os_strncmp(line, "#include \"", 10)) {
            char *last = strrchr(line, '"');
            char *file = &line[10];
            last[0] = '\0';

            if (gx_shader_parse(file, NULL, out)) {
                return -1;
            }
        } else {
            ecs_strbuf_appendstr(out, line);
        }
    } while(true);

    return 0;
}

char* gx_shader_from_file(
    const char *filename)
{
    ecs_strbuf_t out = ECS_STRBUF_INIT;
    if (gx_shader_parse(filename, NULL, &out)) {
        ecs_strbuf_reset(&out);
        return NULL;
    } else {
        return ecs_strbuf_get(&out);
    }
}

char* gx_shader_from_str(
    const char *str)
{
    ecs_strbuf_t out = ECS_STRBUF_INIT;
    if (gx_shader_parse(NULL, str, &out)) {
        ecs_strbuf_reset(&out);
        return NULL;
    } else {
        return ecs_strbuf_get(&out);
    }
}
