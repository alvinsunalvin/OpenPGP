#include <gtest/gtest.h>

#include "sign.h"

#include "../testvectors/msgpass.h"
#include "testvectors/rsa/rsasiggen15_186-2.h"

const uint8_t PKA_RSA = 1;

TEST(RSA, sign_pkcs1_v1_5) {

    ASSERT_EQ(RSA_SIGGEN_N.size(), RSA_SIGGEN_D.size());

    auto e = hextompi(RSA_SIGGEN_E);
    for ( unsigned int i = 0; i < RSA_SIGGEN_N.size(); ++i ) {
        auto n = hextompi(RSA_SIGGEN_N[i]);
        auto d = hextompi(RSA_SIGGEN_D[i]);

        for ( unsigned int x = 0; x < RSA_SIGGEN_MSG[i].size(); ++x ) {
            auto msg = RSA_SIGGEN_MSG[i][x];
            int h = std::get<0>(msg);
            std::string data = unhexlify(std::get<1>(msg));
            std::string digest = use_hash(h, data);
            std::string error;
            auto ret = pka_sign(digest, PKA_RSA, {d}, {n, e}, h, error);
            ASSERT_EQ(ret.size(), (std::size_t) 1);
            EXPECT_EQ(mpitohex(ret[0]), RSA_SIGGEN_SIG[i][x]);
            EXPECT_EQ(pka_verify(digest, h, PKA_RSA, {n, e}, {hextompi(RSA_SIGGEN_SIG[i][x])}, error), true);
        }
    }
}

TEST(RSA, keygen) {
    PKA::Values key = RSA_keygen(512);
    PKA::Values pub = {key[0], key[1]};
    PKA::Values pri = {key[2], key[3], key[4], key[5]};

    PGPMPI message = rawtompi(MESSAGE);

    auto encrypted = RSA_encrypt(message, pub);
    auto decrypted = RSA_decrypt(encrypted, pri, pub);
    EXPECT_EQ(decrypted, message);

    auto signature = RSA_sign(message, pri, pub);
    EXPECT_TRUE(RSA_verify(message, {signature}, pub));
}
