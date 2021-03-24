
#include <wsjcpp_core.h>
#include <wsjcpp_unit_tests.h>
#include "webhook_handler_config.h"

// ---------------------------------------------------------------------
// UnitTestParseCommandsArgs

class UnitTestParseCommandsArgs : public WsjcppUnitTestBase {
    public:
        UnitTestParseCommandsArgs();
        virtual bool doBeforeTest() override;
        virtual void executeTest() override;
        virtual bool doAfterTest() override;
};

REGISTRY_WSJCPP_UNIT_TEST(UnitTestParseCommandsArgs)

UnitTestParseCommandsArgs::UnitTestParseCommandsArgs()
    : WsjcppUnitTestBase("UnitTestParseCommandsArgs") {
}

// ---------------------------------------------------------------------

bool UnitTestParseCommandsArgs::doBeforeTest() {
    // do something before test
    return true;
}

// ---------------------------------------------------------------------

void UnitTestParseCommandsArgs::executeTest() {
    struct LocalTest {
        LocalTest(std::string sCommand, std::vector<std::string> vExpectedArgs) {
            command = sCommand;
            expectedArgs = vExpectedArgs;
        }
        std::string command;
        std::vector<std::string> expectedArgs;
    };
    
    std::vector<LocalTest> vTests;
    vTests.push_back(LocalTest("ls -la", {"ls", "-la"}));
    vTests.push_back(LocalTest("echo \"some text\"", {"echo", "\"some text\""}));
    vTests.push_back(LocalTest("echo 'some text'", {"echo", "'some text'"}));

    for (int i = 0; i < vTests.size(); i++) {
        std::string sCommand = vTests[i].command;
        WebhookShellCommand shellCommand(sCommand);

        std::vector<std::string> vExpectedArgs = vTests[i].expectedArgs;
        compare("Args size for [" + sCommand + "]", vExpectedArgs.size(), shellCommand.getArgs().size());
        if (vExpectedArgs.size() == shellCommand.getArgs().size()) {
            for (int x = 0; x < shellCommand.getArgs().size(); x++) {
                compare("arg", vExpectedArgs[x], shellCommand.getArgs()[x]);
            }
        }
    }
    
    // TODO unit test source code here
}

// ---------------------------------------------------------------------

bool UnitTestParseCommandsArgs::doAfterTest() {
    // do somethig after test
    return true;
}


