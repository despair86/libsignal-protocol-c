# Loki Messenger

Loki Messenger allows for truly decentralised, end to end, and private encrypted chats. Loki Messenger is built to handle both online and fully Asynchronous offline messages. Loki messenger implements the Signal protocol for message encryption. Our Client interface is a fork of [Signal Messenger](https://signal.org/). All communication that passes through Loki messenger is routed through [Lokinet](https://github.com/loki-project/loki-network).

## Summary

Loki messenger integrates directly with Loki [Service Nodes](https://lokidocs.com/ServiceNodes/SNOverview/), which are a set of distributed, decentralized and Sybil resistant nodes. Service Nodes act as both federated servers which store messages offline, and a set of nodes which allow for onion routing functionality obfuscating users IP Addresses. For a full understanding of how Loki messenger works, read the [Loki whitepaper](https://loki.network/whitepaper).

**Online Messages**

If Alice and Bob are both online they can simply resolve each others public keys, to introduction sets, this functionality is handled by interfacing with [Lokinet](https://github.com/loki-project/loki-network). With the appropriate introduction sets Alice and Bob can create a path and using onion routing pass messages through the Loki network without giving away personally identifiable information like their IP address.

**Offline messages**

Offline messaging uses Swarms, given any users public key the user can resolve a public key to a specific grouping of Service Nodes (AKA Swarm) each user in Loki Messenger belongs to a Swarm. When routing a message offline the user selects a Service node in the destination users Swarm, when the user comes online they query any node in their Swarm, if the Swarm is holding any messages for the user they disseminate those messages to the user.

![Swarm Messaging](https://i.imgur.com/o13Knds.png)

Spam protections for Loki Messenger are based on a Proof of Work which is attached to any message that exceeds a default size or Time To Live, this process is discussed further in the [Loki whitepaper](https://loki.network/whitepaper).

## Want to Contribute? Found a Bug or Have a feature request?

Please search for any [existing issues](https://github.com/despair86/loki-pager/issues) that describe your bugs in order to avoid duplicate submissions. Submissions can be made by making a pull request to our development branch, if you don't know where to start contributing , try reading the Github issues page for ideas.

## Build instruction

Build instructions can be found in [CONTRIBUTING.md](CONTRIBUTING.md).

(from old README)

# Overview

This is a ratcheting forward secrecy protocol that works in synchronous and asynchronous messaging 
environments. See the [Java library](https://github.com/whispersystems/libsignal-protocol-java) for more details.

# Using libsignal-protocol-c

## Library initialization

Before using the library, a libsignal-protocol-c client needs to initialize a global
context. This global context is used to provide callbacks for implementations
of functions used across the library that need client-specific implementations.
Refer to "signal_protocol.h" for detailed documentation on these functions, and the unit
tests for example implementations.

```c
signal_context *global_context;
signal_context_create(&global_context, user_data);
signal_context_set_crypto_provider(global_context, &provider);
signal_context_set_locking_functions(global_context, lock_function, unlock_function);
```

## Client install time

At install time, a libsignal-protocol-c client needs to generate its identity keys,
registration id, and prekeys.

```c
ratchet_identity_key_pair *identity_key_pair;
uint32_t registration_id;
signal_protocol_key_helper_pre_key_list_node *pre_keys_head;
session_signed_pre_key *signed_pre_key;

signal_protocol_key_helper_generate_identity_key_pair(&identity_key_pair, global_context);
signal_protocol_key_helper_generate_registration_id(&registration_id, 0, global_context);
signal_protocol_key_helper_generate_pre_keys(&pre_keys_head, start_id, 100, global_context);
signal_protocol_key_helper_generate_signed_pre_key(&signed_pre_key, identity_key_pair, 5, timestamp, global_context);

/* Store identity_key_pair somewhere durable and safe. */
/* Store registration_id somewhere durable and safe. */

/* Store pre keys in the pre key store. */
/* Store signed pre key in the signed pre key store. */
```

The above example is simplified for the sake of clarity. All of these functions return errors
on failure, and those errors should be checked for in real usage.

There are also iteration and serialization methods for the above types that should
be used as appropriate.

## Building a session

A libsignal-protocol-c client needs to implement four data store callback interfaces:
`signal_protocol_identity_key_store`, `signal_protocol_pre_key_store`,
`signal_protocol_signed_pre_key_store`, and `signal_protocol_session_store`.
These will manage loading and storing of identity, prekeys, signed prekeys,
and session state.

These callback interfaces are designed such that implementations should treat
all data flowing through them as opaque binary blobs. Anything necessary for
referencing that data will be provided as separate function arguments to those
callbacks. If it is ever necessary for clients to directly access stored data
in terms of library data structures, they should use the accessor functions
declared in "signal_protocol.h" for these data stores.

Once the callbacks for these data stores are implemented, building a session
is fairly straightforward:

```c
/* Create the data store context, and add all the callbacks to it */
signal_protocol_store_context *store_context;
signal_protocol_store_context_create(&store_context, context);
signal_protocol_store_context_set_session_store(store_context, &session_store);
signal_protocol_store_context_set_pre_key_store(store_context, &pre_key_store);
signal_protocol_store_context_set_signed_pre_key_store(store_context, &signed_pre_key_store);
signal_protocol_store_context_set_identity_key_store(store_context, &identity_key_store);

/* Instantiate a session_builder for a recipient address. */
signal_protocol_address address = {
    "+14159998888", 12, 1
};
session_builder *builder;
session_builder_create(&builder, store_context, &address, global_context);

/* Build a session with a pre key retrieved from the server. */
session_builder_process_pre_key_bundle(builder, retrieved_pre_key);

/* Create the session cipher and encrypt the message */
session_cipher *cipher;
session_cipher_create(&cipher, store_context, &address, global_context);

ciphertext_message *encrypted_message;
session_cipher_encrypt(cipher, message, message_len, &encrypted_message);

/* Get the serialized content and deliver it */
signal_buffer *serialized = ciphertext_message_get_serialized(encrypted_message);

deliver(signal_buffer_data(serialized), signal_buffer_len(serialized));

/* Cleanup */
SIGNAL_UNREF(encrypted_message);
session_cipher_free(cipher);
session_builder_free(builder);
signal_protocol_store_context_destroy(store_context);
```

The above example is simplified for the sake of clarity. All of these functions return errors
on failure, and those errors should be checked for in real usage.

## Memory management notes

For every custom data type that the libsignal-protocol-c library can allocate and
return, a corresponding way of deallocating an instance of that data type
is provided.

The more basic and higher level data types provide a type-specific free or
destroy function. These types include `signal_context`,
`signal_protocol_store_context`, `signal_buffer`, `signal_buffer_list`,
`signal_int_list`, `signal_protocol_key_helper_pre_key_list_node`, `session_builder`,
`session_cipher`, `group_session_builder`, `group_cipher`, and
`fingerprint_generator`.

Most of the other data types, including everything internal, use a reference
counting mechanism. If you are going to hold onto a reference to one of these
types, use the `SIGNAL_REF(x)` macro to increment its count. If you are done
with a reference, use `SIGNAL_UNREF(x)` to decrement its count. When the count
reaches 0, the type's destructor function is called.

# Legal things
## Cryptography Notice

This distribution includes cryptographic software. The country in which you currently reside may have restrictions on the import, possession, use, and/or re-export to another country, of encryption software.
BEFORE using any encryption software, please check your country's laws, regulations and policies concerning the import, possession, or use, and re-export of encryption software, to see if this is permitted.
See <http://www.wassenaar.org/> for more information.

The U.S. Government Department of Commerce, Bureau of Industry and Security (BIS), has classified this software as Export Commodity Control Number (ECCN) 5D002.C.1, which includes information security software using or performing cryptographic functions with asymmetric algorithms.
The form and manner of this distribution makes it eligible for export under the License Exception ENC Technology Software Unrestricted (TSU) exception (see the BIS Export Administration Regulations, Section 740.13) for both object code and source code.

## License

Copyright 2015-2016 Open Whisper Systems

Licensed under the GPLv3: http://www.gnu.org/licenses/gpl-3.0.html

Additional Permissions For Submission to Apple App Store: Provided that you are otherwise in compliance with the GPLv3 for each covered work you convey (including without limitation making the Corresponding Source available in compliance with Section 6 of the GPLv3), Open Whisper Systems also grants you the additional permission to convey through the Apple App Store non-source executable versions of the Program as incorporated into each applicable covered work as Executable Versions only under the Mozilla Public License version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/).

