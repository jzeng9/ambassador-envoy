#include "extauth.h"
#include "common/http/header_map_impl.h"
#include "common/stats/stats_impl.h"

#include "test/mocks/buffer/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/upstream/host.h"

namespace Envoy {
using testing::NiceMock;
using testing::InSequence;

namespace Http {

class ExtAuthTest : public testing::Test {
public:
  ExtAuthTest()
      : config_{new ExtAuthConfig{ExtAuth::generateStats("", store_), 0}}, filter_(config_) {
    filter_.setDecoderFilterCallbacks(callbacks_);
  }

  NiceMock<MockStreamDecoderFilterCallbacks> callbacks_;
  Stats::IsolatedStoreImpl store_;
  std::shared_ptr<ExtAuthConfig> config_;
  ExtAuth filter_;
};

TEST_F(ExtAuthTest, HeaderOnlyRequest) {
  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::Continue, filter_.decodeHeaders(headers, true));
}

TEST_F(ExtAuthTest, RequestWithData) {
  InSequence s;

  TestHeaderMapImpl headers;
  EXPECT_EQ(FilterHeadersStatus::Continue, filter_.decodeHeaders(headers, false));

  Buffer::OwnedImpl data1("hello");
  EXPECT_EQ(FilterDataStatus::Continue, filter_.decodeData(data1, false));

  Buffer::OwnedImpl data2(" world");
  EXPECT_EQ(FilterDataStatus::Continue, filter_.decodeData(data2, true));
}

} // Http
} // Envoy
