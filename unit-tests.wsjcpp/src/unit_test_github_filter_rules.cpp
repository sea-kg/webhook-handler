
#include <wsjcpp_core.h>
#include <wsjcpp_unit_tests.h>

// ---------------------------------------------------------------------
// UnitTestGithubFilterRules

class UnitTestGithubFilterRules : public WsjcppUnitTestBase {
    public:
        UnitTestGithubFilterRules();
        virtual bool doBeforeTest() override;
        virtual void executeTest() override;
        virtual bool doAfterTest() override;
};

REGISTRY_WSJCPP_UNIT_TEST(UnitTestGithubFilterRules)

UnitTestGithubFilterRules::UnitTestGithubFilterRules()
    : WsjcppUnitTestBase("UnitTestGithubFilterRules") {
}

// ---------------------------------------------------------------------

bool UnitTestGithubFilterRules::doBeforeTest() {
    // do something before test
    return true;
}

// ---------------------------------------------------------------------

void UnitTestGithubFilterRules::executeTest() {
    compare("TODO", true, true);
    // compare("Not implemented", true, false);
    // TODO unit test source code here
}

// ---------------------------------------------------------------------

bool UnitTestGithubFilterRules::doAfterTest() {
    // do somethig after test
    return true;
}


