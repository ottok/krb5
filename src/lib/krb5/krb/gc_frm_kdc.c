/*
 * $Source$
 * $Author$
 *
 * Copyright 1990,1991 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * For copying and distribution information, please see the file
 * <krb5/copyright.h>.
 *
 * Get credentials from some KDC somewhere, possibly accumulating tgts
 * along the way.
 */

#if !defined(lint) && !defined(SABER)
static char rcsid_gcfkdc_c[] =
"$Id$";
#endif	/* !lint & !SABER */


#include <krb5/krb5.h>
#include <krb5/ext-proto.h>
#include "int-proto.h"

/*
 * Retrieve credentials for principal creds->client,
 * server creds->server, ticket flags creds->ticket_flags, possibly
 * second_ticket if needed by ticket_flags.
 * 
 * Credentials are requested from the KDC for the server's realm.  Any
 * TGT credentials obtained in the process of contacting the KDC are
 * returned in an array of credentials; tgts is filled in to point to an
 * array of pointers to credential structures (if no TGT's were used, the
 * pointer is zeroed).  TGT's may be returned even if no useful end ticket
 * was obtained.
 * 
 * The returned credentials are NOT cached.
 *
 * This routine should not be called if the credentials are already in
 * the cache.
 * 
 * If credentials are obtained, creds is filled in with the results;
 * creds->ticket and creds->keyblock->key are set to allocated storage,
 * which should be freed by the caller when finished.
 * 
 * returns errors, system errors.
 */

extern krb5_cksumtype krb5_kdc_req_sumtype;

/* helper function: convert flags to necessary KDC options */
#define flags2options(flags) (flags & KDC_TKT_COMMON_MASK)

krb5_error_code
krb5_get_cred_from_kdc (ccache, cred, tgts)
    krb5_ccache ccache;
    krb5_creds *cred;
    krb5_creds ***tgts;
{
    krb5_creds tgt, tgtq;
    krb5_creds **ret_tgts = 0;
    krb5_principal *tgs_list, *next_server;
    krb5_principal final_server;
    krb5_error_code retval;
    int nservers;
    krb5_enctype etype;

    /* in case we never get a TGT, zero the return */
    *tgts = 0;

    /*
     * we know that the desired credentials aren't in the cache yet.
     *
     * To get them, we first need a tgt for the realm of the server.
     * first, we see if we have such a TGT in cache.
     */
    
    /*
     * look for ticket with:
     * client == cred->client,
     * server == "krbtgt/realmof(cred->server)@realmof(cred->client)"
     */

    /*
     * XXX we're sharing some substructure here, which is
     * probably not safe...
     */
    memset((char *)&tgtq, 0, sizeof(tgtq));
    tgtq.client = cred->client;

    if (retval = krb5_tgtname(krb5_princ_realm(cred->server),
			      krb5_princ_realm(cred->client), &final_server))
	return retval;
    tgtq.server = final_server;

    /* try to fetch it directly */
    retval = krb5_cc_retrieve_cred (ccache,
				    0,	/* default is client & server */
				    &tgtq,
				    &tgt);

    if (retval != 0) {
	if (retval != KRB5_CC_NOTFOUND)
	    goto out;
	/* don't have the right TGT in the cred cache.  Time to iterate
	   across realms to get the right TGT. */

	/* get a list of realms to consult */
	retval = krb5_walk_realm_tree(krb5_princ_realm(cred->client),
				      krb5_princ_realm(cred->server),
				      &tgs_list);
	if (retval)
	    goto out;
	/* walk the list BACKWARDS until we find a cached
	   TGT, then move forward obtaining TGTs until we get the last
	   TGT needed */
	for (next_server = tgs_list; *next_server; next_server++);
	nservers = next_server - tgs_list;
	next_server--;

	/* next_server now points to the last TGT */
	for (; next_server >= tgs_list; next_server--) {
	    tgtq.server = *next_server;
	    retval = krb5_cc_retrieve_cred (ccache,
					    KRB5_TC_MATCH_SRV_NAMEONLY,
					    &tgtq,
					    &tgt);
	    if (retval) {
		if (retval != KRB5_CC_NOTFOUND) {
		    krb5_free_realm_tree(tgs_list);
		    goto out;
		}
		continue;
	    }
	    next_server++;
	    break;			/* found one! */
	}
	if (next_server < tgs_list) {
	    /* didn't find any */
	    retval = KRB5_NO_TKT_IN_RLM;
	    krb5_free_realm_tree(tgs_list);
	    goto out;
	}
	/* allocate storage for TGT pointers. */
	ret_tgts = (krb5_creds **)calloc(nservers+1, sizeof(krb5_creds));
	if (!ret_tgts) {
	    retval = ENOMEM;
	    krb5_free_realm_tree(tgs_list);
	    goto out;
	}
	*tgts = ret_tgts;
	for (nservers = 0; *next_server; next_server++, nservers++) {

	    krb5_data *tmpdata;

	    if (!valid_keytype(tgt.keyblock.keytype)) {
		retval = KRB5_PROG_KEYTYPE_NOSUPP;
		krb5_free_realm_tree(tgs_list);
		goto out;
	    }
	    /* now get the TGTs */
	    memset((char *)&tgtq, 0, sizeof(tgtq));
	    tgtq.times = tgt.times;
	    tgtq.client = tgt.client;

	    /* ask each realm for a tgt to the end */
	    if (retval = krb5_copy_data((*next_server)[0], &tmpdata)) {
		krb5_free_realm_tree(tgs_list);
		goto out;
	    }
	    krb5_free_data(final_server[0]);
	    final_server[0] = tmpdata;
	    tgtq.server = final_server;

	    tgtq.is_skey = FALSE;
	    tgtq.ticket_flags = tgt.ticket_flags;

	    etype = krb5_keytype_array[tgt.keyblock.keytype]->system->proto_enctype;
	    if (retval = krb5_get_cred_via_tgt(&tgt,
					       flags2options(tgtq.ticket_flags),
					       etype,
					       krb5_kdc_req_sumtype,
					       &tgtq)) {
		krb5_free_realm_tree(tgs_list);
		goto out;
	    }
	    /* make sure the returned ticket is somewhere in the remaining
	       list, but we can tolerate different expected issuing realms */
	    while (*++next_server &&
		   !krb5_principal_compare(&(next_server[0])[1],
					   &(tgtq.server[1])));
	    if (!next_server) {
		/* what we got back wasn't in the list! */
		krb5_free_realm_tree(tgs_list);
		retval = KRB5_KDCREP_MODIFIED; /* XXX? */
		goto out;
	    }
							   
	    /* save tgt in return array */
	    if (retval = krb5_copy_creds(&tgtq, &ret_tgts[nservers])) {
		krb5_free_realm_tree(tgs_list);
		goto out;
	    }
	    tgt = *ret_tgts[nservers];
	    tgtq.client = 0;
	    tgtq.server = 0;
	    krb5_free_cred_contents(&tgtq);
	}
	krb5_free_realm_tree(tgs_list);
    }
    /* got/finally have tgt! */
    if (!valid_keytype(tgt.keyblock.keytype)) {
	retval = KRB5_PROG_KEYTYPE_NOSUPP;
	goto out;
    }
    etype = krb5_keytype_array[tgt.keyblock.keytype]->system->proto_enctype;

    retval = krb5_get_cred_via_tgt(&tgt,
				   flags2options(tgt.ticket_flags),
				   etype,
				   krb5_kdc_req_sumtype,
				   cred);
    krb5_free_cred_contents(&tgt);
out:
    krb5_free_principal(final_server);
    return retval;
}
