#include<iostream>
#include<string>
#include<sstream>
#include<boost/program_options.hpp>


#include "HopperConfig.h"
#include "parser/parser.h"

namespace po = boost::program_options;

std::string version_number() {
    std::stringstream ss;
    ss << Hopper_VERSION_MAJOR << '.' << Hopper_VERSION_MINOR;
    return ss.str();
}

po::variables_map parse_input(int argc, char *argv[]) {
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("input-files", po::value<std::vector<std::string>>(), "Input files")
        ;
    po::positional_options_description p;
    p.add("input-files", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help") || !vm.count("input-files")) {
        std::cout << "gracc Hopper version " << version_number() << "\n"
                  << desc << "\n";
        exit(1);
    }
    return vm;
}

int main(int argc, char *argv[])
{
    auto vm = parse_input(argc, argv);
    auto inputs = vm["input-files"].as<std::vector<std::string>>();

    for (const std::string& input : inputs) {
        int result = parser::parse(input);
        if (result) exit(result);
    }

    return 0;
}
