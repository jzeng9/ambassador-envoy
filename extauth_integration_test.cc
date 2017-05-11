#include "test/integration/integration.h"
#include "test/integration/utility.h"

class ExtAuthIntegrationTest : public BaseIntegrationTest, public testing::Test {
public:
  /**
   * Global initializer for all integration tests.
   */
  static void SetUpTestCase() {
    createTestServer("extauth_server.json", {"extauth"});
  }

  /**
   * Global destructor for all integration tests.
   */
  static void TearDownTestCase() {
    test_server_.reset();
  }
};

TEST_F(ExtAuthIntegrationTest, DoAuth) {
  Buffer::OwnedImpl buffer("hello");
  std::string response;
  RawConnectionDriver connection(lookupPort("extauth"), buffer,
                                 [&](Network::ClientConnection&, const Buffer::Instance& data)
                                     -> void {
                                       response.append(TestUtility::bufferToString(data));
                                       connection.close();
                                     });

  connection.run();
  EXPECT_EQ("hello", response);
}
