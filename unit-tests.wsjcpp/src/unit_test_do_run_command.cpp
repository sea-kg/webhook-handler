
#include <wsjcpp_core.h>
#include <wsjcpp_unit_tests.h>
#include "do_run_command.h"

// ---------------------------------------------------------------------
// UnitTestDoRunCommand

class UnitTestDoRunCommand : public WsjcppUnitTestBase {
    public:
        UnitTestDoRunCommand();
        virtual bool doBeforeTest() override;
        virtual void executeTest() override;
        virtual bool doAfterTest() override;
};

REGISTRY_WSJCPP_UNIT_TEST(UnitTestDoRunCommand)

UnitTestDoRunCommand::UnitTestDoRunCommand()
    : WsjcppUnitTestBase("UnitTestDoRunCommand") {
}

// ---------------------------------------------------------------------

bool UnitTestDoRunCommand::doBeforeTest() {
    // do something before test
    return true;
}

// ---------------------------------------------------------------------

void UnitTestDoRunCommand::executeTest() {

    DoRunCommand cmd(".", {"sleep", "1"});
    cmd.start(100);
    compare("sleep 1: hasError", cmd.hasError(), true);
    compare("sleep 1: isTimeout", cmd.isTimeout(), true);
    compare("sleep 1: exitCode", cmd.exitCode(), -1);
    compare("sleep 1: outputString", cmd.outputString(), "timeout");

    DoRunCommand cmd2(".", {"echo", "somesomesome"});
    cmd2.start(100);
    compare("echo : hasError", cmd2.hasError(), false);
    compare("echo : isTimeout", cmd2.isTimeout(), false);
    compare("echo : exitCode", cmd2.exitCode(), 0);
    std::string sOutput = cmd2.outputString();
    std::vector<std::string> vLines = WsjcppCore::split(sOutput, "\n");
    if (compare("echo : vLines.size()", vLines.size(), 4)) {
        compare("echo : outputString", vLines[1], "fork: Change dir '.'");
        compare("echo : outputString", vLines[2], "somesomesome");
        compare("echo : outputString", vLines[3], "");
    }
}

// ---------------------------------------------------------------------

bool UnitTestDoRunCommand::doAfterTest() {
    // do somethig after test
    return true;
}


