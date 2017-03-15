/* Copyright (C) 2011 the GSS-PROXY contributors, see COPYING for license */

#include "gssapi_gpm.h"

__thread gssx_status *tls_last_status = NULL;

/* Thread local storage for return status.
 * FIXME: it's not the most portable construct, so may need fixing in future */
void gpm_save_status(gssx_status *status)
{
    int ret;

    if (tls_last_status) {
        xdr_free((xdrproc_t)xdr_gssx_status, (char *)tls_last_status);
        free(tls_last_status);
    }

    ret = gp_copy_gssx_status_alloc(status, &tls_last_status);
    if (ret) {
        /* make sure tls_last_status is zeored on error */
        tls_last_status = NULL;
    }
}

/* This funciton is used to record internal mech errors that are
 * generated by the proxy client code */
void gpm_save_internal_status(uint32_t err, char *err_str)
{
    gssx_status status;

    memset(&status, 0, sizeof(gssx_status));

#define STD_MAJ_ERROR_STR "Internal gssproxy error"
    status.major_status = GSS_S_FAILURE;
    status.major_status_string.utf8string_val = strdup(STD_MAJ_ERROR_STR);
    status.major_status_string.utf8string_len = sizeof(STD_MAJ_ERROR_STR);
    status.minor_status = err;
    status.minor_status_string.utf8string_val = err_str;
    status.minor_status_string.utf8string_len = strlen(err_str) + 1;
    gpm_save_status(&status);
}

OM_uint32 gpm_display_status(OM_uint32 *minor_status,
                             OM_uint32 status_value,
                             int status_type,
                             const gss_OID mech_type UNUSED,
                             OM_uint32 *message_context,
                             gss_buffer_t status_string)
{
    utf8string tmp;
    int ret;

    switch(status_type) {
    case GSS_C_GSS_CODE:
        if (tls_last_status &&
            tls_last_status->major_status == status_value &&
            tls_last_status->major_status_string.utf8string_len) {
                ret = gp_copy_utf8string(&tls_last_status->major_status_string,
                                         &tmp);
                if (ret) {
                    *minor_status = ret;
                    return GSS_S_FAILURE;
                }
                status_string->value = tmp.utf8string_val;
                status_string->length = tmp.utf8string_len;
                *minor_status = 0;
                return GSS_S_COMPLETE;
        } else {
            /* if we do not have it, make it clear */
            return GSS_S_UNAVAILABLE;
        }
    case GSS_C_MECH_CODE:
        if (tls_last_status &&
            tls_last_status->minor_status == status_value &&
            tls_last_status->minor_status_string.utf8string_len) {

            if (*message_context) {
                /* we do not support multiple messages for now */
                *minor_status = EINVAL;
                return GSS_S_FAILURE;
            }

            ret = gp_copy_utf8string(&tls_last_status->minor_status_string,
                                     &tmp);
            if (ret) {
                *minor_status = ret;
                return GSS_S_FAILURE;
            }
            status_string->value = tmp.utf8string_val;
            status_string->length = tmp.utf8string_len;
        } else {
            /* if we do not have it, make it clear */
            return GSS_S_UNAVAILABLE;
        }
        *minor_status = 0;
        return GSS_S_COMPLETE;
    default:
        *minor_status = EINVAL;
        return GSS_S_BAD_STATUS;
    }
}
