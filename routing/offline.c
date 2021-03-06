#include <stdlib.h>
#include <ipfs/routing/routing.h>
#include <ipfs/util/errs.h>
#include "libp2p/crypto/rsa.h"
#include "libp2p/record/record.h"
#include "ipfs/datastore/ds_helper.h"
#include "ipfs/merkledag/merkledag.h"
#include "ipfs/routing/routing.h"
#include "ipfs/importer/resolver.h"

int ipfs_routing_generic_put_value (ipfs_routing* offlineRouting, char *key, size_t key_size, void *val, size_t vlen)
{
    int err;
    char *record, *nkey;
    size_t len, nkey_len;

    err = libp2p_record_make_put_record (&record, &len, offlineRouting->sk, key, val, vlen, 0);

    if (err) {
        return err;
    }

    nkey = malloc(key_size * 2); // FIXME: size of encoded key
    if (!nkey) {
        free (record);
        return -1;
    }

    if (!ipfs_datastore_helper_ds_key_from_binary((unsigned char*)key, key_size, (unsigned char*)nkey, key_size+1, &nkey_len)) {
        free (nkey);
        free (record);
        return -1;
    }

    // TODO: Save to db as offline storage.
    free (record);
    return 0; // success.
}

int ipfs_routing_generic_get_value (ipfs_routing* routing, char *key, size_t key_size, void **val, size_t *vlen)
{
	char key_str[key_size + 1];
	strncpy(key_str, key, key_size);
	key_str[key_size] = 0;
    struct Node* node = ipfs_resolver_get(key_str, NULL, routing->local_node);
    if (node == NULL)
    	return -1;
    // protobuf the node
    int protobuf_size = ipfs_node_protobuf_encode_size(node);
    *val = malloc(protobuf_size);
    if (ipfs_node_protobuf_encode(node, *val, protobuf_size, vlen) == 0)
    	return -1;

    return 0;
}

int ipfs_routing_offline_find_providers (ipfs_routing* offlineRouting, char *key, size_t key_size, void *ret, size_t *rlen)
{
    return ErrOffline;
}

int ipfs_routing_offline_find_peer (ipfs_routing* offlineRouting, char *peer_id, size_t pid_size, void *ret, size_t *rlen)
{
    return ErrOffline;
}

int ipfs_routing_offline_provide (ipfs_routing* offlineRouting, char *cid)
{
    return ErrOffline;
}

int ipfs_routing_offline_ping (ipfs_routing* offlineRouting, struct Libp2pMessage* message)
{
    return ErrOffline;
}

int ipfs_routing_offline_bootstrap (ipfs_routing* offlineRouting)
{
    return ErrOffline;
}

ipfs_routing* ipfs_routing_new_offline (struct IpfsNode* local_node, struct RsaPrivateKey *private_key)
{
    ipfs_routing *offlineRouting = malloc (sizeof(ipfs_routing));

    if (offlineRouting) {
        offlineRouting->local_node     = local_node;
        offlineRouting->sk            = private_key;
        offlineRouting->stream = NULL;

        offlineRouting->PutValue      = ipfs_routing_generic_put_value;
        offlineRouting->GetValue      = ipfs_routing_generic_get_value;
        offlineRouting->FindProviders = ipfs_routing_offline_find_providers;
        offlineRouting->FindPeer      = ipfs_routing_offline_find_peer;
        offlineRouting->Provide       = ipfs_routing_offline_provide;
        offlineRouting->Ping          = ipfs_routing_offline_ping;
        offlineRouting->Bootstrap     = ipfs_routing_offline_bootstrap;
    }

    return offlineRouting;
}
