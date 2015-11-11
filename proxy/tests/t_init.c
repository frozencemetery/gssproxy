/* Copyright (C) 2014 the GSS-PROXY contributors, see COPYING for license */

#include "t_utils.h"

int main(int argc, const char *argv[])
{
    char buffer[MAX_RPC_SIZE];
    uint32_t buflen;
    gss_cred_id_t cred_handle = GSS_C_NO_CREDENTIAL;
    gss_ctx_id_t context_handle = GSS_C_NO_CONTEXT;
    gss_buffer_desc in_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc out_token = GSS_C_EMPTY_BUFFER;
    gss_name_t name;
    uint32_t ret_maj;
    uint32_t ret_min;
    int ret = -1;

    ret = t_string_to_name(argv[1], &name, GSS_C_NT_HOSTBASED_SERVICE);
    if (ret) {
        DEBUG(argv[0], "Failed to import server name from argv[1]\n");
        ret = -1;
        goto done;
    }

    ret_maj = gss_init_sec_context(&ret_min,
                                   cred_handle,
                                   &context_handle,
                                   name,
                                   GSS_C_NO_OID,
                                   GSS_C_MUTUAL_FLAG | GSS_C_REPLAY_FLAG,
                                   0,
                                   GSS_C_NO_CHANNEL_BINDINGS,
                                   &in_token,
                                   NULL,
                                   &out_token,
                                   NULL,
                                   NULL);
    if (ret_maj != GSS_S_CONTINUE_NEEDED) {
        DEBUG(argv[0], "gss_init_sec_context() failed\n");
        t_log_failure(GSS_C_NO_OID, ret_maj, ret_min);
        ret = -1;
        goto done;
    }

    /* We get stuff from stdin and spit it out on stderr */
    if (!out_token.length) {
        DEBUG(argv[0], "No output token ?");
        ret = -1;
        goto done;
    }

    ret = t_send_buffer(STDOUT_FD, out_token.value, out_token.length);
    if (ret) {
        DEBUG(argv[0], "Failed to send data to server!\n");
        ret = -1;
        goto done;
    }

    ret = t_recv_buffer(STDIN_FD, buffer, &buflen);
    if (ret != 0) {
        DEBUG(argv[0], "Failed to read token from STDIN\n");
        ret = -1;
        goto done;
    }

    in_token.value = buffer;
    in_token.length = buflen;

    ret_maj = gss_init_sec_context(&ret_min,
                                   cred_handle,
                                   &context_handle,
                                   name,
                                   GSS_C_NO_OID,
                                   GSS_C_MUTUAL_FLAG | GSS_C_REPLAY_FLAG,
                                   0,
                                   GSS_C_NO_CHANNEL_BINDINGS,
                                   &in_token,
                                   NULL,
                                   &out_token,
                                   NULL,
                                   NULL);
    if (ret_maj) {
        DEBUG(argv[0], "Error initializing context\n");
        t_log_failure(GSS_C_NO_OID, ret_maj, ret_min);
        ret = -1;
        goto done;
    }

    ret = 0;

done:
    gss_delete_sec_context(&ret_min, &context_handle, NULL);
    gss_release_cred(&ret_min, &cred_handle);
    gss_release_buffer(&ret_min, &out_token);
    gss_release_name(&ret_min, &name);
    return ret;
}
