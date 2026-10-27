// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include <munit.h>
#include <chiaki/ecdh.h>
#include <string.h>
#ifndef CHIAKI_LIB_ENABLE_MBEDTLS
#include <openssl/ec.h>
#endif

static MunitResult test_p521(const MunitParameter params[], void *user)
{
 ChiakiECDH a, b;
 munit_assert_int(chiaki_ecdh_init_with_curve(&a, true), ==, CHIAKI_ERR_SUCCESS);
 munit_assert_int(chiaki_ecdh_init_with_curve(&b, true), ==, CHIAKI_ERR_SUCCESS);
#ifndef CHIAKI_LIB_ENABLE_MBEDTLS
 /* Private scalar 1 makes the shared secret exactly the peer public X
  * coordinate, an independent mathematical oracle for 66-byte padding. */
 uint8_t generator[133], one[] = {1};
 munit_assert_size(EC_POINT_point2oct(a.group, EC_GROUP_get0_generator(a.group),
   POINT_CONVERSION_UNCOMPRESSED, generator, sizeof(generator), NULL), ==, 133);
 munit_assert_int(chiaki_ecdh_set_local_key(&a, one, sizeof(one), generator, sizeof(generator)), ==, CHIAKI_ERR_SUCCESS);
#endif
 uint8_t handshake[16] = {0}, apub[133], bpub[133], asig[32], bsig[32];
 size_t aps=sizeof(apub), bps=sizeof(bpub), ass=sizeof(asig), bss=sizeof(bsig);
 munit_assert_int(chiaki_ecdh_get_local_pub_key(&a, apub, &aps, handshake, asig, &ass), ==, CHIAKI_ERR_SUCCESS);
 munit_assert_int(chiaki_ecdh_get_local_pub_key(&b, bpub, &bps, handshake, bsig, &bss), ==, CHIAKI_ERR_SUCCESS);
 munit_assert_size(aps, ==, 133); munit_assert_size(bps, ==, 133);
 munit_assert_size(ass, ==, 32); munit_assert_size(bss, ==, 32);
 uint8_t asecret[67], bsecret[66]; memset(asecret, 0xa5, sizeof(asecret));
 munit_assert_int(chiaki_ecdh_derive_secret_with_size(&a, asecret, 66, bpub, bps, handshake, bsig, bss), ==, CHIAKI_ERR_SUCCESS);
 munit_assert_int(chiaki_ecdh_derive_secret_with_size(&b, bsecret, 66, apub, aps, handshake, asig, ass), ==, CHIAKI_ERR_SUCCESS);
 munit_assert_memory_equal(66, asecret, bsecret);
 munit_assert_uint8(asecret[66], ==, 0xa5);
#ifndef CHIAKI_LIB_ENABLE_MBEDTLS
 munit_assert_memory_equal(66, asecret, bpub + 1);
#endif
 munit_assert_int(chiaki_ecdh_derive_secret(&a, asecret, bpub, bps, handshake, bsig, bss), ==, CHIAKI_ERR_BUF_TOO_SMALL);
 munit_assert_int(chiaki_ecdh_derive_secret_with_size(&a, asecret, 65, bpub, bps, handshake, bsig, bss), ==, CHIAKI_ERR_BUF_TOO_SMALL);
 munit_assert_int(chiaki_ecdh_derive_secret_with_size(&a, asecret, 66, bpub, 132, handshake, bsig, bss), ==, CHIAKI_ERR_INVALID_DATA);
 bsig[0] ^= 1;
 munit_assert_int(chiaki_ecdh_derive_secret_with_size(&a, asecret, 66, bpub, bps, handshake, bsig, bss), ==, CHIAKI_ERR_INVALID_MAC);
 aps = 128;
 munit_assert_int(chiaki_ecdh_get_local_pub_key(&a, apub, &aps, handshake, asig, &ass), ==, CHIAKI_ERR_BUF_TOO_SMALL);
 chiaki_ecdh_fini(&a); chiaki_ecdh_fini(&b);
 return MUNIT_OK;
}
MunitTest tests_ecdh[] = {
 { "/p521", test_p521, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
 { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};
