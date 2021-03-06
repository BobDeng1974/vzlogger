#include <gmock/gmock.h>
	using ::testing::Eq;
	using ::testing::Return;
	using ::testing::ReturnRef;
	using ::testing::InSequence;
	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::Invoke;

#include <gtest/gtest.h>
	using ::testing::Test;

#include "Meter.hpp"
#include "protocols/MeterS0.hpp"


namespace mock_MeterS0
{

class mock_S0hwif : public MeterS0::HWIF
{
public:
	mock_S0hwif() {};
	virtual ~mock_S0hwif() {};
	MOCK_METHOD0(_open, bool());
	MOCK_METHOD0(_close, bool());
	MOCK_METHOD0(waitForImpulse, bool());
	MOCK_METHOD0(status, int());
	MOCK_CONST_METHOD0(is_blocking, bool());

protected:
};

TEST(mock_MeterS0, basic_noopen)
{
	mock_S0hwif *hwif = new mock_S0hwif();
	std::list<Option> opt;

	ASSERT_NO_THROW( MeterS0 m(opt, hwif));
}

TEST(mock_MeterS0, basic_open_fail)
{
	mock_S0hwif *hwif = new mock_S0hwif();
	std::list<Option> opt;

	EXPECT_CALL(*hwif, _open()).Times(1).WillRepeatedly(Return(false));
	EXPECT_CALL(*hwif, _close()).Times(1).WillOnce(Return(false));
	MeterS0 m(opt, hwif);
	ASSERT_EQ(ERR, m.open());
	m.close(); // this might be called and should not cause problems
}

TEST(mock_MeterS0, basic_blocking)
{
	mock_S0hwif *hwif = new mock_S0hwif();
	std::list<Option> opt;
	opt.push_back(Option("send_zero", true));

	EXPECT_CALL(*hwif, _open()).Times(1).WillRepeatedly(Return(true));
	EXPECT_CALL(*hwif, _close()).Times(1).WillOnce(Return(true));
	EXPECT_CALL(*hwif, is_blocking()).Times(2).WillRepeatedly(Return(true));
	MeterS0 m(opt, hwif);
	ASSERT_EQ(SUCCESS, m.open());
	m.close(); // this might be called and should not cause problems
}

TEST(mock_MeterS0, basic_blocking_no_send_zero)
{
	mock_S0hwif *hwif = new mock_S0hwif();
	std::list<Option> opt;
	opt.push_back(Option("send_zero", false));

	EXPECT_CALL(*hwif, _open()).Times(1).WillRepeatedly(Return(true));
	EXPECT_CALL(*hwif, _close()).Times(1).WillOnce(Return(true));
	EXPECT_CALL(*hwif, is_blocking()).Times(2).WillRepeatedly(Return(true));
	MeterS0 m(opt, hwif);
	ASSERT_EQ(SUCCESS, m.open());
	m.close(); // this might be called and should not cause problems
}


TEST(mock_MeterS0, basic_non_blocking)
{
	mock_S0hwif *hwif = new mock_S0hwif();
	std::list<Option> opt;
	opt.push_back(Option("send_zero", true));

	EXPECT_CALL(*hwif, _open()).Times(1).WillRepeatedly(Return(true));
	EXPECT_CALL(*hwif, _close()).Times(1).WillOnce(Return(true));
	EXPECT_CALL(*hwif, is_blocking()).Times(2).WillRepeatedly(Return(false));
	EXPECT_CALL(*hwif, status()).Times(AtLeast(0)).WillRepeatedly(Return(0));
	MeterS0 m(opt, hwif);
	ASSERT_EQ(SUCCESS, m.open());
	m.close(); // this might be called and should not cause problems
}


TEST(mock_MeterS0, basic_non_blocking_read)
{
	mock_S0hwif *hwif = new mock_S0hwif();
	std::list<Option> opt;
	opt.push_back(Option("send_zero", true));

	EXPECT_CALL(*hwif, _open()).Times(1).WillRepeatedly(Return(true));
	EXPECT_CALL(*hwif, _close()).Times(1).WillOnce(Return(true));
	EXPECT_CALL(*hwif, is_blocking()).Times(3).WillRepeatedly(Return(false));
	EXPECT_CALL(*hwif, status()).Times(AtLeast(1)).WillRepeatedly(Return(0));
	MeterS0 m(opt, hwif);
	ASSERT_EQ(SUCCESS, m.open());
	std::vector<Reading> rds(4);
	ASSERT_EQ(m.read(rds, 4), 2); // no Powers for first impulse

	m.close(); // this might be called and should not cause problems
}

/* time out -> endless waiting for first impulse
TEST(mock_MeterS0, basic_non_blocking_read_no_send_zero)
{
	mock_S0hwif *hwif = new mock_S0hwif();
	std::list<Option> opt;
	opt.push_back(Option("send_zero", false));

	EXPECT_CALL(*hwif, _open()).Times(1).WillRepeatedly(Return(true));
	EXPECT_CALL(*hwif, _close()).Times(1).WillOnce(Return(true));
	EXPECT_CALL(*hwif, is_blocking()).Times(2).WillRepeatedly(Return(false));
	MeterS0 m(opt, hwif);
	ASSERT_EQ(SUCCESS, m.open());
	std::vector<Reading> rds(4);
	ASSERT_EQ(m.read(rds, 4), 0);

	m.close(); // this might be called and should not cause problems
} */


} // namespace

void print(log_level_t l, char const*s1, char const*s2, ...)
{
//	if (l!= log_debug)
	{
		fprintf(stdout, "\n  %s:", s2);
		va_list argp;
		va_start(argp, s2);
		vfprintf(stdout, s1, argp);
		va_end(argp);
		fprintf(stdout, "\n");
	}
}

int main(int argc, char** argv) {
  // The following line must be executed to initialize Google Mock
  // (and Google Test) before running the tests.
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
