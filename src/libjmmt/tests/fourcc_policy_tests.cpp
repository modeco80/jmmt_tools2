#include <cstring>
#include <jmmt/fourcc.hpp>
#include <mco/nounit.hpp>

// Strictly speaking these tests *could* be done at compile-time,
// but /shrug. This works just as well.

mcoNoUnitDeclareTest(defaultPolicyWorks, "fourcc default policy works") {
	auto s = jmmt::FourCCDefaultPolicy::execute<"test">();
	auto s2 = jmmt::FourCCDefaultPolicy::execute<"test12345678">();
	// Both strings are truncated to 4 characters,
	// so they should always equal the same.
	mcoNoUnitAssert(!memcmp(s.buf, "test", 4));
	mcoNoUnitAssert(!memcmp(s2.buf, "test", 4));
	mcoNoUnitAssert(!memcmp(s.buf, s2.buf, 4));
}

mcoNoUnitDeclareTest(padPolicyWorks, "fourcc pad policy works") {
	auto s = jmmt::FourCCPadPolicy<>::execute<"test">();
	auto s2 = jmmt::FourCCPadPolicy<>::execute<"te">();
	auto s3 = jmmt::FourCCPadPolicy<>::execute<"t">();

	// s should NOT be padded (it fills 4 characters).
	mcoNoUnitAssert(!memcmp(s.buf, "test", 4));

	// s2 SHOULD be padded with 2 0x20 characters
	mcoNoUnitAssert(!memcmp(s2.buf, "te\x20\x20", 4));

	// s3 also SHOULD be padded with 3 0x20 characters
	mcoNoUnitAssert(!memcmp(s3.buf, "t\x20\x20\x20", 4));
}

mcoNoUnitMain();
