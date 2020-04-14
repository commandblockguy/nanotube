#include <string.h>
#include <fileioc.h>
#include <stdint.h>
#include "tftp.h"

#define SEPARATOR '.'

/*

//todo: turn this into its own program at some point

typedef struct type {
    char *extension;
    uint8_t types[3];
} type_t;

const type_t types[] = {
        {"8xp", {TI_PRGM_TYPE, TI_PPRGM_TYPE, TI_TPRGM_TYPE}},
        {"8xv", {TI_APPVAR_TYPE}},
        {"8xn", {TI_REAL_TYPE}},
        {"8x", {TI_CPLX_TYPE}},
        {"8x", {TI_MATRIX_TYPE}},
        {"8x", {TI_STRING_TYPE}},
        {"8x", {TI_EQU_TYPE}},
        {"8x", {TI_REAL_LIST_TYPE}},
        {"8x", {TI_CPLX_LIST_TYPE}},
};*/

void *tftp_open(const char *fname, const char *mode, u8_t write) {
    char tempstr[100];
    char *fileioc_mode = write ? "w" : "r";

    sprintf(tempstr, "Opening file %s with mode %s and %u", fname, mode, write);
    mainlog(tempstr);

    return (void*)ti_Open(fname, fileioc_mode);
}

void tftp_close(void *handle) {
    char tempstr[50];
    sprintf(tempstr, "Closing handle %u", (uint24_t)handle);
    ti_Close((ti_var_t)handle);
}

int tftp_read(void *handle, void *buf, int bytes) {
    char tempstr[50];
    sprintf(tempstr, "Reading %u bytes from handle %u", bytes, (uint24_t)handle);

    return ti_Read(buf, 1, bytes, (ti_var_t)handle);
}

int tftp_write(void *handle, struct pbuf *p) {
    uint24_t bytes = p->tot_len;
    char tempstr[50];
    sprintf(tempstr, "Reading %u bytes from handle %u", bytes, (uint24_t)handle);

    return ti_Write(p->payload, 1, bytes, (ti_var_t)handle);
}

const struct tftp_context tftpContext = {
        tftp_open,
        tftp_close,
        tftp_read,
        tftp_write
};
