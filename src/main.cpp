#include "helper.h"
#include "scanner.h"
#include "parser.h"
#include "timer.h"

parser::Parser* parser::Parser::s_instance;

int main(int argc, char** argv)
{
    Timer timer;
    parser::Parser* pars = parser::Parser::createInstance(argc, argv);

    if (argc <= 1) {
        Helper::helpMessage();
        return 0;
    } else if (!pars->argOptionExists("--image")) {
        Helper::helpMessage();
        return 0;
    }

    scan::Scanner scan(pars->getCmdArg(std::string("--image")));
    scan.matchDatas();
    parser::Parser::removeInstance();
    return 0;
}
