/* Copyright (C) 2011,2012 the GSS-PROXY contributors, see COPYING for license */

#include "gssapi_gpm.h"
#include "src/gp_conv.h"

OM_uint32 gpm_unwrap(OM_uint32 *minor_status,
                     gssx_ctx *context_handle,
                     const gss_buffer_t input_message_buffer,
                     gss_buffer_t output_message_buffer,
                     int *conf_state,
                     gss_qop_t *qop_state)
{
    union gp_rpc_arg uarg;
    union gp_rpc_res ures;
    gssx_arg_unwrap *arg = &uarg.unwrap;
    gssx_res_unwrap *res = &ures.unwrap;
    uint32_t ret_min = 0;
    uint32_t ret_maj = 0;
    int ret = 0;
    gssx_buffer message_buffer;

    memset(&uarg, 0, sizeof(union gp_rpc_arg));
    memset(&ures, 0, sizeof(union gp_rpc_res));

    if (!context_handle) {
        return GSS_S_CALL_INACCESSIBLE_READ;
    }

    /* format request */
    /* NOTE: the final free will also release the old context */
    arg->context_handle = *context_handle;
    if (qop_state) {
        arg->qop_state = *qop_state;
    }

    ret = gp_conv_buffer_to_gssx(input_message_buffer, &message_buffer);
    if (ret) {
        ret_maj = GSS_S_FAILURE;
        ret_min = ret;
        goto done;
    }
    arg->token_buffer.token_buffer_val = calloc(1, sizeof(gssx_buffer));
    if (!arg->token_buffer.token_buffer_val) {
        ret_maj = GSS_S_FAILURE;
        ret_min = ENOMEM;
        goto done;
    }

    arg->token_buffer.token_buffer_val[0] = message_buffer;
    arg->token_buffer.token_buffer_len = 1;

    /* execute proxy request */
    ret = gpm_make_call(GSSX_UNWRAP, &uarg, &ures);
    if (ret) {
        ret_maj = GSS_S_FAILURE;
        ret_min = ret;
        goto done;
    }

    /* format reply */
    if (res->status.major_status) {
        gpm_save_status(&res->status);
        ret_min = res->status.minor_status;
        ret_maj = res->status.major_status;
        goto done;
    }

    if (conf_state) {
        *conf_state = *res->conf_state;
    }
    if (qop_state) {
        *qop_state = *res->qop_state;
    }

    if (res->message_buffer.message_buffer_len > 0) {
        ret = gp_copy_gssx_to_buffer(&res->message_buffer.message_buffer_val[0],
                                     output_message_buffer);
        if (ret) {
            ret_maj = GSS_S_FAILURE;
            ret_min = ret;
            goto done;
        }
    }

done:
    /* Steal the new context if available.
     * NOTE: We do not want it to be freed by xdr_free, so copy the contents
     * and cear up the structure to be freed so contents are not freed. */
    if (res->context_handle) {
        *context_handle = *res->context_handle;
        memset(res->context_handle, 0, sizeof(gssx_ctx));
    } else {
        /* prevent the contexthandle from being destroyed in case of server
         * error. */
        memset(&arg->context_handle, 0, sizeof(gssx_ctx));
    }

    gpm_free_xdrs(GSSX_UNWRAP, &uarg, &ures);
    *minor_status = ret_min;
    return ret_maj;
}

