# OpenPGP in C++
Copyright (c) 2013, 2014 Jason Lee @ calccrypto at gmail.com

Please see LICENSE file for license.

[![Build Status](https://travis-ci.org/calccrypto/OpenPGP.svg?branch=master)](https://travis-ci.org/calccrypto/OpenPGP)
<pre>
With much help from:
    Alex Stapleton (OpenPGP-SDK)
    Auston Sterling - Massive amounts of debugging and programming help
    Jon Callas (RFC 4880)
    Herbert Hanewinkel (hanewin.net)
    Many people on the StackExchange network
    mugwort-rc - Tons of testing code, code style updates, and bugfixes
    pgpdump.net
    PortablePGP

IMPORTANT:
    This library was not written for actual use.
    Rather, it was meant for learning about the
    internals of PGP can easily use/add a few
    std::couts to see the internal workings.
    So if you choose to use it in a real setting
    where secrecy is required, do so at your own
    risk.

This is a C++ implementation of the majority of RFC 4880,
the OpenPGP Message Format.

The following are the libraries necessary to build OpenPGP:
    GMP (https://gmplib.org/, sudo apt-get install libdev-gmp, etc)
    bzip2 (http://www.bzip.org/, sudo apt-get install libbz2-dev, etc)
    zlib (http://www.zlib.net/, sudo apt-get install zlib1g-dev, etc)

The purpose of this library is to help clear up the mess that
is RFC 4880. It is extremely vague at best, and it took me
a long time to figure out most of it. No one should have to go
through that. However, knowing how PGP is structured is still
good to know.

This library allows for the modification of PGP packets, such
as providing incorrect checksums and public key values. That
was done on purpose. I used it to test keys I created with
known working values. What others do with this capability
is none of my concern or responsibility.

This library should be relatively straightforward to use:
Simply #include whatever functions needed:
    key generation - generatekey.h
    key revocation - revoke.h
    encrypt        - encrypt.h
    decrypt        - decrypt.h
    sign           - sign.h
    verify         - verify.h

Multiple classes inherit from the base class PGP in order 
to make differentiating PGP block types better in code.
    PGP - base class; use if PGP data type is not known
    PGPDetachedSignature - detached signatures for files
    PGPKey - base class for OpenPGP key types
    PGPPublicKey - holds public keys; inherits PGPKey
    PGPSecretKey - holds private keys; inherits PGPKey
    PGPMessage - holds OpenPGP messages
    
PGPCleartextSignature does not inherit from PGP.
    
The exec/main.cpp file provides a simple command line interface,
which can be used as examples on how to use the functions. A lot
of the output was based on/inspired by pgpdump.net and GPG.

All data structures have some standard functions:
    read  - reads data without the respective
            header information

    show  - displays the data in human readable form
            like the way pgpdump.net does it.

    raw   - returns a string that can be read by the
            read function.

    write - returns a string of the entire data,
            including extra data, such as header
            and size.

    clone - returns a pointer to a deep copy of
            the object (mainly used for moving
            PGP data around).

    Ptr   - a typedef for std::shared_ptr <T>, where
            T is the class each typdef is found in.
            
Operator= and the copy constructor have been overloaded
for the data structures that need deep copy.

To build the command line program run make on OpenPGP/exec/main.cpp

Notes:
    Sometimes, there are excerpts from RFC 4880 in the code.
    Most of those excerpts are not the full text of the sections.
    Please refer to the RFC for the full text.
    
    Keyrings were not implemented. Rather, keys are read
    from the directory used as arguments to functions.

    There are also several untested functions, having never
    encountered real versions of those packets/subpackets.

    If for some reason the program cannot operate on some data
    properly, an exception will be thrown. This will be changed
    sometime, so that not all errors will cause crashes.
</pre>