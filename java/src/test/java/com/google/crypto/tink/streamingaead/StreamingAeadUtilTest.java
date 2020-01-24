// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

package com.google.crypto.tink.streamingaead;

import static com.google.crypto.tink.TestUtil.assertExceptionContains;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import com.google.crypto.tink.proto.HashType;
import java.security.NoSuchAlgorithmException;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/** Tests for StreamingAeadUtil. */
@RunWith(JUnit4.class)
public class StreamingAeadUtilTest {
  @Test
  public void testToHmacAlgo() throws Exception {
    assertEquals("HmacSha1", StreamingAeadUtil.toHmacAlgo(HashType.SHA1));
    assertEquals("HmacSha256", StreamingAeadUtil.toHmacAlgo(HashType.SHA256));
    assertEquals("HmacSha512", StreamingAeadUtil.toHmacAlgo(HashType.SHA512));

    try {
      StreamingAeadUtil.toHmacAlgo(HashType.UNKNOWN_HASH);
      fail("should throw NoSuchAlgorithmException");
    } catch (NoSuchAlgorithmException ex) {
      assertExceptionContains(ex, "hash unsupported for HMAC");
    }
  }
}
